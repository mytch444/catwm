CFLAGS+= -Wall
LDADD+= -lX11 -lXinerama
EXEC=catwm

BINDIR?= /usr/bin

CC=gcc

all: $(EXEC)

$(EXEC): catwm.o
	$(CC) $(LDADD) catwm.o -o catwm 

catwm.o: catwm.c config.h
	$(CC) -c catwm.c

install: all
	install -Dm 755 $(EXEC) $(BINDIR)/$(EXEC)

clean:
	rm -f $(EXEC) *.o
