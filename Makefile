all: blockit.so

blockit.so: blockit.c
	$(CC) blockit.c -Wall -fPIC -shared `pkg-config --cflags webkit2gtk-4.0` -o $@

install: all
	mkdir -p /usr/lib/blockit
	cp -f blockit.so /usr/lib/blockit

uninstall:
	rm -f /usr/lib/blockit/blockit.so

clean:
	rm -f blockit.so

.PHONY: install uninstall clean
