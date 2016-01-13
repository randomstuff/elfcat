CC=c99
CFLAGS=-O3 -D_FILE_OFFSET_BITS=64

LD=ld
LDFLAGS=

RM=rm -rf

.DEFAULT: all
.PHONY: clean all

all: elfcat
clean:
	$(RM) elfcat.o elfcat

elfcat.o: elfcat.c
	$(CC) -c $(CFLAGS) -o elfcat.o elfcat.c
elfcat: elfcat.o
	$(CC) $(CFLAGS) -lelf -o elfcat elfcat.o
