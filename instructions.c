#include "includes.h"

void op_00E0() {
    memset(video, 0, 64 * 32 * sizeof(uint32_t));
	drawflag = true;
}

void op_00EE() {
	if (sp == 0) {
        fprintf(stderr, "Stack underflow at RET\n");
        exit(1);
    }
	
    sp--;
    pc = stack[sp];
	printf("Returning to %03X\n", pc);
}

void op_1nnn() {
	uint16_t address = opcode & 0x0FFFu;
	#ifdef DEBUG
    printf("[1NNN] Jump from PC(before)=%03x to 0x%03x\n", pc-2, address);
    #endif
	pc = address;
}

void op_2nnn() {
	uint16_t address = opcode & 0x0FFFu;
	
	if (sp >= 16) {
		fprintf(stderr, "Stack overflow at CALL\n");
		exit(1);
	}
	stack[sp] = pc;  // decrement pc since it was already incremented in cycle()
	sp++;
	pc = address;
}

void op_3xkk() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if (registers[Vx] == byte)
		pc += 2;

}

void op_4xkk() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if (registers[Vx] != byte)
		pc += 2;

}

void op_5xy0() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] == registers[Vy])
		pc += 2;
}

void op_6xkk() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	registers[Vx] = byte;
}

void op_7xkk() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	#ifdef DEBUG
    printf("[7XKK] before: V[%u]=%02x\n", Vx, registers[Vx]);
    #endif
	
	registers[Vx] += byte;

    #ifdef DEBUG
    printf("[7XKK] after: V[%u]=%02x\n", Vx, registers[Vx]);
    #endif
}

void op_8xy0() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] = registers[Vy];
}

void op_8xy1() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] |= registers[Vy];
}

void op_8xy2() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] &= registers[Vy];
}

void op_8xy3() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] ^= registers[Vy];
}

void op_8xy4() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	uint16_t sum = registers[Vx] + registers[Vy];

	if (sum > 255U)
		registers[0xF] = 1;

	else
		registers[0xF] = 0;


	registers[Vx] = sum & 0xFFu;
}

void op_8xy5(){
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] > registers[Vy])
		registers[0xF] = 1;
	else
		registers[0xF] = 0;


	registers[Vx] -= registers[Vy];
}

void op_8xy6() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	// Save LSB in VF
	registers[0xF] = (registers[Vx] & 0x1u);

	registers[Vx] >>= 1;
}

void op_8xy7() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vy] > registers[Vx])
		registers[0xF] = 1;
	else
		registers[0xF] = 0;

	registers[Vx] = registers[Vy] - registers[Vx];
}

void op_8xyE() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	// Save MSB in VF
	registers[0xF] = (registers[Vx] & 0x80u) >> 7u;

	registers[Vx] <<= 1;
}

void op_9xy0() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] != registers[Vy])
		pc += 2;
	
}

void op_Annn(){
	uint16_t address = opcode & 0x0FFFu;

	index_register = address;
}

void op_Bnnn() {
	uint16_t address = opcode & 0x0FFFu;

	pc = registers[0] + address;
}

void op_Cxkk() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;
	uint8_t r = rand() % 256;

	registers[Vx] = r & byte;
}

void op_Dxyn() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	
	uint8_t height = opcode & 0x000Fu;
	
	// Wrap if going beyond screen boundaries
	uint8_t xPos = registers[Vx] % 64;
	uint8_t yPos = registers[Vy] % 32;

	#ifdef DEBUG
	printf("[Dxyn] Enter: Vx=%02x Vy=%02x(xpos=%u ypos=%u) height=%u I=%03x\n",
		registers[Vx], registers[Vy], xPos, yPos, height, index_register);
	#endif

	registers[0xF] = 0;
	for(unsigned int row = 0; row < height; ++row) {
		uint8_t spriteByte = memory[index_register + row];

		for(unsigned int col = 0; col < 8; ++col) {
			uint8_t spritePixel = spriteByte & (0x80u >> col);
			uint32_t* screenPixel = &video[(yPos + row) * 64 + (xPos + col)];

			// Sprite pixel is on
			if(spritePixel) {
				// Screen pixel also on - collision
				if (*screenPixel == 0xFFFFFFFF)
					registers[0xF] = 1;


				// Effectively XOR with the sprite pixel
				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}

	drawflag = true;
}

void op_Ex9E() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	uint8_t key = registers[Vx];
    #ifdef DEBUG
    printf("[EX9E] PC(before)=%03x  V[%d]=0x%02x  keyboard[Vx]=%d\n", pc, Vx, key, keyboard[key & 0xF]);
    #endif
	if(keyboard[key] != 0) {
		pc += 2;
		#ifdef DEBUG
        printf("[EX9E] key pressed -> skipping to PC=%03x\n", pc);
        #endif
	}
}

void op_ExA1() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	uint8_t key = registers[Vx];

	if(keyboard[key] == 0) {
		pc += 2;
		#ifdef DEBUG
		printf("[EXA1] key NOT pressed -> skipping to PC=%03x\n", pc);
		#endif
	}
}

void op_Fx07() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	registers[Vx] = delay_register;
}

void op_Fx0A() {
    uint8_t Vx = (opcode & 0x0F00u) >> 8;

    // Wait for any key press
    bool key_pressed = false;
    for(int i = 0; i < 16; i++) {
        if (keyboard[i]) {
            registers[Vx] = i;
            key_pressed = true;
            // break;
        }
    }

    if(!key_pressed) {
        // repeat this instruction until a key is pressed
        pc -= 2;
    }
}

void op_Fx15() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	delay_register = registers[Vx];
}

void op_Fx18() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	sound_register = registers[Vx];
}

void op_Fx1E() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	index_register += registers[Vx];
}

void op_Fx29() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t digit = registers[Vx];

	index_register = FONTSET_START_ADDRESS + (5 * digit);
}

void op_Fx33() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t value = registers[Vx];

	// Ones-place
	memory[index_register + 2] = value % 10;
	value /= 10;

	// Tens-place
	memory[index_register + 1] = value % 10;
	value /= 10;

	// Hundreds-place
	memory[index_register] = value % 10;
}


void op_Fx55() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; i++)
		memory[index_register + i] = registers[i];
	
}

void op_Fx65() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i)
		registers[i] = memory[index_register + i];

}
