#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>

typedef unsigned char BYTE;
typedef unsigned short WORD;

#define MEMORY_SIZE 0xFFF
#define STACK_START 0xEA0
#define PROGRAM_START 0x200
#define NUM_REGISTERS 16

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define CHANNELS 3
#define MULTIPLIER 20
#define SPRITE_WIDTH 8

// Program should reside in 0x200 - 0xE9F inclusive
BYTE mainMemory[MEMORY_SIZE];

BYTE dataRegisters[NUM_REGISTERS];

WORD regI;  // Address register
WORD PC;    // Program counter (16 bits but only 12 necessary)
WORD SP;    // Stack pointer

// 2D array of bytes representing the data on the screen at any given time
//BYTE screenData[SCREEN_WIDTH * SCREEN_HEIGHT * CHANNELS];
BYTE screenData[SCREEN_HEIGHT][SCREEN_WIDTH][CHANNELS];

void CPUReset();

WORD Fetch();
void DecodeExecute(WORD inst);

void Decode0000(WORD inst);
void Decode8000(WORD inst);
void DecodeE000(WORD inst);
void DecodeF000(WORD inst);

void Execute00E0();
void Execute00EE();
void Execute0NNN(WORD inst);
void Execute1NNN(WORD inst);
void Execute2NNN(WORD inst);
void Execute3XNN(WORD inst);
void Execute4XNN(WORD inst);
void Execute5XY0(WORD inst);
void Execute6XNN(WORD inst);
void Execute7XNN(WORD inst);
void Execute8XY0(WORD inst);
void Execute8XY1(WORD inst);
void Execute8XY2(WORD inst);
void Execute8XY3(WORD inst);
void Execute8XY4(WORD inst);
void Execute8XY5(WORD inst);
void Execute8XY6(WORD inst);
void Execute8XY7(WORD inst);
void Execute8XYE(WORD inst);
void Execute9XY0(WORD inst);
void ExecuteANNN(WORD inst);
void ExecuteBNNN(WORD inst);
void ExecuteCXNN(WORD inst);
void ExecuteDXYN(WORD inst);
void ExecuteEX9E(WORD inst);
void ExecuteEXA1(WORD inst);
void ExecuteFX07(WORD inst);
void ExecuteFX0A(WORD inst);
void ExecuteFX15(WORD inst);
void ExecuteFX18(WORD inst);
void ExecuteFX1E(WORD inst);
void ExecuteFX29(WORD inst);
void ExecuteFX33(WORD inst);
void ExecuteFX55(WORD inst);
void ExecuteFX65(WORD inst);

int main (int argc, char **argv)
{
    int execute = 1;
    WORD inst = 0;
    long inputSize = 0;

    // Check for valid usage
    if (argc != 2)
    {
        fprintf(stderr, "USAGE ERROR!\nCorrect Usage: chip8-emu <rom-file>.");
        return -1;
    }
    
    // Open ROM file
    FILE *input;
    if ((input = fopen(argv[1], "rb")) == NULL)
    {
        fprintf(stderr, "FILE I/O ERROR!\nCould not open file \"%s\".", argv[1]);
        return -1;
    }

    // Initialize CPU and load ROM into mainMemory
    CPUReset();

    // Obtain filesize
    fseek(input, 0, SEEK_END);
    inputSize = ftell(input);
    rewind(input);

    fread(&mainMemory[PROGRAM_START], inputSize, 1, input);

    fclose(input);

    srand(time(NULL));

    // for(int y = 0; y < SCREEN_HEIGHT; ++y) 
    // {
    //     for(int x = 0; x < SCREEN_WIDTH; ++x)
    //     {
    //         if((x % 2 == 0 && y % 2 == 0) || (x % 2 != 0 && y % 2 != 0))
    //         {
    //             screenData[y][x][0] = 0x00;
    //             screenData[y][x][1] = 0x00;
    //             screenData[y][x][2] = 0x00;
    //         }
    //         else
    //         {
    //             screenData[y][x][0] = 0x53;
    //             screenData[y][x][1] = 0x7D;
    //             screenData[y][x][2] = 0x55;
    //         }
    //     }
    // }

    SDL_Window *window = NULL;
    SDL_Surface *surface = NULL;
    SDL_Surface *graphics = NULL;

    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL ERROR!\nCould not initialize: %s", SDL_GetError());
    }
    else
    {
        window = SDL_CreateWindow("chip8-emu", SDL_WINDOWPOS_UNDEFINED,
                 SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH * MULTIPLIER,
                 SCREEN_HEIGHT * MULTIPLIER, SDL_WINDOW_SHOWN);
        if(window == NULL)
        {
            printf("SDL ERROR!\nWindow could not be created: %s", SDL_GetError());
        }
        // else
        // {
        //     surface = SDL_GetWindowSurface(window);
        //     graphics = SDL_CreateRGBSurfaceFrom(
        //         (void *)screenData,
        //         SCREEN_WIDTH,
        //         SCREEN_HEIGHT,
        //         CHANNELS * 8,
        //         SCREEN_WIDTH * CHANNELS,
        //         0x0000FF,
        //         0x00FF00,
        //         0xFF0000,
        //         0x000000
        //     );
        //     SDL_BlitScaled(graphics, NULL, surface, NULL);
        //     SDL_UpdateWindowSurface(window);
        // }
    }

    // Disassemble
    // while(PC < inputSize + PROGRAM_START)
    // {
    //     inst = Fetch();
    //     DecodeExecute(inst);
    // }

    int quit = 0;
    SDL_Event e;

    CPUReset();
    //While application is running
    while( !quit )
    {
        //Handle events on queue
        while( SDL_PollEvent( &e ) != 0 )
        {
            //User requests quit
            if( e.type == SDL_QUIT )
            {
                quit = 1;
            }
        }

        inst = Fetch();
        DecodeExecute(inst);

        surface = SDL_GetWindowSurface(window);
        graphics = SDL_CreateRGBSurfaceFrom(
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
        SDL_BlitScaled(graphics, NULL, surface, NULL);
        SDL_UpdateWindowSurface(window);
    }

    return 0;
}

