#include "chess.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32) || defined(__CYGWIN__)
#include <windows.h>
#endif

const char* helptext = 
"Verwendung: springer [options] <Startposition>\n\
Startposition zB. in der Form \"A6\" oder \"f1\" angeben \n\
Options:\n\
  --help                Diese Hilfe anzeigen\n\
  -l                    Geschlossenen Pfad berechnen\n\
  -h=<Wert>             Standardwert der Feldh\x94he \x84ndern\n\
  -w=<Wert>             Standardwert der Feldbreite \132ndern\n\
  -d                    Dynamische Ausgabe aktivieren (nur Windows)\n\
  -csv=<Filename>       Feld nach Berechnung als CSV nach Filename schreiben\n\
  -p=<n>                Ermittelt die n'te Permutation\n\
  ";
  
int handleCommandLineArguments(parameters* param, int argc, char* argv[]){
  initParameters(param);
  loadParameterIni(param, "init.ini");
  if(argc == 1)
    printf("Fehlende Eingabeparameter. Hilfe mit --help\n");
  for(int i = 1; i < argc; ++i){
    if(!strncmp(argv[i], "-", 1)){
      parseArgument(param, argv[i]);
    }else{
      char posLetter;
      int posNumber;
      if(sscanf(argv[i], "%c%d", &posLetter, &posNumber) == 2){
        param->startingPos_x = convertLetterToPosition(posLetter);
        param->startingPos_y = posNumber;
      }
    }
  }
  return 0;
}

void initParameters(parameters* param){
  strcpy(param->CSVfilename, "");
  param->boardWidth = 8;
  param->boardHeight = 8;
  param->startingPos_x = -1;
  param->startingPos_y = -1;
  param->loop = 0;
  param->dynamicOutput = 0;
  param->permutation = 0;
  // inifile?
}

void loadParameterIni(parameters* param, const char* filename){
  FILE* handle = fopen(filename, "r");
  int intbuffer;
  fscanf(handle, "loop=%d\n", &intbuffer);
  param->loop = intbuffer;
  fscanf(handle, "height=%d\n", &intbuffer);
  param->boardHeight = intbuffer;
  fscanf(handle, "width=%d\n", &intbuffer);
  param->boardWidth = intbuffer;
  fscanf(handle, "dynamic=%d\n", &intbuffer);
  param->dynamicOutput = intbuffer;
  char filenameBuffer[256] = "";
  fscanf(handle, "csv=\"%[^\"]s\"\n", filenameBuffer);
  strcpy(param->CSVfilename, filenameBuffer);
  fscanf(handle, "permutation=%d\n", &intbuffer);
  param->permutation = intbuffer;
}

int parseArgument(parameters* param, const char* argument){
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
  int customWidth;
  if(sscanf(argument, "-w=%d", &customWidth) == 1 || sscanf(argument, "-width=%d", &customWidth) == 1){
    param->boardWidth = customWidth;
    return 0;
  }
  int customHeight;
  if(sscanf(argument, "-h=%d", &param->boardHeight) == 1 || sscanf(argument, "-height=%d", &param->boardHeight) == 1)
    return 0;
  if(sscanf(argument, "-csv=%s", &param->CSVfilename))
    return 0;
  if(sscanf(argument, "-p=%d", &param->permutation) == 1)
    return 0;
  return 1;
}

// den Buchstabenteil aus "A6" oder "f1" zu dem integer-Positionswert konvertieren
int convertLetterToPosition(char letter){
  int pos_x = letter - 97;
  if(pos_x < 0)pos_x = letter - 65;
  return pos_x;
}

// Prüfung ob das Feld in einem Zustand ist in dem keine Lösung mehr möglich ist
int checkDeadEnd(board* boardPointer){
  /*
  for(int i = 0; i < boardPointer->width * boardPointer->height; ++i)
    if(boardPointer->fields[i].value == 0 && boardPointer->fields[i].remainingAccessibleFieldCount == 0)
      return 2;
  return 0;
  */
  int count = 0;
  for(int i = 0; i < boardPointer->width * boardPointer->height; ++i)
    if(boardPointer->fields[i].value == 0 && boardPointer->fields[i].remainingAccessibleFieldCount == 1)
      count++;
  return count;
  
}

field* getFieldPointer(board* boardPointer, int x, int y){
  return &boardPointer->fields[boardPointer->width*y + x];  
}

void setFieldValue(field* fieldPointer, int value){
  fieldPointer->value = value;
  for(int i = 0; i < fieldPointer->accessibleFieldCount; ++i){
    field* nextField = fieldPointer->accessibleFields[i];
    if(nextField != NULL)
      nextField->remainingAccessibleFieldCount -= 1;
  }
}

void resetFieldValue(field* fieldPointer){
  fieldPointer->value = 0;
  for(int i = 0; i < fieldPointer->accessibleFieldCount; ++i){
    field* nextField = fieldPointer->accessibleFields[i];
    if(nextField != NULL)
      nextField->remainingAccessibleFieldCount += 1;
  }
}

void printBoardToCSVFile(board* boardPointer, const char* filename){
  if(filename == NULL || strlen(filename) == 0)return;
  FILE* handle = fopen(filename, "w");
  for(int y = boardPointer->height-1; y >= 0; --y){
    fprintf(handle, "%d", getFieldPointer(boardPointer, 0, y)->value); 
    for(int x = 1; x < boardPointer->width; ++x){
      fprintf(handle, ";%d", getFieldPointer(boardPointer, x, y)->value); 
    }
    fprintf(handle, "\n");
  }
}

int checkBounds(board* boardPointer, int pos_x, int pos_y){
  if(pos_x < 0 || pos_y < 0)return 0;
  if(pos_x >= boardPointer->width || pos_y >= boardPointer->height)return 0;
  return 1;
}

