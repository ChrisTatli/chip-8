#Compiler
CC=clang

#Compiler options
CFLAGS=-c -std=c99 -Wall -Wextra -02

#Compiler and linker options for SDL2
SDL_CFLAGS= $(shell sdl2-config --cflags)
SDL_LFLAGS= $(shell sdl2-config --libs)

override CFLAGS += $(SDL_CFLAGS)

HEADERDIR= src/
SOURCEDIR= src/

HEADERFILES=
SOURCEFILES= main.c 

HEADERS_FP= $(addprefix $(HEADERDIR),$(HEADERFILES))
SOURCE_FP= $(addprefix $(SOURCEDIR),$(SOURCEFILES))

OBJ = $(SOURCE_FP:.c = .o)

EXE = chip8

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(OBJ) $(SDL_LFLAGS) -o $(EXE)

%.o: %.c $(HEADERS_FP)
	$(CC) $(CFLAGS) -o $@ $< 

clean:
	rm -rf src/*.o $(EXE)
