#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "chip8.h"

int main(int argc, char **argv)
{
    WORD inst = 0;      // Current instruction for this cycle
    long inputSize = 0; // Size of input ROM

    // Check for valid usage
    if(argc != 3)
    {
        fprintf(stderr, "USAGE ERROR!\nCorrect Usage: chip8-emu <rom-file> <graphics-multiple>.");
        return -1;
    }
    
    // Open ROM file
    FILE *input;
    if((input = fopen(argv[1], "rb")) == NULL)
    {
        fprintf(stderr, "FILE I/O ERROR!\nCould not open file \"%s\".", argv[1]);
        return -1;
    }

    // Check for valid multiplier
    if(atoi(argv[2]) <= 0)
    {
        fprintf(stderr, "GRAPHICS ERROR!\n<graphics-multiple> must be a positive integer");
        return -1;
    }
    const unsigned int MULTIPLIER = atoi(argv[2]);

    // Obtain filesize
    fseek(input, 0, SEEK_END);
    inputSize = ftell(input);
    rewind(input);

    // Read ROM into mainMemory
    fread(&mainMemory[PROGRAM_START], inputSize, 1, input);
    fclose(input);

    // Seed random number generator
    srand(time(NULL));

    SDL_Window *window = NULL;      // Window rendered to
    SDL_Renderer *renderer = NULL;  // Used to render textures
    Mix_Chunk *beep = NULL;         // Stores the beep effect

    // Non-zero return indicates unrecoverable SDL initialization error. Abort
    if(InitializeSDL(&window, &renderer, &beep, MULTIPLIER) != 0)
        return -1;

    int quit = 0;       // Continue execution until the user quits
    SDL_Event event;    // Represents user input

    InitializeCPU();

    // Begin countdown registers
    SDL_TimerID timerID = SDL_AddTimer(17, DecrementTimers, beep);

    // Main Loop. One iteration represents a single chip8 cycle
    while(!quit)
    {
        //Handle events on queue
        while(SDL_PollEvent(&event) != 0)
        {
            CheckForInput(event);

            //User requests quit
            if(event.type == SDL_QUIT )
                quit = 1;
        }

        // Fetch and execute inst, affecting cpu state
        inst = Fetch();
        DecodeExecute(inst);

        // Draw new graphics based on changed state
        if(Draw(&window, &renderer) != 0)
            return -1;
    }

    // SDL cleanup
    SDL_RemoveTimer(timerID);
    Mix_FreeChunk(beep);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    return 0;
}

