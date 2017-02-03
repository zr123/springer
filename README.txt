Kompilierung:
	gcc chess.c -std=c99 -c
	gcc main.c -std=c99 chess.o -o springer.exe


Verwendung: 
	springer [options] <Startposition>
	Startposition zB. in der Form "A6" oder "f1" angeben

Die meisten Optionen haben mehrere Synonyme zur erleichterten Verwendung. Davon ist ein Synonym in der Regel die Kurzform.
Optionen:
  -h
  --h
  -help
  --help
	Im Program enthaltene Hilfe-Dokumentation anzeigen.

  -l
  -loop
	Geschlossenen Pfad berechnen. (Aufgabenteil C)

  -h=<Wert>
  -height=<Wert>
	Standardwert der Feldhöhe von 8 auf Wert ändern. Wert ohne <> eingeben.

  -w=<Wert>
  -width=<Wert>
	Standardwert der Feldbreite von 8 auf Wert ändern. Wert ohne <> eingeben.

  -d
  -dynamic
	Dynamische Ausgabe aktivieren. Achtung: Die wiederholte Ausgabe ist zeitintensiv und erhöht die Ausführungszeit erheblich.

  -csv=<Filename>
	Das fertig berechnete Feld nach der Berechnung als CSV nach Filename schreiben. Filename ohne <> eingeben.

  -p=<n>
  -permutation=<n>
	Ermittelt die n'te Permutation. Das Program berechnet statisch für jede Eingabe die selbe Lösung. Mit dieser Option können nachfolgende Lösungen vom Programm ermittelt und ausgegeben werden. Achtung: Hohe Werte für n verlangsamen das Programm erheblich.


Die Standardeinstellungen für diese Optionen sind in parameter.ini hinterlegt und können dort abgeändert werden.


Beispieleingaben:

Aufgabenteil (a) & (b): Berechung eines offenen Pfades.
springer.exe A1

Aufgabenteil (c): Berechnung eines geschlossenen Pfades.
springer.exe A1 -loop

Skaalierbarkeit: Das Brett ist dynamisch skaalierbar: Offener Pfad 35x35 Brett
springer.exe A1 -width=35 -height=35

Effizienz:
Der Algorithmus berechnet geschlossene Pfade für 12x12 Bretter sicher innerhalb von Millisekunden:
springer.exe A1 -w=12 -h=12 -l
Der Algorithmus berechnet geschlossene Pfade für 14x14 Bretter sicher innerhalb von Sekunden:
springer.exe A1 -w=14 -h=14 -l
Der Algorithmus berechnet geschlossene Pfade für (bis zu) 20x20 Bretter unsicher (idr. nicht für Rand/Eck-Felder) innerhalb von Millisekunden.
springer.exe E5 -w=20 -h=20 -l

Permutationen: Berechnung weiterer Lösungen für Aufgabenteil (c)
springer.exe A1 -loop -permutation=1
springer.exe A1 -loop -permutation=2
springer.exe A1 -loop -permutation=3
etc.

Korrektheit: Folgendes Eingaben berechnen alle 19724 ungerichteten geschlossenen Pfad für ein 6x6 Brett.
Letzter Pfad/Permutationen: (bis zu 30 Sekunden Rechenzeit)
springer.exe C3 -w=6 -h=6 -d -p=19723
Leeres Feld, da kein weiterer Pfad bzw weitere Permutation vorhanden: (bis zu 30 Sekunden Rechenzeit)
springer.exe C3 -w=6 -h=6 -d -p=19724

Ausgabe als CSV: Die Ausgabe kann zusätzlich als CSV-Datei gespeichert werden.
springer.exe A1 -csv=mein_file.csv

Dynamische Ausgabe: Auf Windows-System können die Berechnungsschritte in Echtzeit in der cmd nachverfolgt werden:
springer.exe A1 -w=10 -h=10 -l -dynamic
