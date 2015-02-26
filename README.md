pcsc-ctapi-wrapper
------------------

Ermoeglicht die Benutzung von Chipkartenlesern, die nur
ein PCSC-Interface haben (z.B. SCM SCR243 / Towitoko Chipdrive
PCMCIA) in Anwendungen, die nur CTAPI-Leser unerstuetzen
(z.B. Moneyplex, StarMoney on Linux). Der Wrapper ist ein CTAPI-
Modul, das die Funktionen in PCSC-Funktionen umwandelt. Es wird
also ein funktionierender Chipkartenleser mit PCSC-Interface
vorausgesetzt. Der Wrapper unterstuetzt die Eingabe der Pin am
Kartenleser NICHT. Es wird nur die Eingabe der Pin am PC
unterstuetzt.

Software-Voraussetzungen:
	- gcc
	- pcsc-lite
	- pcsc-lite includes/libraries

Installation:
	1. Den Wrapper mit "make" compilieren
	2. Den Wrapper mit "make install" (als root) installieren

Um den Wrapper zu nutzen, muss wie gesagt der pcscd laufen
und korrekt eingerichtet sein. 

Man kann nach der Installation dann z.B. in Moneyplex als CTAPI-
Treiber die Datei "/usr/local/lib/libpcsc-ctapi-wrapper.so"
verwenden, um auf den Chipkartenleser zuzugreifen. Sollten in 
PCSC-lite mehrere Kartenleser konfiguriert sein, wird der erste
verwendet, der gefunden wird.

In StarMoney on Linux muss der Wine-Wrapper ctapi32.dll benutzt
werden, der seinerseits den in die Registry eingetragenen Wrapper
libpcsc-ctapi-wrapper.so anzieht (siehe Beschreibung auf
http://www.starmoney.de/index.php?linux ). Sollten in PCSC-lite
mehrere Kartenleser konfiguriert sein, so kann ueber das
Chipkartenleser-Setup der gewuenschte Port eingestellt werden.

ACHTUNG: Der Wrapper wurde nur mit wenigen Kartenleser- und
Treiber-Konfigurationen getestet.