void CPUReset()
{
    int i;
    regI = 0x000;
    PC = PROGRAM_START;
    SP = STACK_START;
    for(i = 0; i < NUM_REGISTERS; ++i)
    {
        dataRegisters[i] = 0x00;
    }
}

WORD Fetch()
{
    WORD inst = mainMemory[PC++];
    inst <<= 8;
    inst |= mainMemory[PC++];
    //printf("0x%04X:", inst);
    return inst;
}

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
    //printf("\n");
}

void Decode0000(WORD inst)
{
    switch(inst)
    {
        case 0x00E0: Execute00E0(); break;
        case 0x00EE: Execute00EE(); break;
        default:     Execute0NNN(inst); break;
    }
}

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

void DecodeE000(WORD inst)
{
    switch(inst & 0xF0FF)
    {
        case 0xE09E: ExecuteEX9E(inst); break;
        case 0xE0A1: ExecuteEXA1(inst); break;
        default: break;
    }
}

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

// Clear the screen
void Execute00E0()
{
    // printf(" 00E0");

    for(int y = 0; y < SCREEN_HEIGHT; ++y) 
    {
        for(int x = 0; x < SCREEN_WIDTH; ++x)
        {
            screenData[y][x][0] = 0x53;
            screenData[y][x][1] = 0x7D;
            screenData[y][x][2] = 0x55;
        }
    }
}

// Return
void Execute00EE()
{
    //printf(" 00EE");
    //WORD lo = mainMemory[--SP];
    //WORD hi = mainMemory[--SP] << 8; 
    PC = mainMemory[--SP] | (mainMemory[--SP] << 8);
}

void Execute0NNN(WORD inst)
{
    //printf(" 0NNN 0x%03X", inst & 0x0FFF);
}

// Jump/GoTo: jmp NNN
void Execute1NNN(WORD inst)
{
    //printf(" 1NNN 0x%03X", inst & 0x0FFF);
    PC = inst & 0x0FFF;
}

// Call subroutine: call NNN
void Execute2NNN(WORD inst)
{
    //printf(" 2NNN 0x%03X", inst & 0x0FFF);
    mainMemory[SP++] = (PC & 0xFF00) >> 8;
    mainMemory[SP++] = PC & 0x00FF;
    PC = inst & 0x0FFF;
}

// Skip next instruction if VX == NN
void Execute3XNN(WORD inst)
{
    //printf(" 3XNN V%X, %d", (inst & 0x0F00) >> 8, inst & 0x00FF);
    unsigned int x = (inst & 0x0F00) >> 8;
    int n = inst & 0x00FF;

    if(dataRegisters[x] == n)
        PC += 2;
}

// Skip next instruction if VX != NN
void Execute4XNN(WORD inst)
{
    //printf(" 4XNN V%X, %d", (inst & 0x0F00) >> 8, inst & 0x00FF);
    unsigned int x = (inst & 0x0F00) >> 8;
    int n = inst & 0x00FF;

    if(dataRegisters[x] != n)
        PC += 2;
}

// Skip next instruction if VX == VY
void Execute5XY0(WORD inst)
{
    //printf(" 5XY0 V%X, V%X", (inst & 0x0F00) >> 8, (inst & 0x00F0) >> 4);
    unsigned int x = (inst & 0x0F00) >> 8;
    unsigned int y = (inst & 0x00F0) >> 4;

    if(dataRegisters[x] == dataRegisters[y])
        PC += 2;
}

