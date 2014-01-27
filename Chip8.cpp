#include "Chip8.hpp"
#include <iostream>
#include <fstream>
#include <ctype.h> // Requiered for debug_dump_mem
#include <stdio.h> // Requiered for debug_dump_mem

void Chip8::clear_screen()
{
    // clear video ram
    for (int i = 0; i < VIDEO_WIDTH; i++)
    {
	for (int j = 0; j < VIDEO_HEIGHT; j++)
	{
	    gfx[i][j] = 0;
	}
    }
}

int Chip8::initialize(unsigned char start_time, std::string rom_path)
{
    reset();
    rand_state = start_time;

    Chip8::rom_path = rom_path;
    if (load_rom())
	return 1;
    return 0;
}

void Chip8::reset()
{
    delay_timer = 0;
    sound_timer = 0;
    opcode = 0x0000;
    I = 0;
    pc = PROGRAM_START;
    sp = 0;

    clear_screen();

    // clear key registers
    for (int i = 0; i < KEYS_SIZE; i++)
	key[i] = 0;

    // clear memory
    for (int i = 0; i < MEMORY_SIZE; i++)
	memory[i] = 0;

    // clear V registers
    for (int i = 0; i < VREG_SIZE; i++)
	V[i] = 0;

    // clear stack
    for (int i = 0; i < STACK_SIZE; i++)
	stack[i] = 0;

    // load fontset
    for (int i = 0; i < 80; i++)
	memory[i] = chip8_fontset[i];
}

int Chip8::load_rom()
{
    std::streampos size;

    std::cout << "Loading rom: " << rom_path << std::endl;
    
    std::ifstream file(rom_path, std::ios::in|std::ios::binary|std::ios::ate);
    if (file.is_open())
    {
	size = file.tellg();
	if (size > MAX_ROM_SIZE)
	{
	    std::cout << "Error: ROM too big" << std::endl;
	    return 1;
	}
	file.seekg(0, std::ios::beg);
	file.read((char*)&memory[PROGRAM_START], size);
	file.close();
    }
    else
    {
	std::cout << "Unable to open " << rom_path << std::endl;
	return 1;
    }
    return 0;

}

