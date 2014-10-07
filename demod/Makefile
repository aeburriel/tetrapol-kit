CC=gcc
PROJECTS=tetrapol tetrapol2 forward
CFLAGS=-g

LIBS=radio.o

all: clean $(PROJECTS)

tetrapol: $(LIBS) tetrapol.o
	$(CC) $(CFLAGS) tetrapol.o $(LIBS) -o tetrapol

tetrapol2: $(LIBS) tetrapol2.o
	$(CC) $(CFLAGS) tetrapol2.o $(LIBS) -o tetrapol2

forward: $(LIBS) forward.o
	$(CC) $(CFLAGS) forward.o $(LIBS) -o forward
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
