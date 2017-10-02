#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

typedef unsigned char BYTE;
typedef unsigned short WORD;

// CPU constants
#define MEMORY_SIZE 0xFFF
#define STACK_START 0xEA0
#define PROGRAM_START 0x200
#define NUM_REGISTERS 16
#define NUM_KEYS 16

// Graphics constants
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define CHANNELS 3
#define SPRITE_WIDTH 8

// Program should reside in 0x200 - 0xE9F inclusive
BYTE mainMemory[MEMORY_SIZE];

// Main general purpose registers with the exception of 0xF
// 0xF is used for flags
BYTE dataRegisters[NUM_REGISTERS];

// Represents the current status of input keys
// 0xFF is pressed, 0x00 is unpressed
BYTE inputKeys[NUM_KEYS];

BYTE regDT; // Delay Timer
BYTE regST; // Sound Timer
WORD regI;  // Address register
WORD PC;    // Program counter
WORD SP;    // Stack pointer

// 3D array of bytes representing the data on the screen at any given time
BYTE screenData[SCREEN_HEIGHT][SCREEN_WIDTH][CHANNELS];

// SDL plumbing stuff...
int InitializeSDL(SDL_Window **window, SDL_Renderer **renderer, Mix_Chunk **beep, const unsigned int MULTIPLIER);

// Helper functions for CPU
void InitializeCPU();
void InitNumericalSprites();
void CheckForInput(SDL_Event event);
int Draw(SDL_Window **window, SDL_Renderer **renderer);
Uint32 DecrementTimers(Uint32 interval, void *param);

// Implement CPU execution. All the work is done here
WORD Fetch();
void DecodeExecute(WORD inst);

// These ensure the correct execute function is called for a given instruction
void Decode0000(WORD inst);
void Decode8000(WORD inst);
void DecodeE000(WORD inst);
void DecodeF000(WORD inst);

// Emulate the execution for the given instruction
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