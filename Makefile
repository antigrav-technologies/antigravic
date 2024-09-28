CC=g++
CFLAGS=-O3 -Wall -Wextra
SDL2FLAGS=-lmingw32 -lSDL2main -lSDL2 -I"..\..\SDL2\x86_64-w64-mingw32\include" -L"..\..\SDL2\x86_64-w64-mingw32\lib"
STATIC_FLAGS=-static-libgcc -static-libstdc++

all:
	$(CC) main.cpp -o antigravic $(CFLAGS) $(SDL2FLAGS)