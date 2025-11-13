#include "includes.h"

uint8_t fontset[FONTSET_SIZE] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* texture;
bool quit = false;
void init_sdl() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        SDL_Quit();
    }
    
    // SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"); // Set to "1" for linear filtering (blurry, but smoother)

    if(!SDL_CreateWindowAndRenderer("Chip8 Emulator", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        fprintf(stderr, "SDL_CreateWindowAndRenderer Error: %s\n", SDL_GetError());
        SDL_Quit();
    }
    
    if(!(texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,64,32))) {
        fprintf(stderr, "SDL_CreateWindowAndRenderer Error: %s\n", SDL_GetError());
        SDL_Quit();
    }

    SDL_SetRenderScale(renderer, 10, 10);
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

}

void init_chip8() {
    // initialize memory buffers and set variables
    memset(registers, 0, 16);
    memset(memory, 0, 4096);
    memset(stack, 0, 16 * sizeof(uint16_t));
    memset(keyboard, 0, 16 * sizeof(uint8_t));
    memset(video, 0, 64 * 32 * sizeof(uint32_t));

    pc = START_ADDRESS;
    sp = 0;
    delay_register = 0;
    sound_register = 0;
    index_register = 0;
    opcode = 0;
    drawflag = true;

    // load fonts into memory
	for (unsigned int i = 0; i < FONTSET_SIZE; i++) {
		memory[FONTSET_START_ADDRESS + i] = fontset[i];
	}
}

void load_rom(char* path) {
    // open file
    int fd = open(path, O_RDONLY);
    if(fd < 0) {
        fprintf(stderr, "Error loading rom file\n");
        SDL_Quit();
    }
    
    // get file size
    struct stat buf;
    fstat(fd, &buf);
    
    off_t file_sz = buf.st_size;
    if(!file_sz) {
        fprintf(stderr, "Error parsing rom file size\n");
        SDL_Quit();
    }

    // read file into buffer
    char* buffer = calloc(1, file_sz);

    ssize_t bytes_read = read(fd, buffer, file_sz);
    if (bytes_read != file_sz) {
        fprintf(stderr, "Failed to read ROM file completely\n");
        SDL_Quit();
    }
    
    // copy buffer into memory
    for(long i = 0; i < file_sz; i++) {
        memory[START_ADDRESS + i] = buffer[i];
    }

    free(buffer);
    close(fd);
}

void process_input() {
    SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_EVENT_QUIT:
                    quit = true;
                    break;

                case SDL_EVENT_KEY_DOWN:
                case SDL_EVENT_KEY_UP: {
                    int pressed = (e.type == SDL_EVENT_KEY_DOWN);
                    switch (e.key.key) {
                        case SDLK_1: keyboard[0x1] = pressed; break;
                        case SDLK_2: keyboard[0x2] = pressed; break;
                        case SDLK_3: keyboard[0x3] = pressed; break;
                        case SDLK_4: keyboard[0xC] = pressed; break;

                        case SDLK_Q: keyboard[0x4] = pressed; break;
                        case SDLK_W: keyboard[0x5] = pressed; break;
                        case SDLK_E: keyboard[0x6] = pressed; break;
                        case SDLK_R: keyboard[0xD] = pressed; break;

                        case SDLK_A: keyboard[0x7] = pressed; break;
                        case SDLK_S: keyboard[0x8] = pressed; break;
                        case SDLK_D: keyboard[0x9] = pressed; break;
                        case SDLK_F: keyboard[0xE] = pressed; break;

                        case SDLK_Z: keyboard[0xA] = pressed; break;
                        case SDLK_X: keyboard[0x0] = pressed; break;
                        case SDLK_C: keyboard[0xB] = pressed; break;
                        case SDLK_V: keyboard[0xF] = pressed; break;

                        case SDLK_ESCAPE: quit = true; break;
                    }
                    break;
                }
                
            }
        }
}