// General SDL plumbing...
int InitializeSDL(SDL_Window **window, SDL_Renderer **renderer, Mix_Chunk **beep, const unsigned int MULTIPLIER)
{
    // Initialize SDL
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) < 0)
    {
        fprintf(stderr, "SDL ERROR!\nCould not initialize: %s", SDL_GetError());
        return -1;
    }
    else
    {
        // Create window
        *window = SDL_CreateWindow("chip8-emu", SDL_WINDOWPOS_UNDEFINED,
                  SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH * MULTIPLIER,
                  SCREEN_HEIGHT * MULTIPLIER, SDL_WINDOW_SHOWN);
        if(*window == NULL)
        {
            fprintf(stderr, "SDL ERROR!\nWindow could not be created: %s", SDL_GetError());
            return -1;
        }
        else
        {
            // Create renderer
            *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
            if(*renderer == NULL)
            {
                fprintf(stderr, "SDL ERROR!\nRenderer could not be created: %s", SDL_GetError());
                return -1;
            }
            SDL_SetRenderDrawColor(*renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        }
    }

    // Initialize SDL_mixer extension
    if(Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
        fprintf(stderr, "SDL ERROR!\nAudio was not initialized: %s", SDL_GetError());

    // Load beep sound effect
    *beep = Mix_LoadWAV("beep.wav");
    if(*beep == NULL)
        fprintf(stderr, "SDL ERROR!\nCouldn't load audio file: %s", SDL_GetError());
    
    return 0;
}

// Set CPU constructs to appropriate values for initial execution
void InitializeCPU()
{
    int i;

    // Zero out all registers
    regI = 0x000;
    regDT = 0x000;
    regST = 0x000;
    for(i = 0; i < NUM_REGISTERS; ++i)
        dataRegisters[i] = 0x00;

    // Initialize pointers to appropriate values
    PC = PROGRAM_START;
    SP = STACK_START;

    // Initialize stock hexadecimal sprites
    InitNumericalSprites();
    
    // Initialize all keys to be unpressed
    for(i = 0; i < NUM_KEYS; ++i)
        inputKeys[i] = 0x00;
}

// Write hard-coded stock sprites into reserved section of memory. Each sprite
// represents a hexadecimal digit.
void InitNumericalSprites()
{
    // The sprite 0
    mainMemory[0x000] = 0xF0;
    mainMemory[0x001] = 0x90;
    mainMemory[0x002] = 0x90;
    mainMemory[0x003] = 0x90;
    mainMemory[0x004] = 0xF0;

    // The sprite 1
    mainMemory[0x005] = 0x20;
    mainMemory[0x006] = 0x60;
    mainMemory[0x007] = 0x20;
    mainMemory[0x008] = 0x20;
    mainMemory[0x009] = 0x70;

    // The sprite 2
    mainMemory[0x00A] = 0xF0;
    mainMemory[0x00B] = 0x10;
    mainMemory[0x00C] = 0xF0;
    mainMemory[0x00D] = 0x80;
    mainMemory[0x00E] = 0xF0;

    // The sprite 3
    mainMemory[0x00F] = 0xF0;
    mainMemory[0x010] = 0x10;
    mainMemory[0x011] = 0xF0;
    mainMemory[0x012] = 0x10;
    mainMemory[0x013] = 0xF0;

    // The sprite 4
    mainMemory[0x014] = 0x90;
    mainMemory[0x015] = 0x90;
    mainMemory[0x016] = 0xF0;
    mainMemory[0x017] = 0x10;
    mainMemory[0x018] = 0x10;

    // The sprite 5
    mainMemory[0x019] = 0xF0;
    mainMemory[0x01A] = 0x80;
    mainMemory[0x01B] = 0xF0;
    mainMemory[0x01C] = 0x10;
    mainMemory[0x01D] = 0xF0;

    // The sprite 6
    mainMemory[0x01E] = 0xF0;
    mainMemory[0x01F] = 0x80;
    mainMemory[0x020] = 0xF0;
    mainMemory[0x021] = 0x90;
    mainMemory[0x022] = 0xF0;

    // The sprite 7
    mainMemory[0x023] = 0xF0;
    mainMemory[0x024] = 0x10;
    mainMemory[0x025] = 0x20;
    mainMemory[0x026] = 0x40;
    mainMemory[0x027] = 0x40;

    // The sprite 8
    mainMemory[0x028] = 0xF0;
    mainMemory[0x029] = 0x90;
    mainMemory[0x02A] = 0xF0;
    mainMemory[0x02B] = 0x90;
    mainMemory[0x02C] = 0xF0;

    // The sprite 9
    mainMemory[0x02D] = 0xF0;
    mainMemory[0x02E] = 0x90;
    mainMemory[0x02F] = 0xF0;
    mainMemory[0x030] = 0x10;
    mainMemory[0x031] = 0xF0;

    // The sprite A
    mainMemory[0x032] = 0xF0;
    mainMemory[0x033] = 0x90;
    mainMemory[0x034] = 0xF0;
    mainMemory[0x035] = 0x90;
    mainMemory[0x036] = 0x90;

    // The sprite B
    mainMemory[0x037] = 0xE0;
    mainMemory[0x038] = 0x90;
    mainMemory[0x039] = 0xE0;
    mainMemory[0x03A] = 0x90;
    mainMemory[0x03B] = 0xE0;

    // The sprite C
    mainMemory[0x03C] = 0xF0;
    mainMemory[0x03D] = 0x80;
    mainMemory[0x03E] = 0x80;
    mainMemory[0x03F] = 0x80;
    mainMemory[0x040] = 0xF0;

    // The sprite D
    mainMemory[0x041] = 0xE0;
    mainMemory[0x042] = 0x90;
    mainMemory[0x043] = 0x90;
    mainMemory[0x044] = 0x90;
    mainMemory[0x045] = 0xE0;

    // The sprite E
    mainMemory[0x046] = 0xF0;
    mainMemory[0x047] = 0x80;
    mainMemory[0x048] = 0xF0;
    mainMemory[0x049] = 0x80;
    mainMemory[0x04A] = 0xF0;

    // The sprite F
    mainMemory[0x04B] = 0xF0;
    mainMemory[0x04C] = 0x80;
    mainMemory[0x04D] = 0xF0;
    mainMemory[0x04E] = 0x80;
    mainMemory[0x04F] = 0x80;
}

// Check parameter event for keyboard input from user
void CheckForInput(SDL_Event event)
{
    // If a key is pressed...
    if(event.type == SDL_KEYDOWN)
    {
        // ...find out which key and...
        int key = -1 ;
        switch(event.key.keysym.sym)
        {
            case SDLK_x: key = 0;  break;
            case SDLK_1: key = 1;  break;
            case SDLK_2: key = 2;  break;
            case SDLK_3: key = 3;  break;
            case SDLK_q: key = 4;  break;
            case SDLK_w: key = 5;  break;
            case SDLK_e: key = 6;  break;
            case SDLK_a: key = 7;  break;
            case SDLK_s: key = 8;  break;
            case SDLK_d: key = 9;  break;
            case SDLK_z: key = 10; break;
            case SDLK_c: key = 11; break;
            case SDLK_4: key = 12; break;
            case SDLK_r: key = 13; break;
            case SDLK_f: key = 14; break;
            case SDLK_v: key = 15; break;
            default: break;
        }
        if (key != -1)
        {
            // ...activate that key
            inputKeys[key] = 0xFF;
        }
    }
    // If a key is released...
    else if(event.type == SDL_KEYUP)
    {
        // ...find out which key and...
        int key = -1 ;
        switch(event.key.keysym.sym)
        {
            case SDLK_x: key = 0;  break;
            case SDLK_1: key = 1;  break;
            case SDLK_2: key = 2;  break;
            case SDLK_3: key = 3;  break;
            case SDLK_q: key = 4;  break;
            case SDLK_w: key = 5;  break;
            case SDLK_e: key = 6;  break;
            case SDLK_a: key = 7;  break;
            case SDLK_s: key = 8;  break;
            case SDLK_d: key = 9;  break;
            case SDLK_z: key = 10; break;
            case SDLK_c: key = 11; break;
            case SDLK_4: key = 12; break;
            case SDLK_r: key = 13; break;
            case SDLK_f: key = 14; break;
            case SDLK_v: key = 15; break;
            default: break;
        }
        if (key != -1)
        {
            // ... deactivate that key
            inputKeys[key] = 0x00;
        }
    }
}

// Draw graphics to screen using data stored in screenData
int Draw(SDL_Window **window, SDL_Renderer **renderer)
{
    // New surface created from data in screenData
    SDL_Surface *graphics = SDL_CreateRGBSurfaceFrom(
        (void *)screenData,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        CHANNELS * 8,
        SCREEN_WIDTH * CHANNELS,
        0x0000FF,
        0x00FF00,
        0xFF0000,
        0x000000
    );

    // Create texture from graphics
    SDL_Texture *texture = SDL_CreateTextureFromSurface(*renderer, graphics);
    if(texture == NULL)
    {
        fprintf(stderr, "SDL ERROR!\nTexture could not be created: %s", SDL_GetError());
        return -1;
    }

    // Render texture
    SDL_RenderClear(*renderer);
    SDL_RenderCopy(*renderer, texture, NULL, NULL);
    SDL_RenderPresent(*renderer);

    // Free SDL resources
    SDL_FreeSurface(graphics);
    SDL_DestroyTexture(texture);

    return 0;
}

// Callback function for SDL_Timer. Executes once for each interval
Uint32 DecrementTimers(Uint32 interval, void *param)
{
    // Decrement delay and sound registers
    if(regDT > 0)
        --regDT;
    if(regST > 0)
        --regST;
    
    // If sound regsiter is positive, play beep
    if(regST > 0)
        Mix_PlayChannel(-1, (Mix_Chunk *)param, 0);

    // This function is called again after this interval has elapsed
    return interval;
}

// Fetch the next instruction for execution
WORD Fetch()
{
    // Bitwise logic is necessary because memory is indexed by BYTE and an instruction
    // is a WORD (two BYTES)
    WORD inst = mainMemory[PC++];
    inst <<= 8;
    inst |= mainMemory[PC++];

    return inst;
}

// Calls the correct execute function for a given instruction or the correct decode
// function for a set of possible instructions
void DecodeExecute(WORD inst)
{
    switch(inst & 0xF000)
    {
        case 0x0000: Decode0000(inst);  break;
        case 0x1000: Execute1NNN(inst); break;
        case 0x2000: Execute2NNN(inst); break;
        case 0x3000: Execute3XNN(inst); break;
        case 0x4000: Execute4XNN(inst); break;
        case 0x5000: Execute5XY0(inst); break;
        case 0x6000: Execute6XNN(inst); break;
        case 0x7000: Execute7XNN(inst); break;
        case 0x8000: Decode8000(inst);  break;
        case 0x9000: Execute9XY0(inst); break;
        case 0xA000: ExecuteANNN(inst); break;
        case 0xB000: ExecuteBNNN(inst); break;
        case 0xC000: ExecuteCXNN(inst); break;
        case 0xD000: ExecuteDXYN(inst); break;
        case 0xE000: DecodeE000(inst);  break;
        case 0xF000: DecodeF000(inst);  break;
        default: break;
    }
}

// Calls the correct execution function for a given instruction that begins with 0
void Decode0000(WORD inst)
{
    switch(inst)
    {
        case 0x00E0: Execute00E0(); break;
        case 0x00EE: Execute00EE(); break;
        default:     Execute0NNN(inst); break;
    }
}

// Calls the correct execution function for a given instruction that begins with 8
void Decode8000(WORD inst)
{
    switch(inst & 0x000F)
    {
        case 0x0000: Execute8XY0(inst); break;
        case 0x0001: Execute8XY1(inst); break;
        case 0x0002: Execute8XY2(inst); break;
        case 0x0003: Execute8XY3(inst); break;
        case 0x0004: Execute8XY4(inst); break;
        case 0x0005: Execute8XY5(inst); break;
        case 0x0006: Execute8XY6(inst); break;
        case 0x0007: Execute8XY7(inst); break;
        case 0x000E: Execute8XYE(inst); break;
        default: break;
    }
}

// Calls the correct execution function for a given instruction that begins with E
void DecodeE000(WORD inst)
{
    switch(inst & 0xF0FF)
    {
        case 0xE09E: ExecuteEX9E(inst); break;
        case 0xE0A1: ExecuteEXA1(inst); break;
        default: break;
    }
}

// Calls the correct execution function for a given instruction that begins with F
void DecodeF000(WORD inst)
{
    switch(inst & 0x00FF)
    {
        case 0x0007: ExecuteFX07(inst); break;
        case 0x000A: ExecuteFX0A(inst); break;
        case 0x0015: ExecuteFX15(inst); break;
        case 0x0018: ExecuteFX18(inst); break;
        case 0x001E: ExecuteFX1E(inst); break;
        case 0x0029: ExecuteFX29(inst); break;
        case 0x0033: ExecuteFX33(inst); break;
        case 0x0055: ExecuteFX55(inst); break;
        case 0x0065: ExecuteFX65(inst); break;
        default: break;
    }
}

// 00E0 - CLS : Clear the screen
void Execute00E0()
{
    for(int y = 0; y < SCREEN_HEIGHT; ++y) 
    {
        for(int x = 0; x < SCREEN_WIDTH; ++x)
        {
            screenData[y][x][0] = 0xFF;
            screenData[y][x][1] = 0xFF;
            screenData[y][x][2] = 0xFF;
        }
    }
}

// 00EE - RET : Return from subroutine
void Execute00EE()
{
    // Memory is indexed by BYTE so fetch both BYTES of the WORD in memory at SP
    WORD lo = mainMemory[--SP];
    WORD hi = mainMemory[--SP] << 8; 
    PC = lo | hi;
}

// 0NNN - SYS addr : Jump to machine code routine at NNN
void Execute0NNN(WORD inst)
{
    // This is only necessary in actual hardware
}

// 1NNN - JP addr : Jump to location NNN
void Execute1NNN(WORD inst)
{
    PC = inst & 0x0FFF;
}

// 2NNN - CALL addr : Call subroutine at NNN
void Execute2NNN(WORD inst)
{
    // Memory is indexed by BYTE so store both BYTES of the WORD in memory at SP
    mainMemory[SP++] = (PC & 0xFF00) >> 8;
    mainMemory[SP++] = PC & 0x00FF;
    PC = inst & 0x0FFF;
}

// 3XNN - SE Vx, NN : Skip next instruction if Vx == NN
void Execute3XNN(WORD inst)
{
    unsigned int x = (inst & 0x0F00) >> 8;
    int n = inst & 0x00FF;

    if(dataRegisters[x] == n)
        PC += 2;
}

// 4XNN - SNE Vx, NN : Skip next instruction if Vx != NN
void Execute4XNN(WORD inst)
{
    unsigned int x = (inst & 0x0F00) >> 8;
    int n = inst & 0x00FF;

    if(dataRegisters[x] != n)
        PC += 2;
}

// 5XY0 - SE Vx, Vy : Skip next instruction if Vx == Vy
void Execute5XY0(WORD inst)
{
    unsigned int x = (inst & 0x0F00) >> 8;
    unsigned int y = (inst & 0x00F0) >> 4;

    if(dataRegisters[x] == dataRegisters[y])
        PC += 2;
}

// 6XNN - LD Vx, NN : Load NN into Vx (Vx == NN)
void Execute6XNN(WORD inst)
{
    unsigned int x = (inst & 0x0F00) >> 8;
    dataRegisters[x] = inst & 0x00FF;
}

// 7XNN - ADD Vx, NN : Add NN to Vx and store result into Vx
void Execute7XNN(WORD inst)
{
    unsigned int x = (inst & 0x0F00) >> 8;
    int n = inst & 0x00FF;

    dataRegisters[x] += n;
}

// 8XY0 - LD Vx, Vy : Load Vy into Vx (Vx == Vy)
void Execute8XY0(WORD inst)
{
    unsigned int x = (inst & 0x0F00) >> 8;
    unsigned int y = (inst & 0x00F0) >> 4;

    dataRegisters[x] = dataRegisters[y];
}

// 8XY1 - OR Vx, Vy : Bitwise or Vx and Vy and store result into Vx
void Execute8XY1(WORD inst)
{
    unsigned int x = (inst & 0x0F00) >> 8;
    unsigned int y = (inst & 0x00F0) >> 4;

    dataRegisters[x] |= dataRegisters[y];
}

// 8XY2 - AND Vx, Vy : Bitwise and Vx and Vy and store result into Vx
void Execute8XY2(WORD inst)
{
    unsigned int x = (inst & 0x0F00) >> 8;
    unsigned int y = (inst & 0x00F0) >> 4;

    dataRegisters[x] &= dataRegisters[y];
}

// 8XY3 - XOR Vx, Vy : Bitwise xor Vx and Vy and store result into Vx
void Execute8XY3(WORD inst)
{
    unsigned int x = (inst & 0x0F00) >> 8;
    unsigned int y = (inst & 0x00F0) >> 4;

    dataRegisters[x] ^= dataRegisters[y];
}

// 8XY4 - ADD Vx, Vy : Add Vx and Vy and store result into Vx
void Execute8XY4(WORD inst)
{
    unsigned int x = (inst & 0x0F00) >> 8;
    unsigned int y = (inst & 0x00F0) >> 4;

    // Check for carry and set VF appropriately
    WORD check = dataRegisters[x] + dataRegisters[y];
    if(check > 0xFF)
        dataRegisters[0xF] = 1;
    else
        dataRegisters[0xF] = 0;

    dataRegisters[x] += dataRegisters[y];
}

// 8XY5 - SUB Vx, Vy : Subtract Vy from Vx and store result into Vx
void Execute8XY5(WORD inst)
{
    unsigned int x = (inst & 0x0F00) >> 8;
    unsigned int y = (inst & 0x00F0) >> 4;

    // Check for borrow and set VF appropriately
    if(dataRegisters[x] > dataRegisters[y])
        dataRegisters[0xF] = 1;
    else
        dataRegisters[0xF] = 0;

    dataRegisters[x] = dataRegisters[x] - dataRegisters[y];
}

// 8XY6 - SHR Vx {, Vy} : Set Vx to Vx >> 1
void Execute8XY6(WORD inst)
{
    unsigned int x = (inst & 0x0F00) >> 8;

    // Set VF to the least significant bit of Vx
    dataRegisters[0xF] = dataRegisters[x] << 7 >> 7;

    dataRegisters[x] >>= 1;
}

// 8XY7 - SUBN Vx, Vy : Subtract Vx from Vy and store result into Vx
void Execute8XY7(WORD inst)
{
    unsigned int x = (inst & 0x0F00) >> 8;
    unsigned int y = (inst & 0x00F0) >> 4;

    // Check for borrow and set VF appropriately
    if(dataRegisters[y] > dataRegisters[x])
        dataRegisters[0xF] = 1;
    else
        dataRegisters[0xF] = 0;

    dataRegisters[x] = dataRegisters[y] - dataRegisters[x];
}

// 8XY6 - SHL Vx {, Vy} : Set Vx to Vx << 1
void Execute8XYE(WORD inst)
{
    unsigned int x = (inst & 0x0F00) >> 8;

    // Set VF to the least significant bit of Vx
    dataRegisters[0xF] = dataRegisters[x] >> 7;

    dataRegisters[x] <<= 1;
}

// 9XY0 - SNE Vx, Vy : Skip next instruction if Vx != Vy
void Execute9XY0(WORD inst)
{
    unsigned int x = (inst & 0x0F00) >> 8;
    unsigned int y = (inst & 0x00F0) >> 4;

    if(dataRegisters[x] != dataRegisters[y])
        PC += 2;
}

// ANNN - LD I, addr : Set regI to NNN
void ExecuteANNN(WORD inst)
{
    regI = inst & 0x0FFF;
}

// BNNN - JP V0, addr : Jump to address NNN + V0
void ExecuteBNNN(WORD inst)
{
    int n = inst & 0x0FFF;
    PC = n + dataRegisters[0];
}

// CXNN - RND Vx, NN : Set Vx to random BYTE & NN
void ExecuteCXNN(WORD inst)
{
    unsigned int x = (inst & 0x0F00) >> 8;
    int n = inst & 0x00FF;

    dataRegisters[x] = (rand() % 256) & n;
}

// DXYN - DRW Vx, Vy, N : Draw N BYTE sprite from memory at regI to screen data starting
// at position Vx, Vy
void ExecuteDXYN(WORD inst)
{
    unsigned int regX = (inst & 0x0F00) >> 8;
    unsigned int regY = (inst & 0x00F0) >> 4;
    unsigned int startX = dataRegisters[regX];
    unsigned int startY = dataRegisters[regY];
    unsigned int height = inst & 0x000F;

    int line, pixelPos, mask, x, y;
    int drawColorR, drawColorG, drawColorB;
    dataRegisters[0xF] = 0;

    // For each horizontal line in the sprite (where height == N)...
    for(line = 0; line < height; ++line)
    {
        // Load sprite data from memory
        BYTE data = mainMemory[regI + line];

        // For each pixel in the horizontal line...
        for(pixelPos = 0; pixelPos < SPRITE_WIDTH; ++pixelPos)
        {
            mask = 1 << (SPRITE_WIDTH - pixelPos - 1);

            // If a pixel should be flipped
            if ((data & mask) != 0x00)
            {
                // Determine position to be fliped
                x = startX + pixelPos;
                y = startY + line;
                drawColorR = drawColorG = drawColorB = 0x00;

                // If a pixel is to be erased
                if(screenData[y][x][0] == 0x00 &&
                   screenData[y][x][1] == 0x00 &&
                   screenData[y][x][2] == 0x00)
                {
                    // Set VF and change color to erase pixel
                    dataRegisters[0xF] = 1;
                    drawColorR = drawColorG = drawColorB = 0xFF;
                }

                // Draw pixel to screen
                screenData[y][x][0] = drawColorR;
                screenData[y][x][1] = drawColorG;
                screenData[y][x][2] = drawColorB;
            }
        }
    }
}

// EX9E - SKP Vx : Skip next instruction if key Vx is pressed
void ExecuteEX9E(WORD inst)
{
    unsigned int x = (inst & 0x0F00) >> 8;
    unsigned int keyIndex = dataRegisters[x];

    if(inputKeys[keyIndex] == 0xFF)
        PC += 2;
}

// EXA1 = SKNP Vx : Skip next instruction if key VX is NOT pressed
void ExecuteEXA1(WORD inst)
{
    unsigned int x = (inst & 0x0F00) >> 8;
    unsigned int keyIndex = dataRegisters[x];

    if(inputKeys[keyIndex] == 0x00)
        PC += 2;
}

// FX07 - LD Vx, DT : Set Vx to the value of the delay timer
void ExecuteFX07(WORD inst)
{
    unsigned int x = (inst & 0x0F00) >> 8;
    dataRegisters[x] = regDT;
}

// FX0A - LD Vx, K : Load Vx with the key that was pressed
void ExecuteFX0A(WORD inst)
{
    unsigned int x = (inst & 0x0F00) >> 8;
    int i;

    int key = -1;
    for(i = 0; i < NUM_KEYS; ++i)
    {
        if(inputKeys[i] == 0xFF)
            key = i;
    }

    // This is a blocking operation. Repeat until a key is pressed
    if(key == -1)
        PC -= 2;
    else
        dataRegisters[x] = key;
}

// FX15 - LD DT, Vx : Set the delay timer to the value of Vx
void ExecuteFX15(WORD inst)
{
    unsigned int x = (inst & 0x0F00) >> 8;
    regDT = dataRegisters[x];
}

// FX18 - LD ST, Vx : Set the sound timer to the value of Vx
void ExecuteFX18(WORD inst)
{
    unsigned int x = (inst & 0x0F00) >> 8;
    regST = dataRegisters[x];
}

// FX1E - ADD I, Vx : Set regI to itself plus the value of Vx
void ExecuteFX1E(WORD inst)
{
    unsigned int x = (inst & 0x0F00) >> 8;
    regI = regI + dataRegisters[x];
}

// FX29 - LD F, Vx : Set regI to the location for the hex sprite in Vx
void ExecuteFX29(WORD inst)
{
    unsigned int x = (inst & 0x0F00) >> 8;
    regI = mainMemory[dataRegisters[x] * 5];
}

// FX33 - LD B, Vx : Store a decimal representation of Vx in memory at regI to regI + 2
void ExecuteFX33(WORD inst)
{
    unsigned int x = (inst & 0x0F00) >> 8;

    int hundreds = dataRegisters[x] / 100;
    int tens = (dataRegisters[x] % 100) / 10;
    int ones = dataRegisters[x] % 10;

    mainMemory[regI] = hundreds;
    mainMemory[regI + 1] = tens;
    mainMemory[regI + 2] = ones;
}

// FX55 - LD [I], Vx : Store registers V0 through Vx into memory at regI
void ExecuteFX55(WORD inst)
{
    unsigned int x = (inst & 0x0F00) >> 8;
    int i;
    
    for(i = 0; i <= x; ++i)
        mainMemory[regI + i] = dataRegisters[i];
}

// FX65 - LD Vx, [I] : Load registers V0 through Vx from memory at regI
void ExecuteFX65(WORD inst)
{
    unsigned int x = (inst & 0x0F00) >> 8;
    int i;

    for(i = 0; i <= x; ++i)
        dataRegisters[i] = mainMemory[regI + i];
}