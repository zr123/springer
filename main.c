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
  - braucht field wirklich sein x und y?
  - -s|seperator
*/
int main(int argc, char* argv[]){
  parameters p;
  board b;

  if(handleCommandLineArguments(&p, argc, argv)){
    printf("Fehlende Eingabeparameter. Hilfe mit --help\n");
    return 1;
  }
  if(initBoard(&b, &p)){
    printf("Initialisierungsfehler. Programmausführung abgebrochen.");
    return 1;
  }
  // Buggy, fix => analysiert position noch nicht korrekt
  //if(!checkBounds(&b, p.startingPos_x, p.startingPos_y))
  //  return 1;
  
  int startingClock = clock();
  if(p.startingPos_x >= 0 && p.startingPos_y >= 0){
    if(p.loop)
      b.endingField = getFieldPointer(&b, p.startingPos_x, b.height - p.startingPos_y);
    solve(&b, getFieldPointer(&b, p.startingPos_x, b.height - p.startingPos_y), 1);
    if(strlen(p.CSVfilename) > 0)
      printBoardToCSVFile(&b, p.CSVfilename); 
    updateOutputString(&b);
    if(!p.dynamicOutput)
      printf(b.outputString);
    printf("\nDone in %d ms.\n", clock()-startingClock);
  }
  return 0;
}