unsigned int Chip8::run_instruction()
{
    // fetch opcode
    opcode = memory[pc] << 8 | memory[pc + 1];

    // TODO: Do the replacement
    unsigned char *vx = &V[(opcode & 0x0F00) >> 8];
    unsigned char *vy = &V[(opcode & 0x00F0) >> 4];
    unsigned short nnn = opcode & 0x0FFF;
    
    // decode opcode
    switch (opcode & 0xF000)
    {
    case 0x0000:
	switch (opcode & 0x0FFF)
	{
	case 0x00E0: // 00E0: Clears the screen.
	    clear_screen();
	    pc += 2;
	    break;
	case 0x00EE: // 00EE: Returns from a subroutine.
	    sp--;
	    pc = stack[sp];
	    pc += 2;
	    break;
	default:
	    printf("Unknown opcode: 0x%04X\n", opcode);
	    printf("No RCA 1802 found in the system :(\n");
	}
	break;
    case 0x1000: //1NNN: Jumps to address NNN.
	pc = nnn;
	break;
    case 0x2000: // 2NNN: Calls subroutine at NNN.
	stack[sp] = pc;
	sp++;
	pc = nnn;
	break;
    case 0x3000: // 3XNN: Skips the next instruction if VX equals NN.
	if (*vx == (opcode & 0x00FF))
	    pc += 4;
	else
	    pc += 2;
	break;
    case 0x4000: // 4XNN: Skips the next instruction if VX doesn't equal NN.
	if (*vx != (opcode & 0x00FF))
	    pc += 4;
	else
	    pc += 2;
	break;
    case 0x5000: // 5XY0: Skips the next instruction if VX equals VY.
	if (*vx == *vy)
	    pc += 4;
	else
	    pc += 2;
	break;
    case 0x6000: // 6XNN: Sets VX to NN.
	*vx = opcode & 0x00FF;
	pc += 2;
	break;
    case 0x7000: // 7XNN: Adds NN to VX.
	*vx += opcode & 0x00FF;
	pc += 2;
	break;
    case 0x8000:
	switch (opcode & 0x000F)
	{
	case 0x0000: // 8XY0: Sets VX to the value of VY.
	    *vx = *vy;
	    pc += 2;
	    break;
	case 0x0001: // 8XY1: Sets VX to VX or VY.
	    *vx =
		*vx | *vy;
	    pc += 2;
	    break;
	case 0x0002: // 8XY2: Sets VX to VX and VY.
	    *vx =
		*vx & *vy;
	    pc += 2;
	    break;
	case 0x0003: // 8XY3: Sets VX to VX xor VY.
	    *vx =
		*vx ^ *vy;
	    pc += 2;
	    break;
	case 0x0004: // 8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
	    if(*vy > (0xFF - *vx))
		V[0xF] = 1; // carry
	    else
		V[0xF] = 0;
	    *vx += *vy;
	    pc += 2;
	    break;
	case 0x0005: // 8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
	    if(*vx >= V[(opcode & 0x0F0) >> 4])
		V[0xF] = 1; // borrow
	    else
		V[0xF] = 0;
	    *vx -= *vy;
	    pc += 2;
	    break;
	case 0x0006: // 8XY6: Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift.
	    V[0xF] = *vx & 0x0001;
	    *vx = *vx >> 1;
	    pc += 2;
	    break;
	case 0x0007: // 8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
	    if(*vy >= *vx)
		V[0xF] = 1; // borrow
	    else
		V[0xF] = 0;
	    *vx =
		*vy - *vx;
	    pc += 2;
	    break;
	case 0x000E: // 8XYE: Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift.
	    V[0xF] = (*vx & 0x80) >> 7;
	    *vx = *vx << 1;
	    pc += 2;
	    break;
	default:
	    printf("Unknown opcode: 0x%04X\n", opcode);    
	}
	break;
    case 0x9000: // 9XY0: Skips the next instruction if VX doesn't equal VY.
	if (*vx != *vy)
	    pc += 4;
	else
	    pc += 2;
	break;
    case 0xA000: // ANNN: Sets I to the address NNN.
	I = nnn;
	pc += 2;
	break;
    case 0xB000: // BNNN: Jumps to the address NNN plus V0.
	pc = nnn + V[0x0];
	break;
    case 0xC000: // CXNN: Sets VX to a random number and NN.
	*vx = rand() & (nnn);
	pc += 2;
	break;
    case 0xD000: // DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
    {
	unsigned short x = *vx;
	unsigned short y = *vy;
	unsigned short height = opcode & 0x000F;
	unsigned short pixel;
	V[0xF] = 0;
	for (int yline = 0; yline < height; yline++)
	{
	    pixel = memory[I + yline];
	    for (int xline = 0; xline < 8; xline++)
	    {
		if ((pixel & (0x80 >> xline)) != 0)
		{
		    if (gfx[x + xline][y + yline] == 1)
			V[0xF] = 1;
		    gfx[x + xline][y + yline] ^= 1;
		}
	    }
	}
	pc += 2;
    }
    break;
    case 0xE000:
	switch (opcode & 0x00FF)
	{
	case 0x009E: // EX9E: Skips the next instruction if the key stored in VX is pressed.
	    if (key[*vx] == 1)
		pc += 4;
	    else
		pc += 2;
	    break;
	case 0x00A1: // EXA1: Skips the next instruction if the key stored in VX isn't pressed.
	    if (key[*vx] == 0)
		pc += 4;
	    else
		pc += 2;
	    break;
	default:
	    printf("Unknown opcode: 0x%04X\n", opcode);    
	}
	break;
    case 0xF000:
	switch (opcode & 0x00FF)
	{
	case 0x0007: // FX07: Sets VX to the value of the delay timer.
	    *vx = delay_timer;
	    pc += 2;
	    break;
	case 0x000A: // FX0A: A key press is awaited, and then stored in VX.
	    for (int i = 0; i < KEYS_SIZE; i++)
	    {
		if (key[i] == 1)
		{
		    *vx = i;
		    pc += 2;
		}
	    }
	    break;
	case 0x0015: // FX15: Sets the delay timer to VX.
	    delay_timer = *vx;
	    pc += 2;
	    break;
	case 0x0018: // FX18: Sets the sound timer to VX.
	    sound_timer = *vx;
	    pc += 2;
	    break;
	case 0x001E: // FX1E: Adds VX to I.
	    if (I + *vx > 0xFFF)
		V[0xF] = 1;
	    else
		V[0xF] = 0;
	    I += *vx;
	    pc += 2;
	    break;
	case 0x0029: // FX29: Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
	    I = *vx * 5;
	    pc += 2;
	    break;
	case 0x0033: // FX33: Stores the Binary-coded decimal representation of VX, with the most significant of three digits at the address in I.
	    memory[I] = *vx / 100;
	    memory[I + 1] = (*vx / 10) % 10;
	    memory[I + 2] = (*vx % 100) % 10;
	    pc += 2;
	    break;
	case 0x0055: // FX55: Stores V0 to VX in memory starting at address I.
	    for (int i = 0; i < ((opcode & 0x0F00) >> 8) + 1; i++)
		memory[I + i] = V[i];
	    pc += 2;
	    break;
	case 0x0065: // FX65: Fills V0 to VX with values from memory starting at address I.
	    for (int i = 0; i < ((opcode & 0x0F00) >> 8) + 1; i++)
		V[i] = memory[I + i];
	    pc += 2;
	    break;	    
	default:
	    printf("Unknown opcode: 0x%04X\n", opcode);
	    // Pass test 23
	    /* 
	    for (int i = 0; i < 8; i++)
		V[i] = i;
	    pc += 2;
	    */	
	}
	break;
    default:
	printf("Unknown opcode: 0x%04X\n", opcode);
    }

    return 1;
}

