#ifndef CHESS_H
#define CHESS_H 
#if defined(_WIN32) || defined(__CYGWIN__)
#include <windows.h>
#endif

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

// forward deklaration damit sich field selbst referenzieren kann
typedef struct field field;
// Feld aus dem das Schachbrett besteht
struct field{
  int x;                              // x-Position des Feldes
  int y;                              // y-Position des Feldes
  int value;                          // Wert; Als wievieltes Feld es angesprungen wurde
  int remainingAccessibleFieldCount;  // Zählvariable für die übrigen, anspringbaren Felder
  int accessibleFieldCount;           // Gesamtanzahl der anspringbaren Felder. =8 außer für Rand- und Eckfelder
  field* accessibleFields[8];         // Pointer zu den anspringbaren Feldern.
};

// In dieser struktur werden die Parameter aus der Kommandozeile gesammelt
typedef struct parameters{
  char CSVfilename[256];  // Name des Ausgabe-CSV Files
  int boardWidth;         // Feldbreiteparameter
  int boardHeight;        // Feldhöheparameter
  int startingPos_x;      // Parameter für Startposition x-Koordinate
  int startingPos_y;      // Parameter für Startposition y-Koordinate
  int loop;               // Flag für geschlossene Pfade
  int dynamicOutput;      // Flag für dynamische Ausgabe
  int permutation;        // n'te Permutation die ermittelt werden soll
}parameters;

typedef struct board{
  field* fields;
  int height;             // Höhe des Schachfeldes (Standardwert 8)
  int width;              // Breite des Schachfeldes (Standardwert 8)
  field* endingField;     // Referenz des gewünschten Zielfeldes
  int permutationCount;   // n'te Permutation die ermittelt werden soll
  // output stuff
  int fieldSize;          // Anzahl der benötigten char um einen Feldwerd darzustellen (Anzahl der Ziffern des höchstmöglichen Wertes)
  int outputHeight;       // tatsächliche Höhe des outputStrings
  int outputWidth;        // tatsächliceh Breite des outputStrings
  int requiredMemory;     // Speicherbedarf des outputStrings
  char* outputString;     // Ausgabestring
  int dynamicOutput;      // Flag für dynamische Ausgabe
  #if defined(_WIN32) || defined(__CYGWIN__)
  HANDLE consoleHandle;
  CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
  #endif
} board;

int handleCommandLineArguments(parameters* param, int argc, char* argv[]);
void initParameters(parameters* param);
void loadParameterIni(parameters* param, const char* filename);
int parseArgument(parameters* param, const char* argument);
int convertLetterToPosition(char letter);

void printBoardToCSVFile(board* boardPointer, const char* filename);
field* getFieldPointer(board* boardPointer, int x, int y);

int initBoard(board* boardPointer, parameters* param);
void initField(board* boardPointer, int pos_x, int pos_y);
void linkFields(board* boardPointer, int x, int y);
field* knightMove(board* boardPointer, int pos_x, int pos_y, int direction);
int checkBounds(board* boardPointer, int pos_x, int pos_y);

int initOutputField(board* boardPointer);
int countDigits(int number);
void drawOutputField(board* out);
void drawLine(board* out, int lineNumber, char leftDelimiter, char contentPlaceholder, char middleDelimiter, char rightDelimiter);
void setOutputFieldByStr(board* out, int x, int y, const char chr);
void setOutputFieldByValue(board* out, int x, int y, int value);

int solve(board* boardPointer, field* currentField, int count);
void setFieldValue(field* fieldPointer, int value);
int checkDeadEnd(board* boardPointer);
int compareFieldValues(const void* a, const void* b);

void updateOutputString(board* boardPointer);

#endif