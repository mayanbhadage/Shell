CC = cc
OBJECTS = minish.o

all: minish

minish.o: minish.c
	$(CC) -c minish.c

minish: $(OBJECTS)
	$(CC) $(OBJECTS) -o minish

clean:
	rm -f *.o minish