void Chip8::emulate_hardware()
{
    if (delay_timer > 0)
	delay_timer--;

    if (sound_timer > 0)
	sound_timer--;
}

void Chip8::debug_dump_mem()
{
    unsigned char *buf = (unsigned char*)&memory;
    int i, j;
    for (i=0; i<MEMORY_SIZE; i+=16) {
	printf("%06x: ", i);
	for (j=0; j<16; j++) 
	    if (i+j < MEMORY_SIZE)
		printf("%02x ", buf[i+j]);
	    else
		printf("   ");
	printf(" ");
	for (j=0; j<16; j++) 
	    if (i+j < MEMORY_SIZE)
		printf("%c", isprint(buf[i+j]) ? buf[i+j] : '.');
	printf("\n");
    }
    printf("\n");
}

void Chip8::debug_dump_reg()
{
    printf("Keys: \t\t");
    for (int i = 0; i < KEYS_SIZE; i++)
	printf("%u", key[i]);
    printf("\n");

    printf("Delay timer: \t%u\n", delay_timer);
    printf("Sound timer: \t%u\n", sound_timer);
    printf("Opcode: \t%04x\n", opcode);
    printf("I: \t\t%04x\n", I);
    printf("pc: \t\t%04x\n", pc);

    printf("VReg: \t\t");
    for (int i  = 0; i < VREG_SIZE / 2; i++)
	printf("V%X: %02x  ", i, V[i]);
    printf("\n\t\t");
    for (int i  = VREG_SIZE / 2; i < VREG_SIZE; i++)
	printf("V%X: %02x  ", i, V[i]);
    printf("\n");
    printf("sp: \t\t%02x\n", sp);

    printf("stack: \t\t");
    for (int i  = 0; i < STACK_SIZE / 2; i++)
	printf("%04x ", stack[i]);
    printf("\n\t\t");
    for (int i  = STACK_SIZE / 2; i < STACK_SIZE; i++)
	printf("%04x ", stack[i]);
    printf("\n");
    
    printf("\n");
}
