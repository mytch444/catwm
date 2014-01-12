CFLAGS+= -Wall
LDADD+= -lX11 -lXinerama
LDFLAGS=
EXEC=catwm

PREFIX?= /usr
BINDIR?= $(PREFIX)/bin

CC=gcc

all: $(EXEC)

#catwm: catwm.o
catwm: catwm.o
	$(CC) $(LDFLAGS) -Os -o $@ $+ $(LDADD)

install: all
	install -Dm 755 catwm $(DESTDIR)$(BINDIR)/catwm

clean:
	rm -f catwm *.o
