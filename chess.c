#include "chess.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#if defined(_WIN32) || defined(__CYGWIN__)
#include <windows.h>
#endif

// Standard-Ini falls das File verloren geht
const char* defaultIni = 
"loop=0\n\
height=8\n\
width=8\n\
dynamic=30\n\
permutation=0\n\
autoRandom=1\n\
csv=\"\"\n";

// Name des ini-Files
const char* iniName = "parameter.ini";

// #####################################################################################
// ### Funktionen für Parameter, Konselenargumente und deren Ininitialisierung, etc. ###
// #####################################################################################

// initiert Parameter-struct
parameters* initParameters(){
  parameters* param = (parameters*)calloc(1, sizeof(parameters));
  if(param == NULL)
    return NULL;
  param->boardWidth = 8;
  param->boardHeight = 8;
  return param;
}

// Lädt parameter.ini 
bool loadParameterIni(parameters* param){
  FILE* handle = fopen(iniName, "r");
  if(handle == NULL)return true;
  fscanf(handle, "loop=%d\n", &param->loop);
  fscanf(handle, "height=%d\n", &param->boardHeight);
  fscanf(handle, "width=%d\n", &param->boardWidth);
  fscanf(handle, "dynamic=%d\n", &param->dynamicOutputDelay);
  fscanf(handle, "permutation=%d\n", &param->permutation);
  fscanf(handle, "autoRandom=%d\n", &param->autoRandom);
  fscanf(handle, "csv=\"%[^\"]\"\n", param->CSVfilename);
  fclose(handle);
  return false;
}

// Erzeugt eine parameter.ini mit Standardwerten
bool createDefaultParameterIni(){
  FILE* handle = fopen(iniName, "w");
  if(handle != NULL){
    fprintf(handle, defaultIni);
    fclose(handle);
    return true;
  }
  return false;
}

// Wertet die Optionen/Argumente aus der Kommandozeile aus. Bie einem Fehler wird die Nummer des Fehlerhaften Arguments zurückgegeben.
int handleCommandLineArguments(parameters* param, int argc, char* argv[]){
  for(int i = 1; i < argc; ++i)
    if(parseArgument(param, argv[i]))
      return i;
  return 0;
}

// wertet aus ob das Argument zu einer der Typen Flag, Wert oder Koordinate gehört
bool parseArgument(parameters* param, const char* argument){
  if(checkFlags(param, argument))
    return 0;
  if(checkArgumentValues(param, argument))
    return 0;
  if(checkArgumentCoordinate(param, argument))
    return 0;
  return 1;
}

// prüft das argument auf alle möglichen Flags ab
bool checkFlags(parameters* param, const char* argument){
  if(checkArgumentFlag(argument, "help") || checkArgumentFlag(argument, "h")){
    param->helpFlag = true;
    return true;
  }
  if(checkArgumentFlag(argument, "loop") || checkArgumentFlag(argument, "l")){
    param->loop = true;
    return true;
  }
  if(checkArgumentFlag(argument, "dynamic") || checkArgumentFlag(argument, "d")){
    param->dynamicOutputDelay = 1;
    return true;
  }
  return false;
}

// prüft ob das Argument die <flag> setzt
bool checkArgumentFlag(const char* argument, const char* flag){
  if(argument[0] == '-' && !strcmp(argument+1, flag))return true;
  if(argument[0] == '-' && argument[1] == '-' && !strcmp(argument+2, flag))return true;
  return false;
}

// prüft ob das Argument einen Wert setzt
bool checkArgumentValues(parameters* param, const char* argument){
  if(!strncmp(argument, "-", 1)){    
    if( sscanf(argument, "-w=%d", &param->boardWidth) ||
        sscanf(argument, "-width=%d", &param->boardWidth) ||
        sscanf(argument, "-h=%d", &param->boardHeight) || 
        sscanf(argument, "-height=%d", &param->boardHeight) ||
        sscanf(argument, "-csv=%s", param->CSVfilename) ||
        sscanf(argument, "-p=%d", &param->permutation) ||
        sscanf(argument, "-d=%d", &param->dynamicOutputDelay) ||
        sscanf(argument, "-dynamic=%d", &param->dynamicOutputDelay))
      return true;
  }
  return false;
}