void cycle() {
    // fetch current operation from the rom that has been loaded into memory
    opcode = (memory[pc] << 8) | memory[pc + 1];
	pc += 2;


    #ifdef DEBUG
    printf("FETCH: PC(before)=%03x  opcode=%04x  PC(after)=%03x  I=%03x\n",
        pc-2, opcode, pc, index_register);
    if (sp > 0 && sp < 15) printf(" SP(top)=%04x\n", stack[sp-1]);
    #endif
    
    
	// decode and execute
    switch (opcode & 0xF000) {
        case 0x0000:
            //00kk
			switch(opcode & 0x00FF) {
				case 0x00E0:
                    op_00E0();
				break;
				case 0x00EE:
                    op_00EE();
				break;
				default: 
                    printf("Opcode error 0xxx -> %x\n",opcode ); 
                    break;
			} 
            break;

        case 0x1000:
            op_1nnn();
            break;
        
        case 0x2000:
            op_2nnn();
            break;
        
        case 0x3000: 
            op_3xkk();
            break;
        
        case 0x4000:
            op_4xkk();
            break;

        case 0x5000:
            op_5xy0();
            break;
        
        case 0x6000:
            op_6xkk();
            break;
        
        case 0x7000:
            op_7xkk();
            break;

        //8xyn
        case 0x8000:
            switch(opcode & 0x000F){
                case 0x0000:
                    op_8xy0();
                    break;
                case 0x0001:
                    op_8xy1();
                    break;
                case 0x0002:
                    op_8xy2();
                    break;
                case 0x0003:
                    op_8xy3();
                    break;
                case 0x0004:
                    op_8xy4();
                    break;
                case 0x0005:
                    op_8xy5();
                    break; 
                case 0x0006:
                    op_8xy6();
                    break;
                case 0x0007:
                    op_8xy7();
                    break;
                case 0x000E:
                    op_8xyE();
                    break; 	
                default: 
                    printf("Opcode error 8xxx -> %x\n",opcode);		
                    break;	
            }
            break;
			
        case 0x9000:
            op_9xy0();
			break;

        case 0xA000:
            op_Annn();
			break;

        case 0xB000:
            op_Bnnn();
			break;

        case 0xC000:
            op_Cxkk();
			break;

        //Dxyn
        case 0xD000:
            op_Dxyn();
            drawflag = true;
            break;
			
        //Exkk
        case 0xE000:
            switch(opcode & 0x00FF){
                case 0x009E:
                    op_Ex9E();
                    break;						
                case 0x00A1:
                    op_ExA1();
                    break;
            }
            break;

        //Fxkk
        case 0xF000:

            switch(opcode & 0x00FF){
                case 0x0007:
                    op_Fx07();
                    break;
                case 0x000A:
                    op_Fx0A();
                    break;
                case 0x0015:
                    op_Fx15();
                    break;
                case 0x0018:
                    op_Fx18();
                    break;
                case 0x001E:
                    op_Fx1E();
                    break;  
                case 0x0029:
                    op_Fx29();
                    break;
                case 0x0033:
                    op_Fx33();
                    break;
                case 0x0055:
                    op_Fx55();
                    break;
                case 0x0065:
                    op_Fx65();
                    break;

            }
            break;	
        default: 
            printf("OPCODE ERROR -> %x \n",opcode); 
            break;
    }
}

void render(void) {
    if (!drawflag) return;
    // draw pixel array to sdl texture
    SDL_RenderClear(renderer);
    SDL_UpdateTexture(texture, NULL, video, 64 * sizeof(uint32_t));

    // draw texture to renderer
    SDL_RenderTexture(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    drawflag = false;
}

int main(int argc, char** argv) {

    CHECK_ARGC;
    init_sdl();
    init_chip8();

    load_rom(argv[1]);

    uint32_t last_frame_time = SDL_GetTicks();
    while (!quit) {
        CHECK_TIME;
        process_input();
        
        for(int i = 0; i < CYCLES_PER_FRAME; i++)
            cycle();


        render();
            
        if (delay_register > 0) delay_register--;
        if (sound_register > 0) sound_register--;

        last_frame_time = SDL_GetTicks();
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}