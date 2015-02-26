CC = gcc
DESTDIR?=
CFLAGS = -O2 -Wall -I /usr/include/PCSC/ -fPIC -D_REENTRANT -D USE_PORT_BASE1

# chose from
#   USE_PORT_BASE1 (Matrica: Moneyplex)
#     Use Portnumber to select smartcard reader, lowest input is 1
#     Moneyplex: select COMX to select the Xth reader
#   USE_PORT_BASE0 (any)
#     Use Portnumber to select smartcard reader, lowest input is 0
#   USE_CTN_BASE1 (any)
#     Use CTN to select smartcard reader, lowest input is 1
#   USE_CTN_BASE0 (jameica / libjavacard)
#     Use CTN to select smartcard reader, lowest input is 0
#     jameica: enter X into "Index des Lesers"/"Index of Reader"
#              to select Xth smartcard reader
# else:
#   use the last reader returned by pcsc

pcsc-ctapi-wrapper: pcsc-ctapi-wrapper.o
	$(CC) $(CFLAGS) -shared -o libpcsc-ctapi-wrapper.so.0.4 -Wl,-soname="libpcsc-ctapi-wrapper.so.0",-z,defs pcsc-ctapi-wrapper.o -lpcsclite
	strip --strip-unneeded pcsc-ctapi-wrapper.o

pcsc-ctapi-wrapper.o: pcsc-ctapi-wrapper.c
	$(CC) $(CFLAGS) -c -fPIC pcsc-ctapi-wrapper.c

clean:
	rm -f libpcsc-ctapi-wrapper.so.0.4 pcsc-ctapi-wrapper.o

install: pcsc-ctapi-wrapper
	install -d $(DESTDIR)/usr/lib
	install -m644 libpcsc-ctapi-wrapper.so.0.4 $(DESTDIR)/usr/lib
	ldconfig -l $(DESTDIR)/usr/lib/libpcsc-ctapi-wrapper.so.0.4

uninstall: pcsc-ctapi-wrapper
	rm $(DESTDIR)/usr/lib/libpcsc-ctapi-wrapper.so
	rm $(DESTDIR)/usr/lib/libpcsc-ctapi-wrapper.so.0.4
	ldconfig
