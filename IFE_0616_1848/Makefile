CC = gcc
CFLAGS = -I./src

SRC = src/main_new.c src/toml.c src/cJSON.c src/cJSON_Utils.c
LIBS = -lSDL2 -lSDL2_image -lSDL2_ttf -lcurl

all:
	$(CC) -o engine $(SRC) $(CFLAGS) $(LIBS)