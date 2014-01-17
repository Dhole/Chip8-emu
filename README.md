Chip8-emu
=========

Chip8 emulator written in C++ using SDL2

Keys:

	Chip8 buttons:
	0 to 9 are mapped on the numeric keypad. 
	A - F are mapped to QAZWSX keys

	Function keys:
	r: dump registers
	d: dump memory
	enter: pause emulation
	backspace: reset Chip8
	esc: exit

Compile with:
g++ Chip8.cpp main.cpp -o chip8_emu -l SDL2 -l SDL2_mixer -std=c++11
