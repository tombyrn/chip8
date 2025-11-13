all: build 

build:
	gcc -O3 -Wall -g -std=c99 emulator.c instructions.c -lSDL3 -o chip8 -lSDL3_image

run:
	./chip8 ./roms/tetris.ch8

test:
	./chip8 ./roms/test_opcode.ch8

clean:
	rm -rf chip8 chip8.dSYM
