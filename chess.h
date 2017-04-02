#ifndef CHESS_H
#define CHESS_H
#include <stdbool.h>
#if defined(_WIN32) || defined(__CYGWIN__)
#include <windows.h>
#endif


// #####################################################################################
// ### Datentypen                                                                    ###
// #####################################################################################

// Um für die Funktion "knightMove" die Sprungichtung anzugeben
enum knightMoveDirection{
  upRight,
  rightUp,
  rightDown,
  downRight,
  downLeft,
  leftDown,
  leftUp,
  upLeft
};

// Koordinaten-struct
typedef struct coord{
  int x;
  int y;
} coord;

// Feld aus dem das Schachbrett besteht
typedef struct field{
  int value;                          // Markierung; Als wievieltes Feld es angesprungen wurde
  int remainingAccessibleFieldCount;  // Zählvariable für die übrigen, anspringbaren Felder
  int accessibleFieldCount;           // Gesamtanzahl der anspringbaren Felder. entspricht 8 außer für Rand- und Eckfelder
  struct field* accessibleFields[8];  // Pointer zu den anspringbaren Feldern.
  char* outputFieldLocation;          // relative Position im Outputstring, an dem das Feld positioniert ist
  #if defined(_WIN32) || defined(__CYGWIN__)
  COORD consoleFieldPosition;         // Feldposition in der Windows Konsolen-API
  #endif
}field;

// In dieser Struktur werden die Programm-Parameter aus Ini-File/Kommandozeile gesammelt
typedef struct parameters{
  char CSVfilename[256];  // Name des Ausgabe-CSV Files
  int boardWidth;         // Feldbreiteparameter
  int boardHeight;        // Feldhöheparameter
  coord startingPos;      // Startposition
  int helpFlag;           // Flag für die Ausgabe der Hilfe
  int loop;               // Flag für geschlossene Pfade
  int dynamicOutputDelay; // Flag für dynamische Ausgabe
  int permutation;        // n'te Permutation die ermittelt werden soll
  int autoRandom;         // Flag um Automatische Auswahl eines zufälligen Feldes zu aktivieren / deaktivieren
}parameters;

typedef struct board{
  // Variablen für die Berechnung
  field* fields;          // "Array" für die Felder des Schachbrettes
  int width;              // Breite des Schachbrettes (Standardwert 8)
  int height;             // Höhe des Schachbrettes (Standardwert 8)
  field* endingField;     // Referenz des gewünschten Zielfeldes
  int permutationCount;   // n'te Permutation die ermittelt werden soll
  // Variablen für den output
  int fieldSize;          // Anzahl der benötigten char um einen Feldwerd darzustellen (Anzahl der Ziffern des höchstmöglichen Wertes)
  int outputHeight;       // tatsächliche Höhe des outputStrings
  int outputWidth;        // tatsächliceh Breite des outputStrings
  int requiredMemory;     // Speicherbedarf des outputStrings
  char* outputString;     // Ausgabestring
  int dynamicOutputDelay; // Flag für dynamische Ausgabe
  #if defined(_WIN32) || defined(__CYGWIN__)
  HANDLE consoleHandle;                   // handle für die Windows Konsolen API
  CONSOLE_SCREEN_BUFFER_INFO bufferInfo;  // Informationsbuffer für Windows Konsole
  #endif
} board;


// #####################################################################################
// ### Funktionen für Parameter, Konselenargumente und deren Ininitialisierung, etc. ###
// #####################################################################################

parameters* initParameters();
bool loadParameterIni(parameters* param);
bool createDefaultParameterIni();
int handleCommandLineArguments(parameters* param, int argc, char* argv[]);
bool parseArgument(parameters* param, const char* argument);
bool checkFlags(parameters* param, const char* argument);
bool checkArgumentFlag(const char* argument, const char* pattern);
bool checkArgumentValues(parameters* param, const char* argument);
bool checkArgumentCoordinate(parameters* param, const char* argument);
int convertLetterToPosition(char letter);
void normalizeCoordinate(coord* coordinate, int boardHeight);
coord getRandomCoord(int max_x, int max_y);


// #####################################################################################
// ### Initierungsfunktionen für das Spielbrett + Allgemeine Board-Funktionen.       ###
// #####################################################################################

board* initBoard(int width, int height);
void initAllFields(board* boardPointer);
void initField(board* boardPointer, coord position);
char* calcOutputFieldLocation(board* boardPointer, coord position);
#if defined(_WIN32) || defined(__CYGWIN__)
COORD calcConsoleFieldPosition(board* boardPointer, coord position);
#endif
void linkFields(board* boardPointer, coord position);
field* getFieldPointer(board* boardPointer, coord position);
coord knightMove(coord coordinate, enum knightMoveDirection direction);
bool checkBounds(board* boardPointer, coord position);
void setFieldValue(field* fieldPointer, int value);
void resetFieldValue(field* fieldPointer);
void adjustNextAccessibleFieldsCount(field* fieldPointer, int adjustmentAmount);
void determineEndingField(board* boardPointer, parameters* param);
bool printBoardToCSVFile(board* boardPointer, const char* filename);


// #####################################################################################
// ### Funktionen für die Initialiserung und Verarbeitung des Outputs.               ###
// #####################################################################################
 
bool initOutputString(board* boardPointer);
int countDigits(int number);
void drawOutputField(board* out);
void drawLine(board* out, int lineNumber, char leftDelimiter, char contentPlaceholder, char middleDelimiter, char rightDelimiter);
void setOutputFieldByStr(board* out, int x, int y, const char chr);
void setOutputFieldByValue(board* out, int x, int y, int value);
void updateOutputString(board* boardPointer);
void updateOutputField(board* boardPointer, field* fieldPointer);
void reprintField(board* boardPointer, field* fieldPointer);


// #####################################################################################
// ### Eigentliche Lösungsfunktionen.                                                ###
// #####################################################################################

bool solve(board* boardPointer, field* currentField, int count); 
void setAndReprintFieldValue(board* boardPointer, field* fieldPointer, int value);
bool checkFieldIsolation(board* boardPointer);
bool checkDeadEnd(board* boardPointer);
void applyHeuristics(field* fieldPointer);
int compareFieldValues(const void* a, const void* b);

#endif
