CFLAGS=-Wall -Werror -I. -g
LDFLAGS=-L /usr/local/lib -lmosquitto -lconfig

OBJS = conn.o json.o

all: msoak

msoak: msoak.c $(OBJS)
	$(CC) $(CFLAGS) -o msoak msoak.c $(OBJS) $(LDFLAGS)

conn.o: conn.c conn.h
json.o: json.c json.h

clean:
	rm -f *.o
clobber: clean
	rm -f msoak
