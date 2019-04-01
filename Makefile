CFLAGS=-Wall -Werror -I. -I/usr/local/include/lua5.3/
LDFLAGS=-L /usr/local/lib -lmosquitto -lconfig -llua

OBJS = conn.o json.o print.o interp.o

all: msoak

msoak: msoak.c ud.h interp.h $(OBJS)
	$(CC) $(CFLAGS) -o msoak msoak.c $(OBJS) $(LDFLAGS)

conn.o: conn.c conn.h
print.o: print.c print.h ud.h utstring.h interp.h
json.o: json.c json.h
interp.o: interp.c interp.h version.h

clean:
	rm -f *.o
clobber: clean
	rm -f msoak
