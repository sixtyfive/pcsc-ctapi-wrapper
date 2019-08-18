pcsc-ctapi-wrapper
==================

Allows using chipcard readers using a PCSC interface with software supporting CTAPI readers only. The wrapper is basically a CTAPI module which translates the CTAPI functions into PCSC functions, so a working chipcard reader with a PCSC interface is needed. The wrapper does *not* support entering the PIN into the chipcard reader, only entering the PIN through the computer's keyboard is supported.

Known-working chipcard readers
------------------------------

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

This list is taken from [willuhn.de](http://www.willuhn.de/wiki/doku.php?id=support:list:kartenleser#scm_chipdrive). Please note that it has been documented there for certain readers to work with certain versions of this wrapper library (all known ones are in the GitHub repository at [sixtyfive/pcsc-ctapi-wrapper](https://github.com/sixtyfive/pcsc-ctapi-wrapper)). Further work on this library might be necessary in order to support everything "out of the box" and without hassle.

Software that should be able to work with pcsc-ctapi-wrapper
------------------------------------------------------------

* Moneyplex
* StarMoney on Linux
* GnuCash, KMyMoney, etc. through libapbanking
* Jameica/Hibiscus (the library is configured by default to work with Jameica/Hibiscus)

Dependencies
------------

* `make`
* `gcc`
* `pcsc-lite` (binaries as well as headers and libraries)

### Debian-based distributions

```
apt install build-essential libudev-dev flex ccid
```

### Solus

```
eopkg it -c system.devel
eopkg it ccid pcsc-tools pcsc-lite pcsc-lite-devel
```

Building
--------

Getting PCSC sources:

```
git submodule init && git submodule update
```

Installing PCSC:

```
cd PCSC
./bootstrap
(cd src/spy; gcc -fPIC -I.. -I../PCSC/ -c libpcscspy.c; mv libpcscspy.o .libs)
./configure --prefix=/usr && make && sudo make install
```

Now take a look at the top-level Makefile and adjust according to your needs. Then simply:

```
make
sudo make install
```

Usage
-----

Further information on how to use the wrapper should be available on the websites of the software you are trying to use it with. This README file used to include notes on Moneyplex and StarMoney in specific. These can still be found in older revisions, e.g. through GitHub.
