Notes by the author:
====================

pcsc-ctapi-wrapper
-------------------

Ermoeglicht die Benutzung von Chipkartenlesern, die nur
ein PCSC-Interface haben (z.B. SCM SCR243 / Towitoko Chipdrive
PCMCIA) in Anwendungen, die nur CTAPI-Leser unerstuetzen
(z.B. Moneyplex). Der Wrapper ist ein CTAPI-Modul, dass die
Funktionen in PCSC-Funktionen umwandelt. Es wird also ein
funktionierender Chipkartenleser mit PCSC-Interface vorausgesetzt.
Der Wrapper unterstuetzt die Eingabe der Pin am Kartenleser
NICHT. Es wird nur die Eingabe der Pin am PC unterstuetzt.

Software-Voraussetzungen:
	- gcc
	- pcsc-lite
	- pcsc-lite includes/libraries

Installation:
	1. Den Wrapper mit "make" compilieren
	2. Den Wrapper mit "make install" (als root) installieren

Um den Wrapper zu nutzen, muss wie gesagt der pcscd laufen
und korrekt eingerichtet sein. Man kann nach der Installation
dann z.B. in Moneyplex als CTAPI-Treiber die Datei
"/usr/local/lib/pcsc-ctapi-wrapper.so" verwenden, um auf den
Chipkartenleser zuzugreifen. Sollten in PCSC-lite mehrere
Kartenleser konfiguriert sein, wird der erste verwendet,
der gefunden wird.

ACHTUNG: Der Wrapper wurde nur mit Moneyplex, einer T1-HBCI-Karte
(Sparkasse), PCSC-lite 1.2.0 und dem SCM SCR243 (mit den ofiziellen
Linux-Treibern von SCM) getestet. Der Wrapper kann auch mit
anderen Konfigurationen laufen, wurde aber wie gesagt nicht
damit getestet. Wenn jemand es schafft, ihn mit anderer
Konfigration einzusetzen, ware eine kurze Meldung darueber an
<info@b1g.de> super, dann kann ich die Konfiguration in die
'getestet'-Liste uebernehmen.

Notes taken from http://www.matrica.de/download/chipdrive_usb_ubuntu.txt:
=========================================================================

--------------------------------------------------------------------
Anleitung freundlicherweise zusammengestellt von Herrn Florian Lauck
--------------------------------------------------------------------

Stand: 09/09/2007

Diese Anleitung beschreibt die Einrichtung eines Chipdrive micro pro (USB) auch bekannt als SCR335 von SCM Microsystems unter Ubuntu 7.04 zum Betrieb mit Moneyplex 2007. Sie sollte jedoch prinzipiell auch mit anderen Linuxdistributionen und -versionen funktionieren.
Sie basiert auf folgendem Beitrag: http://forum.ubuntuusers.de/topic/102322/?highlight=pcsclite

1. Installation von pcsclite

> sudo apt-get install libpcsclite1 libpcsclite-dev pcscd

2. Download des pcsc-ctapi-wrapper von www.matrica.de
(Danke an Patrick Schlangen)

> wget http://www.matrica.de/download/pcsc-ctapi-wrapper-0.1.tar.gz

3. Installation des Wrapper

> cd <Downloadverzeichnis>
> mkdir pcsc-ctapi-wrapper
> tar xfz pcsc-ctapi-wrapper-0.1.tar.gz -C pcsc-ctapi-wrapper/
> cd pcsc-ctapi-wrapper

Das Makefile muss so angepasst werden, dass der gcc die pcsc Bibliothek findet.
(Danke an Dschaeggi von forum.ubuntuusers.de)

Öffne das Makefile mit einem Editor deiner Wahl:
> editor Makefile

Ersetze den Code im Makefile durch:

-------------------8< cut here -------------------
# pcsc-ctapi-wrapper makefile
#
# Angepasst von Dschaeggi
# Uebernehme keinerlei Verantwortung

CC = gcc
CFLAGS = -O2 -Wall -I /usr/include/PCSC/

pcsc-ctapi-wrapper: pcsc-ctapi-wrapper.o
  $(CC) $(CFLAGS) -shared -o pcsc-ctapi-wrapper.so pcsc-ctapi-wrapper.o -lpcsclite

pcsc-ctapi-wrapper.o: pcsc-ctapi-wrapper.c
  $(CC) $(CFLAGS) -c -fPIC pcsc-ctapi-wrapper.c

clean:
  rm pcsc-ctapi-wrapper.so pcsc-ctapi-wrapper.o

install: pcsc-ctapi-wrapper
  cp pcsc-ctapi-wrapper.so /usr/local/lib
   
uninstall: pcsc-ctapi-wrapper
  rm /usr/local/lib/pcsc-ctapi-wrapper.so

-------------------8< cut here -------------------

Uebersetze den Wrapper:

> make
> sudo make install

4. Einrichtung von moneyplex

Einstellung -> Karteikarte Chipkartenleser
Schnittstelle: COM1/USB/Tastatur
CTAPI-Treiber: /usr/local/lib/pcsc-ctapi-wrapper.so

Fertig!

Viel Spaß!


--------------------------------------------------------------------
Hinweis dazu von unserem freundlichen Anwender Herr M. Braun
--------------------------------------------------------------------

Beim Einsatz des Chipdrive gab es ein Problem, wenn man mehrere Karten benutzt,
d.h. wenn diese ein und ausgesteckt wurden. Herr M. Braun schreibt dazu:

Ich nutze pcsc-ctapi-wrapper (http://www.matrica.de/download/chipdrive_usb_ubuntu.txt)
um mit moneyplex auf mit pcsc verwaltete Kartenleser zuzugreifen.
Das Problem lag tatsächlich in diesem Treiber.

Ich habe den Fehler behoben und einen Patch auf
http://sourceforge.net/tracker/index.php?func=detail&aid=1901686&group_id=155271&atid=795139
bereit gestellt. Vielleicht können Sie in
http://www.matrica.de/download/chipdrive_usb_ubuntu.txt
einen Hinweis darauf aufnehmen.

Besten Dank,
M. Braun
