#include <stdio.h>
#include <stdlib.h>

typedef unsigned char BYTE;
typedef unsigned short WORD;

const WORD MEMORY_SIZE = 0xFFF;
const WORD STACK_START = 0xEA0;
const WORD PROGRAM_START = 0x200;

const BYTE SCREEN_WIDTH = 64;
const BYTE SCREEN_HEIGHT = 32;

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

    // In addition VF is carry flag, in subtraction it is the "not borrow"
    // flag. In draw inst, it is set upon pixel collision
    BYTE VF;
} dataRegisters;

WORD regI;  // Address register
WORD PC;    // Program counter (16 bits but only 12 necessary)
WORD SP;    // Stack pointer

BYTE screenData[SCREEN_WIDTH][SCREEN_HEIGHT];

void CPUReset();

int main (int argc, char **argv)
{
    printf("One day, I shall be an emulator!\n");

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

    // load in ROM
    FILE *in;
    in = fopen("C:\\INVADERS", "rb");
    fread(&mainMemory[PROGRAM_START], MEMORY_SIZE, 1, in);
    fclose(in);
}