// prüft ob das Argument ein Positionswert (für die Startposition) ist
bool checkArgumentCoordinate(parameters* param, const char* argument){
  char posLetter;
    if(sscanf(argument, "%c%d", &posLetter, &param->startingPos.y) == 2){
      param->startingPos.x = convertLetterToPosition(posLetter);
      return true;
    }
  return false;
} 

// den Buchstabenteil aus z.B. "A6" oder "f1" zu dem integer-Positionswert konvertieren
int convertLetterToPosition(char letter){
  int pos_x = letter - 'a';
  if(pos_x < 0)pos_x = letter - 'A';
  return pos_x;
}

// Normalisierung der y-Koordinate notwendig weil Spielbrett im Speicher und Ausgabe Seitenverkehrt ist
// lediglich notwendig für Benutzer-WYSIWYG
void normalizeCoordinate(coord* coordinate, int boardHeight){
  coordinate->y = boardHeight - coordinate->y;
}

// generiert eine zufällige Koordinate, innerhalb der gegebenen Grenzen
coord getRandomCoord(int max_x, int max_y){
  srand(time(NULL));
  coord randomPosition;
  randomPosition.x = rand() % max_x;
  randomPosition.y = rand() % max_y;
  return randomPosition;
}


// #####################################################################################
// ### Initierungsfunktionen für das Spielbrett + Allgemeine Board-Funktionen.       ###
// #####################################################################################

// Initiert ein neues Brett mit allem drum und dran
board* initBoard(int width, int height){
  // Brett erzeugen
  board* boardPointer = calloc(1, sizeof(board));
  if(boardPointer == NULL)
    return NULL;
  boardPointer->width = width;
  boardPointer->height = height;
  // Windows-Konsolen-API
  #if defined(_WIN32) || defined(__CYGWIN__)
  boardPointer->consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  GetConsoleScreenBufferInfo(boardPointer->consoleHandle, &boardPointer->bufferInfo);
  #endif
  // Ausgabestring initieren und füllen
  if(initOutputString(boardPointer))
    return NULL;
  drawOutputField(boardPointer);
  // Feld-Array Erzeugen
  boardPointer->fields = (field*)calloc(boardPointer->height*boardPointer->width, sizeof(field));
  if(boardPointer->fields == NULL)
    return NULL;
  initAllFields(boardPointer);  
  return boardPointer;
}

// Initiert und verlinkt alle Felder die einen Sprung voneinander entfernt sind miteinander
void initAllFields(board* boardPointer){
  for(int x = 0; x < boardPointer->width; ++x)
    for(int y = 0; y < boardPointer->height; ++y){
      initField(boardPointer, (coord){x, y});
      linkFields(boardPointer, (coord){x, y});
    }
}

// Initialisiert ein Einzeleld 
void initField(board* boardPointer, coord position){
  field* fieldPointer = getFieldPointer(boardPointer, position);
  fieldPointer->outputFieldLocation = calcOutputFieldLocation(boardPointer, position);     
  #if defined(_WIN32) || defined(__CYGWIN__)
  fieldPointer->consoleFieldPosition = calcConsoleFieldPosition(boardPointer, position);
  #endif
}

// Berechnet den Pointer für die Position des Feldes am Ausgabestring
char* calcOutputFieldLocation(board* boardPointer, coord position){
  return boardPointer->outputString + boardPointer->outputWidth*(2*position.y +2) + (boardPointer->fieldSize+1)*position.x + 1;;
}

#if defined(_WIN32) || defined(__CYGWIN__)
// Berechnet die FeldPosition in der Ausgabekonsole
// Windows-only
COORD calcConsoleFieldPosition(board* boardPointer, coord position){
  COORD tmpCOORD = boardPointer->bufferInfo.dwCursorPosition;
  tmpCOORD.Y += 2*position.y+2;
  tmpCOORD.X += (boardPointer->fieldSize+1)*position.x +1;
  return tmpCOORD;
}
#endif

void linkFields(board* boardPointer, coord position){
  field* fieldPointer = getFieldPointer(boardPointer, position);
  int count = 0;
  for(int direction = 0; direction < 8; ++direction){
    coord nextStep = knightMove(position, direction);
    if(checkBounds(boardPointer, nextStep)){
      fieldPointer->accessibleFields[count] = getFieldPointer(boardPointer, nextStep);
      count++;
    }
  }
  fieldPointer->accessibleFieldCount = count;
  fieldPointer->remainingAccessibleFieldCount = count;
}

