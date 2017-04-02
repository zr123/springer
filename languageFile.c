#ifndef LANGUAGE_FILE
// Alle Texte sind hier als Konstanten hinterlegt
// Für eventuelle Übersetzungen und abweichende Koodierungen
#if defined(_WIN32) || defined(__CYGWIN__)
// Deutsch Windows CodePage OEM-852
const char* strLoadParameterIniFailed = "Warnung: parameter.ini konnte nicht korrekt geladen werden ...\n";
const char* strDefaultParameterIniCreated = "Default-parameter.ini erzeugt.\n";
const char* strDefaultParameterIniCreationFailed = "Default-parameter.ini konnte nicht erzeugt werden.";
const char* strMissingArguments = "Fehlende Eingabeparameter. Hilfe mit --help\n";
const char* strInvalidArgument = "Ung\x81ltiger Eingabeparameter %s. Hilfe mit --help\n";
const char* strInitializationFailed = "Initialisierungsfehler. Programmausf\x81hrung abgebrochen.";
const char* strInvalidStartingPosition = "Ung\x81ltige Anfangsposition.\n";
const char* strTimerDone = "\n\nBerechnung abgeschlossen in %d Millisekdunden.\n";
const char* strNoPathForInput = "F\x81r die Eingabe konnte kein Pfad gefunden werden.\n";
const char* helptext = 
"Verwendung: springer [optionen] <Startposition>\n\
Startposition zB. in der Form \"A6\" oder \"f1\" angeben \n\
Optionen:\n\
  --help                Diese Hilfe anzeigen\n\
  -l                    Geschlossenen Pfad berechnen\n\
  -h=<Wert>             Standardwert der Feldh\x94he \x84ndern\n\
  -w=<Wert>             Standardwert der Feldbreite \132ndern\n\
  -d                    Dynamische Ausgabe aktivieren (nur Windows)\n\
  -csv=<Filename>       Brett nach Berechnung als CSV nach Filename schreiben\n\
  -p=<n>                Ermittelt die n'te Permutation\n\n";

#else
// Deutsch non-Windows UTF-8
const char* strLoadParameterIniFailed = "Warnung: parameter.ini konnte nicht korrekt geladen werden ...\n";
const char* strDefaultParameterIniCreated = "Default-parameter.ini erzeugt.\n";
const char* strDefaultParameterIniCreationFailed = "Default-parameter.ini konnte nicht erzeugt werden.";
const char* strMissingArguments = "Fehlende Eingabeparameter. Hilfe mit --help\n";
const char* strInvalidArgument = "Ungü1ltiger Eingabeparameter %s. Hilfe mit --help\n";
const char* strInitializationFailed = "Initialisierungsfehler. Programmausführung abgebrochen.";
const char* strInvalidStartingPosition = "Ungültige Anfangsposition.\n";
const char* strTimerDone = "\n\nBerechnung abgeschlossen in %d Millisekdunden.\n";
const char* strNoPathForInput = "Für die Eingabe konnte kein Pfad gefunden werden.\n";
const char* helptext = 
"Verwendung: springer [optionen] <Startposition>\n\
Startposition zB. in der Form \"A6\" oder \"f1\" angeben \n\
Optionen:\n\
  --help                Diese Hilfe anzeigen\n\
  -l                    Geschlossenen Pfad berechnen\n\
  -h=<Wert>             Standardwert der Feldhöhe ändern\n\
  -w=<Wert>             Standardwert der Feldbreite \132ndern\n\
  -d                    Dynamische Ausgabe aktivieren (nur Windows)\n\
  -csv=<Filename>       Brett nach Berechnung als CSV nach Filename schreiben\n\
  -p=<n>                Ermittelt die n'te Permutation\n\n";

#endif
#endif