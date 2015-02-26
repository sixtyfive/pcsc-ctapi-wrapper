CC = gcc
CFLAGS = -O2 -Wall -I /usr/include/PCSC/

pcsc-ctapi-wrapper: pcsc-ctapi-wrapper.o
	$(CC) $(CFLAGS) -shared -o libpcsc-ctapi-wrapper.so.0.3 -Wl,-soname="libpcsc-ctapi-wrapper.so" pcsc-ctapi-wrapper.o -lpcsclite

pcsc-ctapi-wrapper.o: pcsc-ctapi-wrapper.c
	$(CC) $(CFLAGS) -c -fPIC pcsc-ctapi-wrapper.c

clean:
	rm libpcsc-ctapi-wrapper.so.0.3 pcsc-ctapi-wrapper.o

install: pcsc-ctapi-wrapper
	cp libpcsc-ctapi-wrapper.so.0.3 /usr/local/lib
	ldconfig

uninstall: pcsc-ctapi-wrapper
	rm /usr/local/lib/libpcsc-ctapi-wrapper.so
	rm /usr/local/lib/libpcsc-ctapi-wrapper.so.0.3
	ldconfig