// Feldpointer aus Koordinaten ermitteln
field* getFieldPointer(board* boardPointer, coord position){
  return &boardPointer->fields[boardPointer->width*position.y + position.x];  
}

// Ermittelt die 8 möglichen anspringbaren Felder, ausgehend vom derzeitigen Feld ...
// ... für 'direction' von 0 bis 7 im Uhrzeigersinn, beginnend oben rechts. 
// Dazu werden die 3 kleinsten Bits von 'direction' ausgewertet
coord knightMove(coord position, enum knightMoveDirection direction){
  int x = 1, y = 2;
  if((direction+1) & 2)x = 2, y = 1;
  if((direction+2) & 4)y *= -1;
  if(direction & 4)x *= -1;
  position.x += x;
  position.y += y;
  return position;
}

// Prüfung ob die Koordinate für das derzeitige Brett erlaubt ist
bool checkBounds(board* boardPointer, coord position){
  if(position.x < 0 || position.y < 0)return false;
  if(position.x >= boardPointer->width || position.y >= boardPointer->height)return false;
  return true;
}

// Sprungmarkierung setzten und Anzahl der Anspringbaren Felder reduzieren
void setFieldValue(field* fieldPointer, int value){
  fieldPointer->value = value;
  adjustNextAccessibleFieldsCount(fieldPointer, -1);
}

// Gegenfunktion zu setFieldValue um ein Feld zurückzusetzen
void resetFieldValue(field* fieldPointer){
  fieldPointer->value = 0;
  adjustNextAccessibleFieldsCount(fieldPointer, +1);
}

// Für alle anspringbaren Felder die Anzahl der Sprungmöglichkeiten anpassen, wenn das Feld angesprungen/zurückgesetzt wird
void adjustNextAccessibleFieldsCount(field* fieldPointer, int adjustmentAmount){
  for(int i = 0; i < fieldPointer->accessibleFieldCount; ++i)
    if(fieldPointer->accessibleFields[i] != NULL)
      fieldPointer->accessibleFields[i]->remainingAccessibleFieldCount += adjustmentAmount;
}

// Bestimmt das Zielfeld falls die -loop Option gewählt ist
void determineEndingField(board* boardPointer, parameters* param){
  if(param->loop)
    boardPointer->endingField = getFieldPointer(boardPointer, param->startingPos);
}

// Derzeitiges Brett in eine CSV (comma seperated values) schreiben
bool printBoardToCSVFile(board* boardPointer, const char* filename){
  if(boardPointer == NULL)
    return true;
  FILE* handle = fopen(filename, "w");
  if(handle == NULL)
    return true;
  for(int i = 0; i < boardPointer->height*boardPointer->width; ++i){
    fprintf(handle, "%d,", boardPointer->fields[i].value);
    if((i+1) % boardPointer->width == 0)
      fprintf(handle, "\n");
  }
  fclose(handle);
  return false;
}


// #####################################################################################
// ### Funktionen für die Initialiserung und Verarbeitung des Outputs.               ###
// #####################################################################################
 
// initiert den Ausgabestring
bool initOutputString(board* boardPointer){
  boardPointer->fieldSize = countDigits(boardPointer->width * boardPointer->height);
  boardPointer->outputHeight = (boardPointer->height + 1) *2;
  boardPointer->outputWidth = (boardPointer->width + 2) * (boardPointer->fieldSize + 1);
  boardPointer->requiredMemory = boardPointer->outputHeight * boardPointer->outputWidth;
  boardPointer->outputString = (char*)malloc(sizeof(char)*boardPointer->requiredMemory);
  if(boardPointer->outputString == NULL)
    return true;
  for(int y = 0; y < boardPointer->outputHeight; ++y){
    for(int x = 0; x < boardPointer->outputWidth; ++x)
      boardPointer->outputString[y * boardPointer->outputWidth + x] = ' ';
    boardPointer->outputString[(y + 1) * boardPointer->outputWidth -1] = '\n';
  }
  boardPointer->outputString[boardPointer->requiredMemory-1] = '\0';
  return false;
}

