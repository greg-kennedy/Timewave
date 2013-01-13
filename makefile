CC = gcc
PROF = -O2 -fomit-frame-pointer -frename-registers -pipe
DEFS =
C_FLAGS = -Wall $(PROF) $(DEFS)
L_FLAGS = $(PROF)
LIBS = -lSDL -lSDL_mixer -lSDL_image

all:	timewave

timewave: game.c main.c menu.c title.c hs.c winlose.c hspick.c keyconf.c
	$(CC) -c main.c $(C_FLAGS)
	$(CC) -c game.c $(C_FLAGS)
	$(CC) -c menu.c $(C_FLAGS)
	$(CC) -c title.c $(C_FLAGS)
	$(CC) -c winlose.c $(C_FLAGS)
	$(CC) -c hs.c $(C_FLAGS)
	$(CC) -c hspick.c $(C_FLAGS)
	$(CC) -c keyconf.c $(C_FLAGS)
	$(CC) -o timewave main.o winlose.o game.o menu.o title.o hs.o hspick.o keyconf.o $(L_FLAGS) $(LIBS)
	strip timewave

clean:
	rm -f *.o timewave config.ini
