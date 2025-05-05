CC=gcc
CFLAGS=-Isrc/include -Lsrc/lib
LIBS=-lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer

all:
	$(CC) $(CFLAGS) -o pong pong.c $(LIBS)