field* knightMove(board* boardPointer, int pos_x, int pos_y, int direction){
  // Bitte einen Moment inne halten und bewundern wie elegant ich das hier gelöst habe.
  // from 0 to 7 clockwise, starting with "upRight" (2 Up, 1 Right)
  int x = 1, y = 2;
  if((direction+1) & 2)x = 2, y = 1;
  if((direction+2) & 4)y *= -1;
  if(direction & 4)x *= -1;
  pos_x += x;
  pos_y += y;
  if(checkBounds(boardPointer, pos_x, pos_y))
    return getFieldPointer(boardPointer, pos_x, pos_y);
  else
    return NULL;
}

void linkFields(board* boardPointer, int x, int y){
  field* fieldPointer = getFieldPointer(boardPointer, x, y);
  int count = 0;
  for(int direction = 0; direction < 8; ++direction){
    field* nextField = knightMove(boardPointer, x, y, direction);
    if(nextField != NULL){
      fieldPointer->accessibleFields[count] = nextField;
      count++;
    }
  }
  fieldPointer->accessibleFieldCount = count;
  fieldPointer->remainingAccessibleFieldCount = count;
}

void initField(board* boardPointer, int pos_x, int pos_y){
  field* fieldPointer = getFieldPointer(boardPointer, pos_x, pos_y);
  fieldPointer->value = 0;
  fieldPointer->x = pos_x;
  fieldPointer->y = pos_y;
  for(int i = 0; i < 8; ++i)
    fieldPointer->accessibleFields[i] = NULL;
  linkFields(boardPointer, pos_x, pos_y);
}

int initBoard(board* boardPointer, parameters* param){
  // Alles initieren
  boardPointer->height = param->boardHeight;
  boardPointer->width = param->boardWidth;
  boardPointer->fields = (field*)malloc(sizeof(field)*boardPointer->height*boardPointer->width);
  if(boardPointer->fields == NULL)
    return 1;
  boardPointer->endingField = NULL;
  boardPointer->dynamicOutput = param->dynamicOutput;
  boardPointer->permutationCount = param->permutation;
  if(boardPointer->dynamicOutput){
    #if defined(_WIN32) || defined(__CYGWIN__)
    boardPointer->consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(boardPointer->consoleHandle, &boardPointer->bufferInfo);
    #endif
  }
  // link all fields accessible by a knights move to each other
  for(int x = 0; x < boardPointer->width; ++x)
    for(int y = 0; y < boardPointer->height; ++y)
      initField(boardPointer, x, y);
  if(initOutputField(boardPointer))
    return 1;
  drawOutputField(boardPointer);
  return 0;
}

int countDigits(int number){
  if(number == 0){
    return 0;
  }else{
    return 1 + countDigits(number/10);
  }
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
    setOutputFieldByStr(boardPointer, x, 0, letter);
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

// Vergleichsfunktion. Wird zur Sortierung des accessibleFieldCount-Arrays benötigt
int compareFieldValues(const void* a, const void* b){
  field* field1 = *(field**)a;
  field* field2 = *(field**)b;
  if(field1->remainingAccessibleFieldCount < field2->remainingAccessibleFieldCount)return -1;
  if(field1->remainingAccessibleFieldCount > field2->remainingAccessibleFieldCount)return 1;
  if(field1->remainingAccessibleFieldCount == field2->remainingAccessibleFieldCount)return 0;
}

void updateOutputString(board* boardPointer){
  for(int y = 0; y < boardPointer->height; ++y)
    for(int x = 0; x < boardPointer->width; ++x){
      field* fp = getFieldPointer(boardPointer, x, y);
      setOutputFieldByValue(boardPointer, x, y+1, fp->value);
    }
}



/*
Optimisierung:
  Bei der Wahl der Route:
    - (wenn ein ziel eine möglichkeit hat und Count != 35 -> suicide)
    - wenn ein ziel 2 möglichkeiten hat -> anspringen und restliche möglichkeiten killen
    - wenn zwei oder mehr ziele nur 2 möglichkeiten haben -> suicide
*/
int solve(board* boardPointer, field* currentField, int count){
  // Aufgabe C: Abbrechen wenn das Endefeld gesetzt ist
  if(boardPointer->endingField != NULL && boardPointer->endingField->remainingAccessibleFieldCount == 0)
    return 0;
  // recursion "good end"
  if(count == boardPointer->height*boardPointer->width){
    if(boardPointer->permutationCount > 0){
      boardPointer->permutationCount--;
      return 0;
    }else{
      setFieldValue(currentField, count);
      return 1;
    }
  }
  //check if any field has been set to 0, or multiple have been set to 1
  if(checkDeadEnd(boardPointer) > 2){
    return 0;
  }
  // set field and next recursion level 
  setFieldValue(currentField, count);
  
  if(boardPointer->dynamicOutput){
    setOutputFieldByValue(boardPointer, currentField->x, currentField->y+1, count);
    SetConsoleCursorPosition(boardPointer->consoleHandle, boardPointer->bufferInfo.dwCursorPosition);
    printf("%s", boardPointer->outputString);
  }
  qsort(currentField->accessibleFields, currentField->accessibleFieldCount, sizeof(field*), compareFieldValues);
  for(int direction = 0; direction < currentField->accessibleFieldCount; ++direction){
    if(currentField->accessibleFields[direction]->value == 0){
      if(solve(boardPointer, currentField->accessibleFields[direction], count+1))
        return 1;
      if(currentField->accessibleFields[direction]->value == 2)
        break;
    }
  }
  // recursion bad end -> reset field
  resetFieldValue(currentField);
  if(boardPointer->dynamicOutput)
    setOutputFieldByStr(boardPointer, currentField->x, currentField->y+1, ' ');
  return 0;
}