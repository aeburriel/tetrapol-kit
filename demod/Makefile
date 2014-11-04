CC=gcc
PROJECTS=tetrapol
CFLAGS=-ggdb3 -std=gnu11

LIBS=radio.o multiblock.o tsdu.o tpdu.o misc.o

all: clean $(PROJECTS)

tetrapol: $(LIBS) tetrapol.o
	$(CC) $(CFLAGS) tetrapol.o $(LIBS) -o tetrapol

xor: xor.o
	$(CC) $(CFLAGS) xor.o -o xor

sync: sync.o
	$(CC) $(CFLAGS) sync.o -o sync

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

run: $(PROJECT)
	./$(PROJECT)

clean:
	rm -f $(PROJECT) *.o
