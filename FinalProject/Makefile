CC = gcc
CFLAGS = -I./src

SRC = src/main.c src/toml.c src/cJSON.c src/cJSON_Utils.c
LIBS = -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lcurl

all:
	$(CC) -o engine $(SRC) $(CFLAGS) $(LIBS)