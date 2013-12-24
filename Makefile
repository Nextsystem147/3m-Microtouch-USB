VERSION=0.9
EXTRAVERSION?=
APPNAME=opengalax2
AUTHOR=(c)2013 Oskari Rauta <oskari.rauta@gmail.com>, (c)2012 Pau Oliva Fora <pof@eslack.org>
DESTDIR?=/
SHELL=/bin/sh
CC?=gcc
CFLAGS?=-Wall
APP_CFLAGS=-Wall -Wextra -Wwrite-strings -O -g -DAPPNAME="\"$(APPNAME)\"" -DVERSION="\"$(VERSION)\"" -DEXTRAVERSION="\"$(EXTRAVERSION)\"" -DAUTHOR="\"$(AUTHOR)\""
LDFLAGS?=
APP_LDFLAGS=-lts
INSTALL?=/usr/bin/install -c
INSTALLDATA?=/usr/bin/install -c -m 644

srcdir = .
prefix = $(DESTDIR)
bindir = $(prefix)/usr/bin
docdir = $(prefix)/usr/share/doc
mandir = $(prefix)/usr/share/man

OBJ=functions.o configfile.o opengalax2.o
BIN=opengalax2

all: ${OBJ}
	$(CC) $(CFLAGS) $(APP_CFLAGS) ${OBJ} $(APP_LDFLAGS) $(LDFLAGS) -o ${BIN}

functions.o: functions.c
	$(CC) $(CFLAGS) $(APP_CFLAGS) -c -o functions.o functions.c

configfile.o: configfile.c
	$(CC) $(CFLAGS) $(APP_CFLAGS) -c -o configfile.o configfile.c

opengalax2.o: opengalax2.c
	$(CC) $(CFLAGS) $(APP_CFLAGS) -c -o opengalax2.o opengalax2.c

install: all
	mkdir -p $(bindir)
	$(INSTALL) $(BIN) $(bindir)/$(BIN)
	mkdir -p $(docdir)/$(BIN)/
	$(INSTALLDATA) $(srcdir)/README.md $(docdir)/$(BIN)/
	$(INSTALLDATA) $(srcdir)/LICENSE $(docdir)/$(BIN)/
	mkdir -p $(prefix)/etc/pm/sleep.d/
	$(INSTALL) $(srcdir)/75_opengalax2 $(prefix)/etc/pm/sleep.d/
	mkdir -p $(prefix)/etc/X11/xorg.conf.d/
	$(INSTALLDATA) $(srcdir)/10-opengalax2.conf $(prefix)/etc/X11/xorg.conf.d/

uninstall:
	rm -rf $(bindir)/$(BIN)
	rm -rf $(docdir)/$(BIN)/
	rm -rf $(prefix)/etc/pm/sleep.d/75_opengalax2
	rm -rf $(prefix)/etc/X11/xorg.conf.d/10-opengalax2.conf

clean:
	rm -f $(BIN) *.o
