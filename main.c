#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "chess.h"
#include "languageFile.c"

/*
  Kleinigkeiten die noch verbessert werden könnten:
    Problem 1:
      - Parameter und Kommandozeilenargumente werden Hart gecoded ausgewertet.
      - Schön wäre eine dynamischere Auswertung der .ini und Argumente.
    Lösung:
      - Eigenens Struct für die Parameter, mit name, typ (flag oder wert) und Synonymliste.
        -> Das Handling von Parametern und Kommandozeilenargumente modularisieren.
    Problem 2:
      - Tests fehlen leider vollständig.
    Lösung:
      - Tests schreiben. Ein großtteil der Funktionen ist in einen gut Unit-testbaren Zustand.
    Problem 3:
      - Anpassungen für *nix Systeme fehlen.
    Lösung:
      - Machen.
    
    Features die noch gemacht werden könnten:
      -e|end=xx Argument um das Endfeld direkt zu setzten
      -ein Argument/Parameter um die Konsolenausgabe komplett abzuschalten
      -ein Teile-und-Herrsche Algorythmus auf Basis des bestehenden für SEHR große Felder
*/

int prepareParameters();

int main(int argc, char* argv[]){
  // parameter für die funktion ermitteln
  parameters* p = initParameters();
  if(loadParameterIni(p)){
    printf(strLoadParameterIniFailed);
    if(createDefaultParameterIni())
      printf("%s", strDefaultParameterIniCreated);
    else
      printf("%s", strDefaultParameterIniCreationFailed);
  } 
  if(argc == 1 && !p->autoRandom){
    printf("%s", strMissingArguments);
    return 1;
  }
  int e = handleCommandLineArguments(p, argc, argv);
  if(e > 0){
    printf(strInvalidArgument, argv[e]);
    return 1;
  }else{
    normalizeCoordinate(&p->startingPos, p->boardHeight);
  }
  if(p->helpFlag){
    printf("%s", helptext);
    return 0;
  }
  if(p->autoRandom && p->startingPos.x == 0 && p->startingPos.y == p->boardHeight)
    p->startingPos = getRandomCoord(p->boardWidth, p->boardHeight);
  
  // Board initieren 
  board* b = initBoard(p->boardWidth, p->boardHeight);
  if(b == NULL){
    printf("%s", strInitializationFailed);
    return 1;
  }
  determineEndingField(b, p);
  b->dynamicOutputDelay = p->dynamicOutputDelay;
  b->permutationCount = p->permutation;
  if(!checkBounds(b, p->startingPos)){
    printf("%s", strInvalidStartingPosition);
    return 1;
  }
  
  // 
  if(p->dynamicOutputDelay)
    printf("%s", b->outputString);
  int timer = clock();
  bool success = solve(b, getFieldPointer(b, p->startingPos), 1);
  timer = clock()-timer;
  if(strlen(p->CSVfilename) > 0)
    printBoardToCSVFile(b, p->CSVfilename); 
  
  // Ergebnis ausgeben
  #if defined(_WIN32) || defined(__CYGWIN__)
  if(p->dynamicOutputDelay) 
    SetConsoleCursorPosition(b->consoleHandle, b->bufferInfo.dwCursorPosition);
  #endif
  updateOutputString(b);
  printf("%s", b->outputString);
  printf(strTimerDone, timer);
  if(success == false)
    printf("%s", strNoPathForInput);
  
  return 0;
}