// Constant set: Vx = NN
void Execute6XNN(WORD inst)
{
    //printf(" 6XNN V%X, %d", (inst & 0x0F00) >> 8, inst & 0x00FF);
    unsigned int x = (inst & 0x0F00) >> 8;
    dataRegisters[x] = inst & 0x00FF;
}

// Increment VX by NN. Carry is NOT affected
void Execute7XNN(WORD inst)
{
    //printf(" 7XNN V%X, %d", (inst & 0x0F00) >> 8, inst & 0x00FF);
    unsigned int x = (inst & 0x0F00) >> 8;
    int n = inst & 0x00FF;

    dataRegisters[x] += n;
}

// VX = VY
void Execute8XY0(WORD inst)
{
    //printf(" 8XY0 V%X, V%X", (inst & 0x0F00) >> 8, (inst & 0x00F0) >> 4);
    unsigned int x = (inst & 0x0F00) >> 8;
    unsigned int y = (inst & 0x00F0) >> 4;

    dataRegisters[x] = dataRegisters[y];
}

// VX = VX | VY
void Execute8XY1(WORD inst)
{
    //printf(" 8XY1 V%X, V%X", (inst & 0x0F00) >> 8, (inst & 0x00F0) >> 4);
    unsigned int x = (inst & 0x0F00) >> 8;
    unsigned int y = (inst & 0x00F0) >> 4;

    dataRegisters[x] |= dataRegisters[y];
}

// VX = VX & VY
void Execute8XY2(WORD inst)
{
    //printf(" 8XY2 V%X, V%X", (inst & 0x0F00) >> 8, (inst & 0x00F0) >> 4);
    unsigned int x = (inst & 0x0F00) >> 8;
    unsigned int y = (inst & 0x00F0) >> 4;

    dataRegisters[x] &= dataRegisters[y];
}

// VX = VX ^ VY
void Execute8XY3(WORD inst)
{
    //printf(" 8XY3 V%X, V%X", (inst & 0x0F00) >> 8, (inst & 0x00F0) >> 4);
    unsigned int x = (inst & 0x0F00) >> 8;
    unsigned int y = (inst & 0x00F0) >> 4;

    dataRegisters[x] ^= dataRegisters[y];
}

// VX = VX + VY. Set VF if carry, otherwise unset
void Execute8XY4(WORD inst)
{
    //printf(" 8XY4 V%X, V%X", (inst & 0x0F00) >> 8, (inst & 0x00F0) >> 4);
    unsigned int x = (inst & 0x0F00) >> 8;
    unsigned int y = (inst & 0x00F0) >> 4;

    WORD check = dataRegisters[x] + dataRegisters[y];
    if(check > 0xFF)
        dataRegisters[0xF] = 1;
    else
        dataRegisters[0xF] = 0;

    dataRegisters[x] += dataRegisters[y];
}

// VX = VX - VY. Unset VF if borrow, otherwise set
void Execute8XY5(WORD inst)
{
    //printf(" 8XY5 V%X, V%X", (inst & 0x0F00) >> 8, (inst & 0x00F0) >> 4);
    unsigned int x = (inst & 0x0F00) >> 8;
    unsigned int y = (inst & 0x00F0) >> 4;

    if(dataRegisters[x] > dataRegisters[y])
        dataRegisters[0xF] = 1;
    else
        dataRegisters[0xF] = 0;

    dataRegisters[x] = dataRegisters[x] - dataRegisters[y];
}

// VX = VX >> 1. VF is set to least significant bit of VY before shift
void Execute8XY6(WORD inst)
{
    //printf(" 8XY6 V%X, V%X", (inst & 0x0F00) >> 8, (inst & 0x00F0) >> 4);
    unsigned int x = (inst & 0x0F00) >> 8;
    unsigned int y = (inst & 0x00F0) >> 4;

    dataRegisters[0xF] = dataRegisters[x] << 7 >> 7;

    dataRegisters[x] >>= 1;
}

// VX = VY - VX. Unset VF if borrow, otherwise set
void Execute8XY7(WORD inst)
{
    //printf(" 8XY7 V%X, V%X", (inst & 0x0F00) >> 8, (inst & 0x00F0) >> 4);
    unsigned int x = (inst & 0x0F00) >> 8;
    unsigned int y = (inst & 0x00F0) >> 4;

    if(dataRegisters[y] > dataRegisters[x])
        dataRegisters[0xF] = 1;
    else
        dataRegisters[0xF] = 0;

    dataRegisters[x] = dataRegisters[y] - dataRegisters[x];
}

