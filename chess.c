#include "chess.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32) || defined(__CYGWIN__)
#include <windows.h>
#endif

const char* helptext = 
"Verwendung: springer [optionen] <Startposition>\n\
Startposition zB. in der Form \"A6\" oder \"f1\" angeben \n\
Options:\n\
  --help                Diese Hilfe anzeigen\n\
  -l                    Geschlossenen Pfad berechnen\n\
  -h=<Wert>             Standardwert der Feldh\x94he \x84ndern\n\
  -w=<Wert>             Standardwert der Feldbreite \132ndern\n\
  -d                    Dynamische Ausgabe aktivieren (nur Windows)\n\
  -csv=<Filename>       Brett nach Berechnung als CSV nach Filename schreiben\n\
  -p=<n>                Ermittelt die n'te Permutation\n\
  ";
  
int handleCommandLineArguments(parameters* param, int argc, char* argv[]){
  if(param == NULL || argc == 1){
    printf("Fehlende Eingabeparameter. Hilfe mit --help\n");
    return 1;
  }
  for(int i = 1; i < argc; ++i)
    if(parseArgument(param, argv[i]))
      return 1;
  return 0;
}

parameters* initParameters(){
  parameters* param = (parameters*)calloc(sizeof(parameters), 1);
  if(param == NULL)
    return NULL;
  param->boardWidth = 8;
  param->boardHeight = 8;
  //param->startingPos_x = -1;
  //param->startingPos_y = -1;
  loadParameterIni(param, "init.ini");
  return param;
}

int loadParameterIni(parameters* param, const char* filename){
  FILE* handle = fopen(filename, "r");
  if(handle == NULL)return 1;
  fscanf(handle, "loop=%d\n", &param->loop);
  fscanf(handle, "height=%d\n", &param->boardHeight);
  fscanf(handle, "width=%d\n", &param->boardWidth);
  fscanf(handle, "dynamic=%d\n", &param->dynamicOutput);
  fscanf(handle, "csv=\"%[^\"]s\"\n", param->CSVfilename);
  fscanf(handle, "permutation=%d\n", &param->permutation);
  fclose(handle);
  return 0;
}

int parseArgument(parameters* param, const char* argument){
  if(!strncmp(argument, "-", 1)){
    if(!strcmp(argument, "-help") || !strcmp(argument, "--help") || !strcmp(argument, "-h") || !strcmp(argument, "--h")){
      printf("%s", helptext);
      return 1;
    }
    if(!strcmp(argument, "-loop") || !strcmp(argument, "-l")){
      param->loop = 1;
      return 0;
    }
    if(!strcmp(argument, "-dynamic") || !strcmp(argument, "-d")){
      param->dynamicOutput = 1;
      return 0;
    }
    if(sscanf(argument, "-w=%d", &param->boardWidth) || sscanf(argument, "-width=%d", &param->boardWidth))
      return 0;
    if(sscanf(argument, "-h=%d", &param->boardHeight) || sscanf(argument, "-height=%d", &param->boardHeight))
      return 0;
    if(sscanf(argument, "-csv=%s", param->CSVfilename))
      return 0;
    if(sscanf(argument, "-p=%d", &param->permutation))
      return 0;
    printf("Ung\x81ltige Option: %s\n", argument);
    return 1;
  }else{
    char posLetter;
    if(sscanf(argument, "%c%d", &posLetter, &param->startingPos_y) == 2){
      param->startingPos_x = convertLetterToPosition(posLetter);
      return 0;
    }else{
      printf("Ung\x81ltige Eingabe: %s\n", argument);
      return 1;
    }
  }
}

// den Buchstabenteil aus "A6" oder "f1" zu dem integer-Positionswert konvertieren
int convertLetterToPosition(char letter){
  int pos_x = letter - 97;
  if(pos_x < 0)pos_x = letter - 65;
  return pos_x;
}

// Prüfung ob ein Feld isoliert (nicht mehr anspringbar) ist
int checkFieldIsolation(board* boardPointer){
  for(int i = 0; i < boardPointer->width * boardPointer->height; ++i)
    if(boardPointer->fields[i].value == 0 && boardPointer->fields[i].remainingAccessibleFieldCount == 0)
      return 1;
  return 0;
}

// Es dürfen maximal 2 Felder nur noch eine Sprungoption haben (derzeitiges Feld und schließliches Endfeld) 
int checkDeadEnd(board* boardPointer){
  // eigentlich dürfen nur currentfield und endingfield remainingAccessibleFieldCount==1 haben
  int count = 0;
  for(int i = 0; i < boardPointer->width * boardPointer->height; ++i)
    if(boardPointer->fields[i].value == 0 && boardPointer->fields[i].remainingAccessibleFieldCount == 1)
      count++;
  return (count>2);  
}

