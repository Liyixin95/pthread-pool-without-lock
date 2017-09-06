CC = gcc
CFLAGS = -Wall -g -lpthread

test:main.c pool.c
	$(CC) $^ -o $@ $(CFLAGS)

debug:main.c pool.c
	$(CC) $^ -o $@ $(CFLAGS) -D DEBUG

clean:
	$(RM) .*.sw? test debug *.o

.PHONY:all clean