// VX = VX >> 1. VF is set to least significant bit of VY before shift
void Execute8XYE(WORD inst)
{
    //printf(" 8XYE V%X, V%X", (inst & 0x0F00) >> 8, (inst & 0x00F0) >> 4);
    unsigned int x = (inst & 0x0F00) >> 8;
    unsigned int y = (inst & 0x00F0) >> 4;

    dataRegisters[0xF] = dataRegisters[x] >> 7;

    dataRegisters[x] <<= 1;
}

// Skip next instruction if VX != VY
void Execute9XY0(WORD inst)
{
    //printf(" 9XY0 V%X, V%X", (inst & 0x0F00) >> 8, (inst & 0x00F0) >> 4);
    unsigned int x = (inst & 0x0F00) >> 8;
    unsigned int y = (inst & 0x00F0) >> 4;

    if(dataRegisters[x] != dataRegisters[y])
        PC += 2;
}

// Set address register: I = NNN
void ExecuteANNN(WORD inst)
{
    //printf(" ANNN 0x%03X", inst & 0x0FFF);
    regI = inst & 0x0FFF;
}

// Jump to NNN plus the value of V0
void ExecuteBNNN(WORD inst)
{
    //printf(" BNNN 0x%03X", inst & 0x0FFF);
    int n = inst & 0x0FFF;

    PC = n + dataRegisters[0];
}

// VX = rand() & NN
void ExecuteCXNN(WORD inst)
{
    //printf(" CXNN V%X, %d", (inst & 0x0F00) >> 8, inst & 0x00FF);
    unsigned int x = (inst & 0x0F00) >> 8;
    int n = inst & 0x00FF;

    dataRegisters[x] = (rand() % 256) & n;
}

// Draw to screenData
void ExecuteDXYN(WORD inst)
{
    //printf(" DXYN V%X, V%X, %d",
    //    (inst & 0x0F00) >> 8, (inst & 0x00F0) >> 4, inst & 0x000F);
    unsigned int regX = (inst & 0x0F00) >> 8;
    unsigned int regY = (inst & 0x00F0) >> 4;
    unsigned int startX = dataRegisters[regX];
    unsigned int startY = dataRegisters[regY];
    unsigned int height = inst & 0x000F;

    int line, pixelPos, mask, x, y;
    int drawColorR, drawColorG, drawColorB;
    dataRegisters[0xF] = 0;

    for(line = 0; line < height; ++line)
    {
        BYTE data = mainMemory[regI + line];

        for(pixelPos = 0; pixelPos < SPRITE_WIDTH; ++pixelPos)
        {
            mask = 1 << (SPRITE_WIDTH - pixelPos - 1);

            if ((data & mask) != 0x00)
            {
                x = startX + pixelPos;
                y = startY + line;
                drawColorR = drawColorG = drawColorB = 0x00;

                if(screenData[y][x][0] == 0x00 &&
                   screenData[y][x][1] == 0x00 &&
                   screenData[y][x][2] == 0x00)
                {
                    dataRegisters[0xF] = 1;
                    drawColorR = 0x53;
                    drawColorG = 0x7D;
                    drawColorB = 0x55;
                }

                screenData[y][x][0] = drawColorR;
                screenData[y][x][1] = drawColorG;
                screenData[y][x][2] = drawColorB;
            }
        }
    }
}

void ExecuteEX9E(WORD inst)
{
    //printf(" EX9E V%X", (inst & 0x0F00) >> 8);
}

void ExecuteEXA1(WORD inst)
{
    //printf(" EXA1 V%X", (inst & 0x0F00) >> 8);
}

void ExecuteFX07(WORD inst)
{
    //printf(" FX07 V%X", (inst & 0x0F00) >> 8);
}

void ExecuteFX0A(WORD inst)
{
    //printf(" FX0A V%X", (inst & 0x0F00) >> 8);
}

void ExecuteFX15(WORD inst)
{
    //printf(" FX15 V%X", (inst & 0x0F00) >> 8);
}

void ExecuteFX18(WORD inst)
{
    //printf(" FX18 V%X", (inst & 0x0F00) >> 8);
}

void ExecuteFX1E(WORD inst)
{
    //printf(" FX1E V%X", (inst & 0x0F00) >> 8);
}

void ExecuteFX29(WORD inst)
{
    //printf(" FX29 V%X", (inst & 0x0F00) >> 8);
}

void ExecuteFX33(WORD inst)
{
    //printf(" FX33 V%X", (inst & 0x0F00) >> 8);
}

void ExecuteFX55(WORD inst)
{
    //printf(" FX55 V%X", (inst & 0x0F00) >> 8);
}

void ExecuteFX65(WORD inst)
{
    //printf(" FX65 V%X", (inst & 0x0F00) >> 8);
}