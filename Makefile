CC=g++
CFLAGS=-O3 -Wall -Wextra
SDL2FLAGS=-lmingw32 -lSDL2main -lSDL2 -I"..\..\SDL2\x86_64-w64-mingw32\include" -L"..\..\SDL2\x86_64-w64-mingw32\lib"
STATIC_FLAGS=-static-libgcc -static-libstdc++
DEBUG_FLAGS=-g -O0 -Wall -Wextra

release:
	$(CC) main.cpp -o antigravic $(CFLAGS) $(SDL2FLAGS) $(STATIC_FLAGS)

all:
	$(CC) main.cpp -o antigravic $(SDL2FLAGS) $(DEBUG_FLAGS)