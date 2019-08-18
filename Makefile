CC = gcc
DESTDIR = /usr/lib
CFLAGS = -O2 -std=gnu90 -Wall -I /usr/include/PCSC/ -fPIC -D_REENTRANT -D USE_CTN_BASE0

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
	$(CC) $(CFLAGS) -shared -o pcsc-ctapi-wrapper.so -Wl,-soname="pcsc-ctapi-wrapper.so",-z,defs pcsc-ctapi-wrapper.o -lpcsclite
	strip --strip-unneeded pcsc-ctapi-wrapper.o

pcsc-ctapi-wrapper.o: pcsc-ctapi-wrapper.c
	$(CC) $(CFLAGS) -c -fPIC pcsc-ctapi-wrapper.c

clean:
	rm -f pcsc-ctapi-wrapper.so pcsc-ctapi-wrapper.o

install: pcsc-ctapi-wrapper
	install -d $(DESTDIR)
	install -m644 pcsc-ctapi-wrapper.so $(DESTDIR)
	ldconfig -l $(DESTDIR)/pcsc-ctapi-wrapper.so

uninstall: pcsc-ctapi-wrapper
	rm $(DESTDIR)/libpcsc-ctapi-wrapper.so
	ldconfig