// Feldpointer aus Koordinaten ermitteln
field* getFieldPointer(board* boardPointer, int x, int y){
  return &boardPointer->fields[boardPointer->width*y + x];  
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

// Derzeitiges Brett in eine CSV (comma seperated values) schreiben
int printBoardToCSVFile(board* boardPointer, const char* filename){
  if(boardPointer == NULL)
    return 1;
  FILE* handle = fopen(filename, "w");
  if(handle == NULL)
    return 1;
  for(int i = 0; i < boardPointer->height*boardPointer->width; ++i){
    fprintf(handle, "%d,", boardPointer->fields[i].value);
    if(i % boardPointer->width == 0)
      fprintf(handle, "\n");
  }
  fclose(handle);
  return 0;
}

// Prüfung ob die Koordinate für das derzeitige Brett erlaubt ist
int checkBounds(board* boardPointer, coord position){
  if(position.x < 0 || position.y < 0)return 0;
  if(position.x >= boardPointer->width || position.y >= boardPointer->height)return 0;
  return 1;
}

// Ermittelt die 8 möglichen anspringbaren Felder, ausgehend vom derzeitigen Feld ...
// ... für 'direction' von 0 bis 7 im Uhrzeigersinn, beginnend oben rechts. 
// Dazu werden die 3 kleinsten Bits von 'direction' ausgewertet
coord knightMove(coord position, int direction){
  int x = 1, y = 2;
  if((direction+1) & 2)x = 2, y = 1;
  if((direction+2) & 4)y *= -1;
  if(direction & 4)x *= -1;
  position.x += x;
  position.y += y;
  return position;
}

void linkFields(board* boardPointer, int x, int y){
  field* fieldPointer = getFieldPointer(boardPointer, x, y);
  int count = 0;
  for(int direction = 0; direction < 8; ++direction){
    coord coordinate = knightMove((coord){x, y}, direction);
    if(checkBounds(boardPointer, coordinate)){
      fieldPointer->accessibleFields[count] = getFieldPointer(boardPointer, coordinate.x, coordinate.y);;
      count++;
    }
  }
  fieldPointer->accessibleFieldCount = count;
  fieldPointer->remainingAccessibleFieldCount = count;
}

void initField(board* boardPointer, int pos_x, int pos_y){
  field* fieldPointer = getFieldPointer(boardPointer, pos_x, pos_y);
  fieldPointer->outputFieldLocation = boardPointer->outputString + boardPointer->outputWidth*(2*pos_y +2) + (boardPointer->fieldSize+1)*pos_x + 1;
  linkFields(boardPointer, pos_x, pos_y);
}

int initBoard(board* boardPointer, parameters* param){
  // Alles initieren
  boardPointer->height = param->boardHeight;
  boardPointer->width = param->boardWidth;
  boardPointer->fields = (field*)calloc(boardPointer->height*boardPointer->width, sizeof(field));
  if(boardPointer->fields == NULL)
    return 1;
  boardPointer->endingField = NULL;
  boardPointer->dynamicOutput = param->dynamicOutput;
  boardPointer->permutationCount = param->permutation;
  // schön machen start
  if(param->loop)
    boardPointer->endingField = getFieldPointer(boardPointer, param->startingPos_x, boardPointer->height - param->startingPos_y);
  // schön machen end
  if(boardPointer->dynamicOutput){
    #if defined(_WIN32) || defined(__CYGWIN__)
    boardPointer->consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(boardPointer->consoleHandle, &boardPointer->bufferInfo);
    #endif
  }
  if(initOutputField(boardPointer))
    return 1;
  drawOutputField(boardPointer);
  // link all fields accessible by a knights move to each other
  for(int x = 0; x < boardPointer->width; ++x)
    for(int y = 0; y < boardPointer->height; ++y)
      initField(boardPointer, x, y);
  return 0;
}

int countDigits(int number){
  if(number == 0)
    return 0;
  else
    return 1 + countDigits(number/10);
}

void drawLine(board* boardPointer, int lineNumber, char leftDelimiter, char contentPlaceholder, char middleDelimiter, char rightDelimiter){
  boardPointer->outputString[boardPointer->outputWidth*lineNumber] = leftDelimiter;
  for(int x = 1; x < boardPointer->outputWidth - (2*boardPointer->fieldSize) -2; ++x){
    boardPointer->outputString[boardPointer->outputWidth*lineNumber +x] = contentPlaceholder;
    if(x % (boardPointer->fieldSize+1) == 0)
      boardPointer->outputString[boardPointer->outputWidth*lineNumber +x] = middleDelimiter;
  }
  boardPointer->outputString[boardPointer->outputWidth*lineNumber + (boardPointer->outputWidth - (2*boardPointer->fieldSize) -2)] = rightDelimiter;
}

void drawOutputField(board* boardPointer){
  drawLine(boardPointer, 1, '\xC9', '\xCD', '\xCB', '\xBB');
  for(int y = 2; y < boardPointer->outputHeight; y += 2){
    drawLine(boardPointer, y, '\xBA', ' ', '\xBA', '\xBA');
    drawLine(boardPointer, y+1, '\xCC', '\xCD', '\xCE', '\xB9');
  }
  drawLine(boardPointer, boardPointer->outputHeight-1, '\xC8', '\xCD', '\xCA', '\xBC');
  char letter = 'a';
  for(int x = 0; x < boardPointer->width; ++x){
    setOutputFieldByStr(boardPointer, x, 0, 'a'+x);
    letter++;
  }
  for(int y = boardPointer->height; y > 0; --y)
    setOutputFieldByValue(boardPointer, boardPointer->width, boardPointer->height - y + 1, y);
}

int initOutputField(board* boardPointer){
  boardPointer->fieldSize = countDigits(boardPointer->width * boardPointer->height);
  boardPointer->outputHeight = (boardPointer->height + 1) *2;
  boardPointer->outputWidth = (boardPointer->width + 2) * (boardPointer->fieldSize + 1);
  boardPointer->requiredMemory = boardPointer->outputHeight * boardPointer->outputWidth;
  boardPointer->outputString = malloc(sizeof(char)*boardPointer->requiredMemory);
  if(boardPointer->outputString == NULL)
    return 1;
  for(int y = 0; y < boardPointer->outputHeight; ++y){
    for(int x = 0; x < boardPointer->outputWidth; ++x)
      boardPointer->outputString[y * boardPointer->outputWidth + x] = ' ';
    boardPointer->outputString[(y + 1) * boardPointer->outputWidth -1] = '\n';
  }
  boardPointer->outputString[boardPointer->requiredMemory-1] = '\0';
  return 0;
}

void updateOutputField(board* boardPointer, field* fieldPointer){
  char buffer[boardPointer->fieldSize+1];
  if(fieldPointer->value != 0)
    sprintf(buffer, "%*d", boardPointer->fieldSize, fieldPointer->value);
  else
    sprintf(buffer, "%*c", boardPointer->fieldSize, ' ');
  strncpy(fieldPointer->outputFieldLocation, buffer, boardPointer->fieldSize);
}

void setOutputFieldByValue(board* boardPointer, int x, int y, int value){
  char buffer[boardPointer->fieldSize+1];
  sprintf(buffer, "%*d", boardPointer->fieldSize, value);
  char* location = boardPointer->outputString + boardPointer->outputWidth*(2*y) + (boardPointer->fieldSize+1)*x + 1;
  strncpy(location, buffer, boardPointer->fieldSize);
}

void setOutputFieldByStr(board* boardPointer, int x, int y, const char chr){
  char buffer[boardPointer->fieldSize+1];
  sprintf(buffer, "%*c", boardPointer->fieldSize, chr);
  char* location = boardPointer->outputString + boardPointer->outputWidth*(2*y) + (boardPointer->fieldSize+1)*x + 1;
  strncpy(location, buffer, boardPointer->fieldSize);
}

// Vergleichsfunktion. Wird zur Sortierung nach restlich anspringbaren Felder des accessibleFields-Arrays benötigt
int compareFieldValues(const void* a, const void* b){
  field* field1 = *(field**)a;
  field* field2 = *(field**)b;
  if(field1->remainingAccessibleFieldCount < field2->remainingAccessibleFieldCount)return -1;
  if(field1->remainingAccessibleFieldCount > field2->remainingAccessibleFieldCount)return 1;
  return 0;
}

void updateOutputString(board* boardPointer){
  for(int i = 0; i < boardPointer->height*boardPointer->width; ++i)
    updateOutputField(boardPointer, &boardPointer->fields[i]);
}

int solve(board* boardPointer, field* currentField, int count){
  // Aufgabe C: Abbrechen wenn das Endefeld gesetzt ist
  if(boardPointer->endingField != NULL && boardPointer->endingField->remainingAccessibleFieldCount == 0)
    return 0;
  // recursion "good end"
  if(count == boardPointer->height*boardPointer->width)
    if(boardPointer->permutationCount > 0){
      boardPointer->permutationCount--;
      return 0;
    }else{
      setFieldValue(currentField, count);
      if(boardPointer->dynamicOutput){
        updateOutputField(boardPointer, currentField);
        SetConsoleCursorPosition(boardPointer->consoleHandle, boardPointer->bufferInfo.dwCursorPosition);
        printf("%s", boardPointer->outputString);
      }
      return 1;
    }
  //check if any field has been set to 0, or multiple have been set to 1
  if(checkFieldIsolation(boardPointer) || checkDeadEnd(boardPointer))
    return 0;
  // set field and next recursion level 
  setFieldValue(currentField, count);
  
  if(boardPointer->dynamicOutput){
    updateOutputField(boardPointer, currentField);
    SetConsoleCursorPosition(boardPointer->consoleHandle, boardPointer->bufferInfo.dwCursorPosition);
    printf("%s", boardPointer->outputString);
  }
  qsort(currentField->accessibleFields, currentField->accessibleFieldCount, sizeof(field*), compareFieldValues);
  for(int direction = 0; direction < currentField->accessibleFieldCount; ++direction)
    if(currentField->accessibleFields[direction]->value == 0){
      if(solve(boardPointer, currentField->accessibleFields[direction], count+1))
        return 1;
      // unnötig
      //if(currentField->accessibleFields[direction]->value == 2)
      //  break;
    }
  // recursion bad end -> reset field
  resetFieldValue(currentField);
  if(boardPointer->dynamicOutput)
    updateOutputField(boardPointer, currentField);
  return 0;
}
