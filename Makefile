CC=c99
CFLAGS=-O3 -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE `pkg-config --cflags libelf`
LDFLAGS=`pkg-config --libs libelf`

PREFIX?=/usr/local

MANDIR?=share/man
BINMODE?=0755
MANMODE?=644

RM=rm -rf

.DEFAULT: all
.PHONY: clean all

all: elfcat
clean:
	$(RM) elfcat

elfcat: elfcat.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o elfcat elfcat.c

install: elfcat elfcat.1
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m $(BINMODE) elfcat $(DESTDIR)$(PREFIX)/bin
	install -d $(DESTDIR)$(PREFIX)/$(MANDIR)/man1
	install -m $(MANMODE) elfcat.1 $(DESTDIR)$(PREFIX)/$(MANDIR)/man1
