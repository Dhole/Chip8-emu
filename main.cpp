#include "SDL2/SDL.h"
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <fstream>

#include "Chip8.hpp"


const Uint32 width = 64;
const Uint32 height = 32;
int scale = 8;
const unsigned char NCOLORS = 2;

const Uint32 fps = 60;
const Uint32 freq = 400;
const Uint32 minframetime = 1000 / fps;

// Key mapping

// Chip8 Keypad
const Uint32 KEY_0 = SDL_SCANCODE_KP_0;
const Uint32 KEY_1 = SDL_SCANCODE_KP_1;
const Uint32 KEY_2 = SDL_SCANCODE_KP_2;
const Uint32 KEY_3 = SDL_SCANCODE_KP_3;
const Uint32 KEY_4 = SDL_SCANCODE_KP_4;
const Uint32 KEY_5 = SDL_SCANCODE_KP_5;
const Uint32 KEY_6 = SDL_SCANCODE_KP_6;
const Uint32 KEY_7 = SDL_SCANCODE_KP_7;
const Uint32 KEY_8 = SDL_SCANCODE_KP_8;
const Uint32 KEY_9 = SDL_SCANCODE_KP_9;
const Uint32 KEY_A = SDL_SCANCODE_Q;
const Uint32 KEY_B = SDL_SCANCODE_A;
const Uint32 KEY_C = SDL_SCANCODE_Z;
const Uint32 KEY_D = SDL_SCANCODE_W;
const Uint32 KEY_E = SDL_SCANCODE_S;
const Uint32 KEY_F = SDL_SCANCODE_X;

// Function keys
const Uint32 KEY_DUMP_RAM = SDLK_d;
const Uint32 KEY_DUMP_REGS = SDLK_r;
const Uint32 KEY_PAUSE = SDLK_RETURN;
const Uint32 KEY_RESET = SDLK_BACKSPACE;
const Uint32 KEY_EXIT = SDLK_ESCAPE;

Uint32 palette[NCOLORS];

SDL_Window *window = nullptr;
SDL_Surface *screen = nullptr;
Mix_Music *beep_sound = nullptr;

bool pause = false;

int init_SDL() 
{
    std::cout << "Initializing SDL..." << std::endl;

    //Initialize all SDL subsystems
    if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
    {
	std::cout << SDL_GetError() << std::endl;
	return 1;
    }
  
    window = SDL_CreateWindow("Chip8 Emulator by Dhole", 100, 100, 
			      width * scale, height * scale, 
			      SDL_WINDOW_SHOWN);
    if (window == nullptr)
    {
	std::cout << SDL_GetError() << std::endl;
	return 1;
    }

    screen = SDL_GetWindowSurface(window);

    //Initialize SDL_mixer
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 1, 4096 ) == -1)
    {
	std::cout << SDL_GetError() << std::endl;
	return 1;
    }
    
    //Load the beep sound
    beep_sound= Mix_LoadMUS("beep.wav");
    if(beep_sound == nullptr)
    {
	std::cout << SDL_GetError() << std::endl;
	return 1;
    }
   
    return 0;
}

void stop_SDL()
{
    std::cout << "Stopping SDL..." << std::endl;
    Mix_FreeMusic(beep_sound);
    Mix_CloseAudio();
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void render_SDL(unsigned char gfx[][Chip8::VIDEO_HEIGHT])
{
    SDL_Rect pixel = {0, 0, scale, scale};
  
    for (int i = 0; i < width; i++)
    {
	pixel.x = i * scale;
	for (int j = 0; j < height; j++)
	{
	    pixel.y = j * scale;
	    SDL_FillRect(screen, &pixel, palette[gfx[i][j]]);
	}
    }
    SDL_UpdateWindowSurface(window);
}

void play_audio(unsigned char sound_timer)
{
    if (sound_timer > 0)
    {
	if (Mix_PlayingMusic() == 0)
	    Mix_PlayMusic(beep_sound, -1);
    }
    else
    {
	if (Mix_PlayingMusic() == 1)
	    Mix_HaltMusic();
    }
}

void setup_palette()
{
    palette[0] = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
    palette[1] = SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF);
}

