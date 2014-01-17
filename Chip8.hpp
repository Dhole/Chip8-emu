#ifndef __Chip8_H__
#define __Chip8_H__

#include <iostream>

class Chip8
{
public:
    static const unsigned int VIDEO_WIDTH = 64;
    static const unsigned int VIDEO_HEIGHT = 32;
    static const unsigned int KEYS_SIZE = 16;
    static const unsigned int MEMORY_SIZE = 4096;
    static const unsigned int VREG_SIZE = 16;
    static const unsigned int STACK_SIZE = 16;
    static const unsigned int PROGRAM_START = 0x200;

    static const unsigned int MAX_ROM_SIZE = MEMORY_SIZE - PROGRAM_START;

    // hardware
    unsigned char gfx[VIDEO_WIDTH][VIDEO_HEIGHT];
    unsigned char key[KEYS_SIZE];
    unsigned char delay_timer;
    unsigned char sound_timer;

private:
    // CPU
    unsigned short opcode;
    unsigned char memory[MEMORY_SIZE];
    unsigned char V[VREG_SIZE];
    unsigned short I;
    unsigned short pc;
    unsigned short stack[STACK_SIZE];
    unsigned char sp;

    unsigned char chip8_fontset[80] =
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

    unsigned char rand_state;
    std::string rom_path;

public:

    //Chip8();
    //virtual ~Chip8();

    void clear_screen();
    
    /* Initializes all the registers and memory to 0
       Sets pc to 0x200 where the program will be loaded */
    int initialize(unsigned char start_time, std::string rom_path);

    void reset();

    /* Loads game given by path at position 0x200 of memory
       Returns 0 upon succes or 1 otherwise */
    int load_rom();

    /* Fetches, decodes and runs instruction from memory at pc
       Returns the number of cycles spent */
    unsigned int run_instruction();

    /* Emulates the internal hardware (timers) */
    void emulate_hardware();

    // debug functions

    /* Dumps the current state of memory to stdout */
    void debug_dump_mem();

    /* Dumps the current state of the registers */
    void debug_dump_reg();
};

#endif /* defined(__Chip8_H__) */
