CC=c99
PKG_CONFIG=pkg-config
INSTALL=install

CFLAGS=-O3 -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE
LDFLAGS=

CFLAGS+=`$(PKG_CONFIG) --silence-errors --cflags libelf || echo -lelf`
LDFLAGS+=`$(PKG_CONFIG) --silence-errors --libs libelf || true`

PREFIX?=/usr/local

MANDIR?=share/man
BINMODE?=0755
MANMODE?=644

RM=rm -rf

.DEFAULT: all
.PHONY: clean all install

all: elfcat
clean:
	$(RM) elfcat

elfcat: elfcat.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o elfcat elfcat.c

install: elfcat elfcat.1
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -m $(BINMODE) elfcat $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/$(MANDIR)/man1
	$(INSTALL) -m $(MANMODE) elfcat.1 $(DESTDIR)$(PREFIX)/$(MANDIR)/man1
