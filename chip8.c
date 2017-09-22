#include <stdio.h>
#include <stdlib.h>

typedef unsigned char BYTE;
typedef unsigned short WORD;

// Program should reside in 0x200 - 0xE9F inclusive
BYTE mainMemory[0xFFF];
WORD STACK_START = 0xEA0;

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
WORD* SP;   // Stack pointer

int main (int argc, char** argv)
{
    printf("One day, I shall be an emulator!\n");

    return 0;
}