// Ermittelt die Anzahl der Ziffern die benötigt werden um einen positiven Integer darzustellen
int countDigits(int number){
  if(number == 0)
    return 0;
  else
    return 1 + countDigits(number/10);
}

// Zeichnet das Brett in den Ausgabestring
void drawOutputField(board* boardPointer){
  // Linien des Spielfeldes malen
  drawLine(boardPointer, 1, '\xC9', '\xCD', '\xCB', '\xBB');
  for(int y = 2; y < boardPointer->outputHeight; y += 2){
    drawLine(boardPointer, y, '\xBA', ' ', '\xBA', '\xBA');
    drawLine(boardPointer, y+1, '\xCC', '\xCD', '\xCE', '\xB9');
  }
  drawLine(boardPointer, boardPointer->outputHeight-1, '\xC8', '\xCD', '\xCA', '\xBC');
  // Schreibt Buchstaben am oberem Rand des Brettes
  char letter = 'a';
  for(int x = 0; x < boardPointer->width; ++x){
    setOutputFieldByStr(boardPointer, x, 0, 'a'+x);
    letter++;
  }
  // Schreibt Zahlen am rechtem Rand des Bretts
  for(int y = boardPointer->height; y > 0; --y)
    setOutputFieldByValue(boardPointer, boardPointer->width, boardPointer->height - y + 1, y);
}

// Schreibt eine Zeile mit den jeweiligen Trennungszeichen senkrecht in den Outputstring 
void drawLine(board* boardPointer, int lineNumber, char leftDelimiter, char contentPlaceholder, char middleDelimiter, char rightDelimiter){
  boardPointer->outputString[boardPointer->outputWidth*lineNumber] = leftDelimiter;
  for(int x = 1; x < boardPointer->outputWidth - (2*boardPointer->fieldSize) -2; ++x){
    boardPointer->outputString[boardPointer->outputWidth*lineNumber +x] = contentPlaceholder;
    if(x % (boardPointer->fieldSize+1) == 0)
      boardPointer->outputString[boardPointer->outputWidth*lineNumber +x] = middleDelimiter;
  }
  boardPointer->outputString[boardPointer->outputWidth*lineNumber + (boardPointer->outputWidth - (2*boardPointer->fieldSize) -2)] = rightDelimiter;
}

// Feld an Position (x,y) im Ausgabestring direkt mit einem char beschreiben
void setOutputFieldByStr(board* boardPointer, int x, int y, const char chr){
  char buffer[boardPointer->fieldSize+1];
  sprintf(buffer, "%*c", boardPointer->fieldSize, chr);
  char* location = boardPointer->outputString + boardPointer->outputWidth*(2*y) + (boardPointer->fieldSize+1)*x + 1;
  strncpy(location, buffer, boardPointer->fieldSize);
}

// Feld an Position (x,y) im Ausgabestring direkt mit einem Wert beschreiben  
void setOutputFieldByValue(board* boardPointer, int x, int y, int value){
  char buffer[boardPointer->fieldSize+1];
  sprintf(buffer, "%*d", boardPointer->fieldSize, value);
  char* location = boardPointer->outputString + boardPointer->outputWidth*(2*y) + (boardPointer->fieldSize+1)*x + 1;
  strncpy(location, buffer, boardPointer->fieldSize);
}

// aktuallisiert den gesammten
void updateOutputString(board* boardPointer){
  for(int i = 0; i < boardPointer->height*boardPointer->width; ++i)
    updateOutputField(boardPointer, &boardPointer->fields[i]);
}
// Aktuallisiert den Wert des Feldes im Ausgabestring (outputField)
void updateOutputField(board* boardPointer, field* fieldPointer){
  char buffer[boardPointer->fieldSize +1];
  if(fieldPointer->value != 0)
    sprintf(buffer, "%*d", boardPointer->fieldSize, fieldPointer->value);
  else
    sprintf(buffer, "%*c", boardPointer->fieldSize, ' ');
  strncpy(fieldPointer->outputFieldLocation, buffer, boardPointer->fieldSize);
}

#if defined(_WIN32) || defined(__CYGWIN__)
// Gibt das Feld dynamisch neu aus
void reprintField(board* boardPointer, field* fieldPointer){
  SetConsoleCursorPosition(boardPointer->consoleHandle, fieldPointer->consoleFieldPosition);
  char buffer[boardPointer->fieldSize+1];
  strncpy(fieldPointer->outputFieldLocation, buffer, boardPointer->fieldSize);
  buffer[boardPointer->fieldSize] = '\0';
  printf("%s", buffer);
}
#endif


