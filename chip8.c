#include <stdio.h>
#include <stdlib.h>

typedef unsigned char BYTE;
typedef unsigned short WORD;

#define MEMORY_SIZE 0xFFF
#define STACK_START 0xEA0
#define PROGRAM_START 0x200

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

// Program should reside in 0x200 - 0xE9F inclusive
BYTE mainMemory[MEMORY_SIZE];

struct DataRegisters
{
    BYTE V0;
    BYTE V1;
    BYTE V2;
    BYTE V3;
    BYTE V4;
    BYTE V5;
    BYTE V6;
    BYTE V7;
    BYTE V8;
    BYTE V9;
    BYTE VA;
    BYTE VB;
    BYTE VC;
    BYTE VD;
    BYTE VE;

    // In addition VF is carry flag, in subtraction it is the "not borrow" flag. In draw
    // inst, it is set upon pixel collision
    BYTE VF;
} dataRegisters;

WORD regI;  // Address register
WORD PC;    // Program counter (16 bits but only 12 necessary)
WORD SP;    // Stack pointer

BYTE screenData[SCREEN_WIDTH][SCREEN_HEIGHT];

void CPUReset();

WORD Fetch();
void DecodeExecute(WORD inst);

void Execute1NNN(WORD inst);
void Execute2NNN(WORD inst);
void Execute3XNN(WORD inst);
void Execute4XNN(WORD inst);
void Execute5XY0(WORD inst);
void Execute6XNN(WORD inst);
void Execute7XNN(WORD inst);

void Execute9XY0(WORD inst);

int main (int argc, char **argv)
{
    int execute = 1;
    WORD inst = 0;

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
    fread(&mainMemory[PROGRAM_START], MEMORY_SIZE, 1, input);
    fclose(input);

    while(PC < 0x284)
    {
        inst = Fetch();
        DecodeExecute(inst);
    }

    return 0;
}

void CPUReset()
{
    // Reset register values
    regI = 0x000;
    PC = PROGRAM_START;
    SP = STACK_START;
    dataRegisters.V0 = 0x00;
    dataRegisters.V1 = 0x00;
    dataRegisters.V2 = 0x00;
    dataRegisters.V3 = 0x00;
    dataRegisters.V4 = 0x00;
    dataRegisters.V5 = 0x00;
    dataRegisters.V6 = 0x00;
    dataRegisters.V7 = 0x00;
    dataRegisters.V8 = 0x00;
    dataRegisters.V9 = 0x00;
    dataRegisters.VA = 0x00;
    dataRegisters.VB = 0x00;
    dataRegisters.VC = 0x00;
    dataRegisters.VD = 0x00;
    dataRegisters.VE = 0x00;
    dataRegisters.VF = 0x00;
}

WORD Fetch()
{
    WORD inst = mainMemory[PC++];
    inst <<= 8;
    inst |= mainMemory[PC++];
    printf("0x%04X:", inst);
    return inst;
}

void DecodeExecute(WORD inst)
{
    switch(inst & 0xF000)
    {
        case 0x0000: break;
        case 0x1000: Execute1NNN(inst); break;
        case 0x2000: Execute2NNN(inst); break;
        case 0x3000: Execute3XNN(inst); break;
        case 0x4000: Execute4XNN(inst); break;
        case 0x5000: Execute5XY0(inst); break;
        case 0x6000: Execute6XNN(inst); break;
        case 0x7000: Execute7XNN(inst); break;
        case 0x8000: break;
        case 0x9000: Execute9XY0(inst); break;
        case 0xA000: break;
        case 0xB000: break;
        case 0xC000: break;
        case 0xD000: break;
        case 0xE000: break;
        case 0xF000: break;
        default: fprintf(stderr, "DECODE ERROR!\nInvalid opcode encountered.\n");
                 exit(-1);
    }
    printf("\n");
}

void Execute1NNN(WORD inst)
{
    printf(" jump 0x%03X", inst & 0x0FFF);
}

void Execute2NNN(WORD inst)
{
    printf(" call 0x%03X", inst & 0x0FFF);
}

void Execute3XNN(WORD inst)
{
    // skip next inst if equal
    printf(" seq V%X, %d", (inst & 0x0F00) >> 8, inst & 0x00FF);
}

void Execute4XNN(WORD inst)
{
    // skip next inst if NOT equal
    printf(" sneq V%X, %d", (inst & 0x0F00) >> 8, inst & 0x00FF);
}

void Execute5XY0(WORD inst)
{
    // skip next inst if NOT equal
    printf(" seq V%X, V%X", (inst & 0x0F00) >> 8, (inst & 0x00F0) >> 4);
}

void Execute6XNN(WORD inst)
{
    printf(" set V%X, %d", (inst & 0x0F00) >> 8, inst & 0x00FF);
}

void Execute7XNN(WORD inst)
{
    // increment register by constant
    printf(" incr V%X, %d", (inst & 0x0F00) >> 8, inst & 0x00FF);
}


void Execute9XY0(WORD inst)
{
    printf(" sneq V%X, V%X", (inst & 0x0F00) >> 8, (inst & 0x00F0) >> 4);
}