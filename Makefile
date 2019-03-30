CFLAGS=-Wall -Werror -I. -g
LDFLAGS=-L /usr/local/lib -lmosquitto -lconfig

OBJS = conn.o json.o print.o

all: msoak

msoak: msoak.c ud.h $(OBJS)
	$(CC) $(CFLAGS) -o msoak msoak.c $(OBJS) $(LDFLAGS)

conn.o: conn.c conn.h
print.o: print.c print.h ud.h utstring.h
json.o: json.c json.h

clean:
	rm -f *.o
clobber: clean
	rm -f msoak
