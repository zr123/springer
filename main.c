#include <stdio.h>
#include <string.h>
#include <time.h>
#include "chess.h"

/* TODO
  - Sauberes Refactoring
  - Kommentieren
  - header ordentlich ordnen
  - Saubere Fehlerbehandlung und Input-Prüfung
  - Readme Schreiben
  - Windows/Linux Problematik
  - -e|end=
*/
int main(int argc, char* argv[]){
  // WIP: parameter struct kann eigentlich weg
  parameters* p = initParameters();
  if(loadParameterIni(p, "parameter.ini"))
    printf("Warnung: parameter.ini konnte nicht korrekt geladen werden.\n");
  // WIP
  if(handleCommandLineArguments(p, argc, argv))
    return 1;
  //p->startingPos = getRandomCoord(p->boardWidth, p->boardHeight);
  board* b = initBoard(p);
  if(b == NULL){
    printf("Initialisierungsfehler. Programmausführung abgebrochen.");
    return 1;
  }
  if(!checkBounds(b, p->startingPos)){
    printf("Ung\x81ltige Anfangsposition.\n");
    return 1;
  }
  
  if(p->dynamicOutput)
    printf(b->outputString);
  int timer = clock();
  solve(b, getFieldPointer(b, p->startingPos), 1);
  timer = clock()-timer;
  if(strlen(p->CSVfilename) > 0)
    printBoardToCSVFile(b, p->CSVfilename); 
  updateOutputString(b);
  if(!p->dynamicOutput)
    printf(b->outputString);
  printf("\nDone in %d ms.\n", timer);
  return 0;
}
