SHELL = /bin/sh

PREFIX ?= /usr/local
LIBDIR = $(PREFIX)/lib

EXTFLAGS = -fPIC -shared
EXTLIBS = `pkg-config --cflags webkit2gtk-4.0 webkit2gtk-web-extension-4.0 glib-2.0 gio-2.0`

all: blockit.so

blockit.so: blockit.c
	$(CC) blockit.c -o blockit.so $(EXTFLAGS) $(EXTLIBS) $(CFLAGS)

install: all
	mkdir -p $(DESTDIR)$(LIBDIR)
	cp -f blockit.so $(DESTDIR)$(LIBDIR)
	chmod 755 $(DESTDIR)$(LIBDIR)/blockit.so

uninstall:
	rm -f $(DESTDIR)$(LIBDIR)/blockit.so

clean:
	rm -f blockit.so

.PHONY: install uninstall clean
