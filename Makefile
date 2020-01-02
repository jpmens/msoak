CFLAGS=-Wall -Werror -I. `pkg-config --cflags lua5.3`
LDFLAGS=-L /usr/local/lib -lmosquitto -lconfig `pkg-config --libs lua5.3`

BINDIR = /usr/local/bin
MANDIR  = /usr/local/share/man

OBJS = conn.o json.o print.o interp.o

all: msoak

msoak: msoak.c ud.h interp.h $(OBJS)
	$(CC) $(CFLAGS) -o msoak msoak.c $(OBJS) $(LDFLAGS)

conn.o: conn.c conn.h
print.o: print.c print.h ud.h utstring.h interp.h
json.o: json.c json.h
interp.o: interp.c interp.h version.h

install: msoak msoak.1
	install -m755 msoak $(BINDIR)/msoak
	install -m644 msoak.1 $(MANDIR)/man1/msoak.1

clean:
	rm -f *.o
clobber: clean
	rm -f msoak
