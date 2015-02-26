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
