CFLAGS=-Wall -Werror -I.
LDFLAGS=-L /usr/local/lib -lmosquitto -lconfig

### Linux
# CFLAGS += `pkg-config --cflags lua5.3`
# LDFLAGS += `pkg-config --libs lua5.3`
# LDFLAGS += `pkg-config --libs libbsd`
# BINDIR = /usr/local/bin
# MANDIR  = /usr/local/share/man

### macOS
CFLAGS += `pkg-config --cflags lua5.3`
LDFLAGS += `pkg-config --libs lua5.3`
BINDIR = /usr/local/bin
MANDIR  = /usr/local/share/man

### FreeBSD
# CFLAGS += -I/usr/local/include -I/usr/local/include/lua53/
# LDFLAGS += -llua-5.3
# BINDIR = /usr/local/bin
# MANDIR  = /usr/local/share/man

### OpenBSD
# CFLAGS += -I/usr/local/include -I/usr/local/include/lua-5.3/
# LDFLAGS += -llua5.3
# BINDIR = /usr/local/bin
# MANDIR  = /usr/local/man


OBJS = conn.o json.o print.o interp.o

all: msoak

msoak: msoak.c ud.h interp.h $(OBJS)
	$(CC) $(CFLAGS) -o msoak msoak.c $(OBJS) $(LDFLAGS)

conn.o: conn.c conn.h
print.o: print.c print.h ud.h utstring.h interp.h
json.o: json.c json.h
interp.o: interp.c interp.h version.h

install: msoak msoak.1
	install -m755 msoak $(DESTDIR)$(BINDIR)/msoak
	install -m644 msoak.1 $(DESTDIR)$(MANDIR)/man1/msoak.1

pdf:
	groff -Tps -man msoak.1 > msoak.tmp_ && pstopdf -i msoak.tmp_ -o msoak.pdf && rm -f msoak.tmp_

clean:
	rm -f *.o
clobber: clean
	rm -f msoak
