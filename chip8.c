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

// 2D array of bytes representing the data on the screen at any given time
BYTE screenData[SCREEN_WIDTH][SCREEN_HEIGHT];

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

    // Disassemble
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
    WORD inst = 0;
    inst = mainMemory[PC];
    inst <<= 8;
    inst |= mainMemory[PC+1];
    PC += 2;
    printf("0x%04X:", inst);
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
    printf("\n");
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

void Execute00E0()
{
    printf(" 00E0");
}

void Execute00EE()
{
    printf(" 00EE");
}

void Execute0NNN(WORD inst)
{
    printf(" 0NNN 0x%03X", inst & 0x0FFF);
}

void Execute1NNN(WORD inst)
{
    printf(" 1NNN 0x%03X", inst & 0x0FFF);
}

void Execute2NNN(WORD inst)
{
    printf(" 2NNN 0x%03X", inst & 0x0FFF);
}

void Execute3XNN(WORD inst)
{
    printf(" 3XNN V%X, %d", (inst & 0x0F00) >> 8, inst & 0x00FF);
}

void Execute4XNN(WORD inst)
{
    printf(" 4XNN V%X, %d", (inst & 0x0F00) >> 8, inst & 0x00FF);
}

void Execute5XY0(WORD inst)
{
    printf(" 5XY0 V%X, V%X", (inst & 0x0F00) >> 8, (inst & 0x00F0) >> 4);
}

void Execute6XNN(WORD inst)
{
    printf(" 6XNN V%X, %d", (inst & 0x0F00) >> 8, inst & 0x00FF);
}

void Execute7XNN(WORD inst)
{
    printf(" 7XNN V%X, %d", (inst & 0x0F00) >> 8, inst & 0x00FF);
}

void Execute8XY0(WORD inst)
{
    printf(" 8XY0 V%X, V%X", (inst & 0x0F00) >> 8, (inst & 0x00F0) >> 4);
}

void Execute8XY1(WORD inst)
{
    printf(" 8XY1 V%X, V%X", (inst & 0x0F00) >> 8, (inst & 0x00F0) >> 4);
}

void Execute8XY2(WORD inst)
{
    printf(" 8XY2 V%X, V%X", (inst & 0x0F00) >> 8, (inst & 0x00F0) >> 4);
}

void Execute8XY3(WORD inst)
{
    printf(" 8XY3 V%X, V%X", (inst & 0x0F00) >> 8, (inst & 0x00F0) >> 4);
}

void Execute8XY4(WORD inst)
{
    printf(" 8XY4 V%X, V%X", (inst & 0x0F00) >> 8, (inst & 0x00F0) >> 4);
}

void Execute8XY5(WORD inst)
{
    printf(" 8XY5 V%X, V%X", (inst & 0x0F00) >> 8, (inst & 0x00F0) >> 4);
}

void Execute8XY6(WORD inst)
{
    printf(" 8XY6 V%X, V%X", (inst & 0x0F00) >> 8, (inst & 0x00F0) >> 4);
}

void Execute8XY7(WORD inst)
{
    printf(" 8XY7 V%X, V%X", (inst & 0x0F00) >> 8, (inst & 0x00F0) >> 4);
}

void Execute8XYE(WORD inst)
{
    printf(" 8XYE V%X, V%X", (inst & 0x0F00) >> 8, (inst & 0x00F0) >> 4);
}

void Execute9XY0(WORD inst)
{
    printf(" 9XY0 V%X, V%X", (inst & 0x0F00) >> 8, (inst & 0x00F0) >> 4);
}

void ExecuteANNN(WORD inst)
{
    printf(" ANNN 0x%03X", inst & 0x0FFF);
}

void ExecuteBNNN(WORD inst)
{
    printf(" BNNN 0x%03X", inst & 0x0FFF);
}

void ExecuteCXNN(WORD inst)
{
    printf(" CXNN V%X, %d", (inst & 0x0F00) >> 8, inst & 0x00FF);
}

void ExecuteDXYN(WORD inst)
{
    printf(" DXYN V%X, V%X, %d",
        (inst & 0x0F00) >> 8, (inst & 0x00F0) >> 4, inst & 0x000F);
}

void ExecuteEX9E(WORD inst)
{
    printf(" EX9E V%X", (inst & 0x0F00) >> 8);
}

void ExecuteEXA1(WORD inst)
{
    printf(" EXA1 V%X", (inst & 0x0F00) >> 8);
}

void ExecuteFX07(WORD inst)
{
    printf(" FX07 V%X", (inst & 0x0F00) >> 8);
}

void ExecuteFX0A(WORD inst)
{
    printf(" FX0A V%X", (inst & 0x0F00) >> 8);
}

void ExecuteFX15(WORD inst)
{
    printf(" FX15 V%X", (inst & 0x0F00) >> 8);
}

void ExecuteFX18(WORD inst)
{
    printf(" FX18 V%X", (inst & 0x0F00) >> 8);
}

void ExecuteFX1E(WORD inst)
{
    printf(" FX1E V%X", (inst & 0x0F00) >> 8);
}

void ExecuteFX29(WORD inst)
{
    printf(" FX29 V%X", (inst & 0x0F00) >> 8);
}

void ExecuteFX33(WORD inst)
{
    printf(" FX33 V%X", (inst & 0x0F00) >> 8);
}

void ExecuteFX55(WORD inst)
{
    printf(" FX55 V%X", (inst & 0x0F00) >> 8);
}

void ExecuteFX65(WORD inst)
{
    printf(" FX65 V%X", (inst & 0x0F00) >> 8);
}