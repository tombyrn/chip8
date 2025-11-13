#ifndef INCLUDES_H
#define INCLUDES_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include <SDL3/SDL.h>

#define DEBUG

#define START_ADDRESS 0x200
#define FONTSET_SIZE 80
#define FONTSET_START_ADDRESS 0x50

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 320

#define FPS 60
#define CYCLES_PER_FRAME 10
#define MS_PER_FRAME (1000/FPS)

#define CHECK_ARGC if(argc != 2) {\
        fprintf(stderr, "Usage: ./chip8 [rom_path]\n");\
        return EXIT_FAILURE;\
    }
#define CHECK_TIME int time_to_wait = MS_PER_FRAME - (SDL_GetTicks() - last_frame_time);\
	    if( time_to_wait > 0 && time_to_wait <= MS_PER_FRAME)\
		    SDL_Delay(time_to_wait);\

extern uint8_t fontset[FONTSET_SIZE];

uint8_t registers[16];
uint8_t memory[4096];

uint16_t pc;
uint16_t stack[16];
uint16_t index_register;
uint8_t sp;
uint8_t delay_register;
uint8_t sound_register;
uint8_t keyboard[16];
uint32_t video[64 * 32];
uint16_t opcode;
bool quit;

uint_fast8_t drawflag;
void op_00E0();
void op_00EE();
void op_1nnn();
void op_2nnn();
void op_3xkk();
void op_4xkk();
void op_5xy0();
void op_6xkk();
void op_7xkk();
void op_8xy0();
void op_8xy1();
void op_8xy2();
void op_8xy3();
void op_8xy4();
void op_8xy5();
void op_8xy6();
void op_8xy7();
void op_8xyE();

void op_9xy0();
void op_Annn();
void op_Bnnn();
void op_Cxkk();
void op_Dxyn();

void op_Ex9E();
void op_ExA1();
void op_Fx07();
void op_Fx0A();
void op_Fx15();
void op_Fx18();
void op_Fx1E();
void op_Fx29();
void op_Fx33();
void op_Fx55();
void op_Fx65();


#endif