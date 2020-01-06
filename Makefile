# detect OS type and use that as default first "detect" target
OS.exec = ./ostype
OS = $(shell $(OS.exec))$(OS.exec:sh)

CFLAGS=-Wall -Werror -I.
LDFLAGS=-L /usr/local/lib -lmosquitto -lconfig

detect:
	@echo "detected: $(OS)"
	@$(MAKE) TARGET=all $(OS)


debian:
	@$(MAKE) $(TARGET) CFLAGS="$(CFLAGS) `pkg-config --cflags lua5.3`" \
		LDFLAGS="$(LDFLAGS) `pkg-config --libs lua5.3` `pkg-config --libs libbsd`" \
		BINDIR=/usr/local/bin \
		MANDIR=/usr/local/share/man

ubuntu:
	@$(MAKE) TARGET=$(TARGET) debian


alpine:
	@$(MAKE) $(TARGET) CFLAGS="-I/usr/include/lua5.3 -Wall -I." \
		LDFLAGS="$(LDFLAGS) -L/usr/lib/lua5.3 -llua -lbsd" \
		BINDIR=/usr/local/bin \
		MANDIR=/usr/local/share/man


darwin:
	@$(MAKE) $(TARGET) CFLAGS="$(CFLAGS) `pkg-config --cflags lua5.3`" \
		LDFLAGS="$(LDFLAGS) `pkg-config --libs lua5.3`" \
		BINDIR=/usr/local/bin \
		MANDIR=/usr/local/share/man

freebsd:
	@$(MAKE) $(TARGET) CFLAGS="$(CFLAGS) -I/usr/local/include -I/usr/local/include/lua53/" \
		LDFLAGS="$(LDFLAGS) -llua-5.3" \
		BINDIR=/usr/local/bin \
		MANDIR=/usr/local/share/man

openbsd:
	@$(MAKE) $(TARGET) CFLAGS="$(CFLAGS) -I/usr/local/include -I/usr/local/include/lua-5.3/" \
		LDFLAGS="$(LDFLAGS) -llua5.3" \
		BINDIR=/usr/local/bin \
		MANDIR=/usr/local/man

OBJS = conn.o json.o print.o interp.o

all: msoak

msoak: msoak.c ud.h interp.h $(OBJS) version.h
	$(CC) $(CFLAGS) -o msoak msoak.c $(OBJS) $(LDFLAGS)

conn.o: conn.c conn.h
print.o: print.c print.h ud.h utstring.h interp.h
json.o: json.c json.h
interp.o: interp.c interp.h version.h

install: msoak msoak.1
	@$(MAKE) TARGET=realinstall $(OS)

realinstall: msoak msoak.1
	install -m755 msoak $(DESTDIR)$(BINDIR)/msoak
	install -m644 msoak.1 $(DESTDIR)$(MANDIR)/man1/msoak.1

msoak.pdf: msoak.1
	groff -Tps -man msoak.1 > msoak.tmp_ && pstopdf -i msoak.tmp_ -o msoak.pdf && rm -f msoak.tmp_

clean:
	rm -f *.o
clobber: clean
	rm -f msoak
