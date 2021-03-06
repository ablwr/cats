SHELL=/bin/sh

CC=cc
CFLAGS+=-O2 -std=c89 -Wpedantic -Wall -Wextra -Wunused -Wshadow -Wdouble-promotion -Wstrict-overflow=5

INSTALL=install -D
INSTALL_DATA=$(INSTALL) -m 644

BINDIR=$(DESTDIR)/usr/bin
MANDIR=$(DESTDIR)/usr/share/man/man6

STRIP=strip
ifeq ($(OS),Windows_NT)
 X = .exe
else
  ifeq ($(shell uname), SunOS)
    STRIP=gstrip
  endif
endif

PROG=cats$X
MANPAGE=cats.6.gz

$(PROG): *.c
	$(CC) -o $@ $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $^
	-$(STRIP) -s $@

$(MANPAGE): cats.6
	gzip -9 -n -c cats.6 > cats.6.gz

install: $(PROG) $(MANPAGE)
	$(INSTALL) $(PROG) $(BINDIR)/$(PROG)
	$(INSTALL_DATA) $(MANPAGE) $(MANDIR)/$(MANPAGE)

uninstall:
	rm -f $(BINDIR)/$(PROG)
	rm -f $(MANDIR)/$(MANPAGE)

fmt: *.c
	VERSION_CONTROL=never indent -kr -i4 -ppi4 -nut -l100 -cp0 -ncs -sob \
		-T HANDLE \
		$^

.PHONY: clean install uninstall
clean:
	rm -f $(PROG)
	rm -f $(MANPAGE)
