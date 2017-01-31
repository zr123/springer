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
  parameters* p = initParameters();
  if(handleCommandLineArguments(p, argc, argv))
    return 1;
  board b;
  if(initBoard(&b, p)){
    printf("Initialisierungsfehler. Programmausführung abgebrochen.");
    return 1;
  }
  if(!checkBounds(&b, (coord){p->startingPos_x, p->startingPos_y-1})){
    printf("Ung\x81ltige Anfangsposition.\n", p->startingPos_x, p->startingPos_y);
    return 1;
  }
  
  int timer = clock();
  solve(&b, getFieldPointer(&b, p->startingPos_x, b.height - p->startingPos_y), 1);
  timer = clock()-timer;
  if(strlen(p->CSVfilename) > 0)
    printBoardToCSVFile(&b, p->CSVfilename); 
  updateOutputString(&b);
  if(!p->dynamicOutput)
    printf(b.outputString);
  printf("\nDone in %d ms.\n", timer);
  return 0;
}
