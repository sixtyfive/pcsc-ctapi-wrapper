pcsc-ctapi-wrapper
------------------

Allows using chipcard readers using a PCSC interface with software supporting CTAPI readers only. The wrapper is basically a CTAPI module which translates the CTAPI functions into PCSC functions, so a working chipcard reader with a PCSC interface is needed. The wrapper does *not* support entering the PIN into the chipcard reader, only entering the PIN through the computer's keyboard is supported.

Working chipcard readers include, but are possibly not limited to:

* Cyberjack e-com/Pinpad
* Cyberjack RFID/Standard
* Cyberjack RFID/Komfort
* SmartBoard G83-6744
* ST-1044U
* ST-1044U
* Chipdrive Micro / Towitoko Kartenzwerg
* Chipdrive Pinpad / SCM SPR 532
* Chipdrive Pinpad pro / SCM SPR 532
* SCR 335 (Chipdrive micro pro)
* Omnikey CardMan Mobile 4040 PCMCIA

This list is taken from http://www.willuhn.de/wiki/doku.php?id=support:list:kartenleser#scm_chipdrive. Please note that it has been documented there for certain readers to work with certain versions of this wrapper library (all known ones are in the GitHub repository at https://github.com/sixtyfive/pcsc-ctapi-wrapper). Further work on this library might be necessary in order to support everything "out of the box" and without hassle.

Software that should be able to work with pcsc-ctapi-wrapper:

* Moneyplex
* StarMoney on Linux
* GnuCash, KMyMoney, etc. through libapbanking
* Jameica/Hibiscus (the library is configured by default to work with Jameica/Hibiscus)

Requirements for compilation are currently:

* gcc
* pcsc-lite (binaries as well as headers and libraries)

Under Debian 9 (Stretch) the dependencies can be installed as follows:

* apt install build-essential libudev-dev flex
* git clone git://anonscm.debian.org/pcsclite/PCSC.git
  * cd PCSC
  * ./bootstrap
  * ./configure --prefix=/usr/local && make && make install

For installation please take a look at the Makefile and adjust according to your needs. Then simply:

1. make
2. sudo make install

Further information on how to use the wrapper should be available on the websites of the software you are trying to use it with. This README file used to include notes on Moneyplex and StarMoney in specific. These can still be found in older revisions, e.g. through GitHub.