int setup_graphics()
{
    if (init_SDL())
	return 1;
    setup_palette();
   
    return 0;
}

void update_key_reg(unsigned char key[])
{
    const Uint8 *keystates = SDL_GetKeyboardState( NULL );
    key[0x0] = keystates[KEY_0];
    key[0x1] = keystates[KEY_1];
    key[0x2] = keystates[KEY_2];
    key[0x3] = keystates[KEY_3];
    key[0x4] = keystates[KEY_4];
    key[0x5] = keystates[KEY_5];
    key[0x6] = keystates[KEY_6];
    key[0x7] = keystates[KEY_7];
    key[0x8] = keystates[KEY_8];
    key[0x9] = keystates[KEY_9];
    key[0xA] = keystates[KEY_A];
    key[0xB] = keystates[KEY_B];
    key[0xC] = keystates[KEY_C];
    key[0xD] = keystates[KEY_D];
    key[0xE] = keystates[KEY_E];
    key[0xF] = keystates[KEY_F];
}

int process_event(SDL_Event *e, Chip8* myChip8)
{
    if (e->type == SDL_QUIT)
	return 1;
    if (!e->key.repeat)
    {
	if (e->type == SDL_KEYDOWN)
	{
	    switch (e->key.keysym.sym)
	    {
/*
	    case KEY_A:
		std::cout << "key Z pressed" << std::endl;
		break;
*/
	    case KEY_DUMP_RAM:
		myChip8->debug_dump_mem();
		break;
	    case KEY_DUMP_REGS:
		myChip8->debug_dump_reg();
		break;
	    case KEY_RESET:
		myChip8->reset();
		myChip8->load_rom();
		break;
	    case KEY_PAUSE:
		pause = pause ^ true;
		break;
	    case KEY_EXIT:
		return 1;
		break;
	    default:
		break;
	    }
	}
/*      
	if (e->type == SDL_KEYUP)
	{
	    switch (e->key.keysym.sym)
	    {
	    case KEY_A:
		std::cout << "key Z unpressed" << std::endl;
		break;
	    default:
		break;
	    }
	}
*/	
	if (e->type == SDL_KEYUP || e->type == SDL_KEYDOWN)
	    update_key_reg(myChip8->key);
    }
    return 0;
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
	std::cout << "Usage: " << argv[0] << " ROM" << std::endl;
	return 1;
    }
    std::string rom_path = argv[1];

    if (setup_graphics())
	return 1;

    SDL_Event e;
    bool quit = false;
    Uint32 start_time;
    Uint32 last_time;
    Uint32 elapsed_time;
    Chip8 myChip8;

    std::cout << "Initializing Chip8..." << std::endl;
    if (myChip8.initialize(SDL_GetTicks(), rom_path))
	return 1;
        
    float cycles = 0;
    last_time = SDL_GetTicks();
    while (!quit)
    {
	start_time = SDL_GetTicks();
	elapsed_time = start_time - last_time;
		
	while (SDL_PollEvent(&e))
	{
	    if (process_event(&e, &myChip8))
		quit = true;
	}

	if (!pause)
	{
	    myChip8.emulate_hardware();

	    cycles += (float)elapsed_time * freq / 1000;
	    while (cycles > 1)
		cycles -= myChip8.run_instruction();

	    play_audio(myChip8.sound_timer);
	}
	render_SDL(myChip8.gfx);

	// Limit frame rate
	if (SDL_GetTicks() - start_time < minframetime)
	    SDL_Delay(minframetime - (SDL_GetTicks() - start_time));
	
	last_time = start_time;
    }

    stop_SDL();

    return 0;
}