// #####################################################################################
// ### Eigentliche Lösungsfunktionen.                                                ###
// #####################################################################################

// Lösungsfunktion. Nimmt zu Lösendes Spielbrett und, Startfeld und Zähler entgegen
// Arbeitet sich rekursiv mit einem Backtracing-Verfahren durch
bool solve(board* boardPointer, field* currentField, int count){
  // Aufgabe C: Abbrechen wenn das Endefeld gesetzt ist
  if(boardPointer->endingField != NULL && boardPointer->endingField->remainingAccessibleFieldCount == 0)
    return false;
  // recursion "good end"; beendet die Funktion (oder ggf. dekrementiert den Permutationszähler)
  if(count == boardPointer->height*boardPointer->width){
    if(boardPointer->permutationCount > 0){
      boardPointer->permutationCount--;
      return false;
    }else{
      setAndReprintFieldValue(boardPointer, currentField, count);
      return true;
    }
  }
  // Abbruch wenn das Brett im derzeitigen Zustand nicht mehr lösbar ist
  if(checkFieldIsolation(boardPointer) || checkDeadEnd(boardPointer))
    return false;
  // Das Feld "setzen" und eine Rekursionsstufe tiefer gehen
  setAndReprintFieldValue(boardPointer, currentField, count);
  applyHeuristics(currentField);
  for(int direction = 0; direction < currentField->accessibleFieldCount; ++direction)
    if(currentField->accessibleFields[direction]->value == 0)
      if(solve(boardPointer, currentField->accessibleFields[direction], count+1))
        return true;
  // Wenn alle Möglichkeiten durchprobiert wurden und keine Lösung gefunden:
  // Feld zurücksetzen und eine Rekursionstufe zurückspringen
  setAndReprintFieldValue(boardPointer, currentField, 0);
  return false;
}

// Gibt dem Feld den Sprungwert (oder setzt ihn zurück)
// bei dynamischer Ausgabe wird das Feld neu ausgegeben
void setAndReprintFieldValue(board* boardPointer, field* fieldPointer, int value){
  if(value != 0)
    setFieldValue(fieldPointer, value);
  else
    resetFieldValue(fieldPointer);
  if(boardPointer->dynamicOutputDelay){
    updateOutputField(boardPointer, fieldPointer);
    #if defined(_WIN32) || defined(__CYGWIN__)
    reprintField(boardPointer, fieldPointer);
    Sleep(boardPointer->dynamicOutputDelay);
    #endif
  }
}

// Prüfung ob ein Feld isoliert (nicht mehr anspringbar) ist
bool checkFieldIsolation(board* boardPointer){
  for(int i = 0; i < boardPointer->width * boardPointer->height; ++i)
    if(boardPointer->fields[i].value == 0 && boardPointer->fields[i].remainingAccessibleFieldCount == 0)
      return true;
  return false;
}

// Es dürfen maximal 2 Felder nur noch eine Sprungoption haben (derzeitiges Feld und schließliches Endfeld) 
bool checkDeadEnd(board* boardPointer){
  int count = 0;
  for(int i = 0; i < boardPointer->width * boardPointer->height; ++i)
    if(boardPointer->fields[i].value == 0 && boardPointer->fields[i].remainingAccessibleFieldCount == 1)
      count++;
  return (count>2);  
}

// heurisische Priorisierung der anspringbaren Felder
// momentan Priorisierung nach Anzahl der übrigen Sprungoptionen
void applyHeuristics(field* fieldPointer){
  qsort(fieldPointer->accessibleFields, fieldPointer->accessibleFieldCount, sizeof(field*), compareFieldValues); 
}

// Vergleichsfunktion. Wird zur Sortierung nach restlich anspringbaren Felder des accessibleFields-Arrays benötigt
int compareFieldValues(const void* a, const void* b){
  field* field1 = *(field**)a;
  field* field2 = *(field**)b;
  if(field1->remainingAccessibleFieldCount < field2->remainingAccessibleFieldCount)return -1;
  if(field1->remainingAccessibleFieldCount > field2->remainingAccessibleFieldCount)return 1;
  return 0;
}
