#ifndef CHESS_H
#define CHESS_H 
#if defined(_WIN32) || defined(__CYGWIN__)
#include <windows.h>
#endif

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

// forward declaration damit sich field selbst referenzieren kann
typedef struct field field;

// Feld aus dem das Schachbrett besteht
struct field{
  int value;                          // Markierung; Als wievieltes Feld es angesprungen wurde
  int remainingAccessibleFieldCount;  // Zählvariable für die übrigen, anspringbaren Felder
  int accessibleFieldCount;           // Gesamtanzahl der anspringbaren Felder. entspricht 8 außer für Rand- und Eckfelder
  field* accessibleFields[8];         // Pointer zu den anspringbaren Feldern.
  char* outputFieldLocation;          // relative Position im Outputstring, an dem das Feld positioniert ist
  #if defined(_WIN32) || defined(__CYGWIN__)
  COORD consoleFieldPosition;         // Feldposition in der Windows cmd-API
  #endif
};

// In dieser Struktur werden die Programm-Parameter aus Ini-File/Kommandozeile gesammelt
typedef struct parameters{
  char CSVfilename[256];  // Name des Ausgabe-CSV Files
  int boardWidth;         // Feldbreiteparameter
  int boardHeight;        // Feldhöheparameter
  coord startingPos;      // Startposition
  //coord endingPos;        // Zielkoordinate
  int loop;               // Flag für geschlossene Pfade
  int dynamicOutput;      // Flag für dynamische Ausgabe
  int permutation;        // n'te Permutation die ermittelt werden soll
  int autoRandom;         // Flag um Automatische Auswahl eines zufälligen Feldes zu aktivieren / deaktivieren
  int disableOutput;      // Flag um den Output zu deaktivieren
}parameters;

typedef struct board{
  field* fields;          // "Array" für die Felder des Schachbrettes
  int height;             // Höhe des Schachfeldes (Standardwert 8)
  int width;              // Breite des Schachfeldes (Standardwert 8)
  field* endingField;     // Referenz des gewünschten Zielfeldes
  int permutationCount;   // n'te Permutation die ermittelt werden soll
  // output stuff
  int disableOutput;      // Flag um den Output zu deaktivieren
  int fieldSize;          // Anzahl der benötigten char um einen Feldwerd darzustellen (Anzahl der Ziffern des höchstmöglichen Wertes)
  int outputHeight;       // tatsächliche Höhe des outputStrings
  int outputWidth;        // tatsächliceh Breite des outputStrings
  int requiredMemory;     // Speicherbedarf des outputStrings
  char* outputString;     // Ausgabestring
  int dynamicOutput;      // Flag für dynamische Ausgabe
  #if defined(_WIN32) || defined(__CYGWIN__)
  HANDLE consoleHandle;                   // handle für die Windows Konsolen API
  CONSOLE_SCREEN_BUFFER_INFO bufferInfo;  // Informationsbuffer für Windows Konsole
  #endif
} board;

int handleCommandLineArguments(parameters* param, int argc, char* argv[]);
parameters* initParameters();
int loadParameterIni(parameters* param, const char* filename);
int parseArgument(parameters* param, const char* argument);
int convertLetterToPosition(char letter);
coord getRandomCoord(int max_x, int max_y);

int printBoardToCSVFile(board* boardPointer, const char* filename);
field* getFieldPointer(board* boardPointer, coord position);

board* initBoard(parameters* param);
void initField(board* boardPointer, coord position);
void linkFields(board* boardPointer, coord position);
coord knightMove(coord coordinate, int direction);
int checkBounds(board* boardPointer, coord position);

int initOutputField(board* boardPointer);
int countDigits(int number);
void drawOutputField(board* out);
void drawLine(board* out, int lineNumber, char leftDelimiter, char contentPlaceholder, char middleDelimiter, char rightDelimiter);
void setOutputFieldByStr(board* out, int x, int y, const char chr);
void setOutputFieldByValue(board* out, int x, int y, int value);

int solve(board* boardPointer, field* currentField, int count);
void setFieldValue(field* fieldPointer, int value);
void resetFieldValue(field* fieldPointer);
void adjustNextAccessibleFieldsCount(field* fieldPointer, int adjustmentAmount);
int checkDeadEnd(board* boardPointer);
int compareFieldValues(const void* a, const void* b);

void updateOutputString(board* boardPointer);

#endif
