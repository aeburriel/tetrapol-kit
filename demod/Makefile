CC=gcc
PROJECT=tetrapol
CFLAGS=-ggdb3 -std=gnu11 -Og -Wall

SRC=misc.c multiblock.c radio.c tetrapol.c tpdu.c tsdu.c
OBJS=$(SRC:%.c=%.o)

all: clean $(PROJECT)

tetrapol: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

run: $(PROJECT)
	./$(PROJECT)

clean:
	rm -f $(PROJECT) *.o
