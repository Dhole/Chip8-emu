#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <iostream>
#include <fstream>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

#include "Chip8.hpp"



const Uint32 width = 64;
const Uint32 height = 32;
Uint16 scale = 8;
const unsigned char NCOLORS = 2;

const Uint32 fps = 60;
const Uint32 freq = 400;
const Uint32 minframetime = 1000 / fps;

// Key mapping

// Chip8 Keypad
/* // SDL2
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
*/ // end SDL2
// SDL2
const Uint32 KEY_0 = SDLK_x;
const Uint32 KEY_1 = SDLK_1;
const Uint32 KEY_2 = SDLK_2;
const Uint32 KEY_3 = SDLK_3;
const Uint32 KEY_4 = SDLK_q;
const Uint32 KEY_5 = SDLK_w;
const Uint32 KEY_6 = SDLK_e;
const Uint32 KEY_7 = SDLK_a;
const Uint32 KEY_8 = SDLK_s;
const Uint32 KEY_9 = SDLK_d;
const Uint32 KEY_A = SDLK_z;
const Uint32 KEY_B = SDLK_c;
const Uint32 KEY_C = SDLK_4;
const Uint32 KEY_D = SDLK_r;
const Uint32 KEY_E = SDLK_f;
const Uint32 KEY_F = SDLK_v;
// end SDL2
// Function keys
const Uint32 KEY_DUMP_RAM = SDLK_o;
const Uint32 KEY_DUMP_REGS = SDLK_p;
const Uint32 KEY_PAUSE = SDLK_RETURN;
const Uint32 KEY_RESET = SDLK_BACKSPACE;
const Uint32 KEY_SCALE_1 = SDLK_8;
const Uint32 KEY_SCALE_2 = SDLK_9;
const Uint32 KEY_EXIT = SDLK_ESCAPE;

Uint32 palette[NCOLORS];

/* // SDL2
SDL_Window *window = nullptr;
*/ // end SDL2
SDL_Surface *screen = nullptr;
Mix_Music *beep_sound = nullptr;

Chip8 myChip8;
bool quit = false;
bool paused = false;
Uint32 last_time;
Uint32 elapsed_time;
SDL_Event e;
Uint32 start_time;
float cycles = 0;

int init_SDL() 
{
    std::cout << "Initializing SDL..." << std::endl;

    //Initialize all SDL subsystems
    if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
    {
	std::cout << SDL_GetError() << std::endl;
	return 1;
    }
/*  // SDL2
    window = SDL_CreateWindow("Chip8 Emulator by Dhole", 100, 100, 
			      width * scale, height * scale, 
			      SDL_WINDOW_SHOWN);
    if (window == nullptr)
    {
	std::cout << SDL_GetError() << std::endl;
	return 1;
    }

    screen = SDL_GetWindowSurface(window);
    // end SDL2
*/
    // SDL1.2
    screen = SDL_SetVideoMode(width * scale, height * scale, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (screen == nullptr)
    {
	std::cout << SDL_GetError() << std::endl;
	return 1;
    }
    SDL_WM_SetCaption("Chip8 Emulator by Dhole", NULL);
    SDL_EnableKeyRepeat(0, 0);
    // end SDL1.2
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
    // SDL1.2
    SDL_FreeSurface(screen);
    // end SDL1.2
/*  // SDL2
    SDL_DestroyWindow(window);
*/  // end SDL2
    SDL_Quit();
}

void change_scale(unsigned int s)
{
    scale = s;
/*  // SDL2
    SDL_SetWindowSize(window, width * scale, height * scale);
    screen = SDL_GetWindowSurface(window);
*/  // end SDL2
    // SDL1.2
    screen = SDL_SetVideoMode(width * scale, height * scale, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    // end SDL1.2
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
/*  // SDL2  
    SDL_UpdateWindowSurface(window);
*/  // end SDL2

    // SDL1.2
    SDL_Flip(screen);
    // end SDL1.2
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
/*  // SDL2
    const Uint8 *keystates = SDL_GetKeyboardState(NULL);
*/  // end SDL2
    // SDL1.2
#ifdef EMSCRIPTEN
    const Uint8 *keystates = SDL_GetKeyboardState(NULL);
#else
    const Uint8 *keystates = SDL_GetKeyState(NULL);
#endif
    // end SDL1.2
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
/*  // SDL2 
    if (!e->key.repeat)
    {
*/  // end SDL2
    

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
		paused = paused ^ true;
		break;
	    case KEY_SCALE_1:
		if (scale != 8)
		    change_scale(8);
		break;
	    case KEY_SCALE_2:
		if (scale != 16)
		    change_scale(16);
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
/*  // SDL2 
    }
*/  // end SDL2

    return 0;
}

void main_loop()
{
    start_time = SDL_GetTicks();
    elapsed_time = start_time - last_time;
    last_time = start_time;
		
    while (SDL_PollEvent(&e))
    {
	if (process_event(&e, &myChip8))
	    quit = true;
    }

    if (!paused)
    {
	myChip8.emulate_hardware();

	cycles += (float)elapsed_time * freq / 1000;
	while (cycles > 1)
	    cycles -= myChip8.run_instruction();

	play_audio(myChip8.sound_timer);
    }
    render_SDL(myChip8.gfx);
}
#ifdef EMSCRIPTEN
void print_games()
{
    std::cout << "INVADERS" << std::endl;
}
#endif
int main(int argc, char** argv)
{
    std::string rom_path;
#ifdef EMSCRIPTEN
    //std::cout << "Select game: ";
    //std::cin >> rom_path;
    //if (rom_path == "")
//	return 1;
    rom_path = "INVADERS";
#else
    if (argc < 2)
    {
	std::cout << "Usage: " << argv[0] << " ROM" << std::endl;
	return 1;
    }
    rom_path = argv[1];
#endif
    
    if (setup_graphics())
	return 1;
       
    std::cout << "Initializing Chip8..." << std::endl;
    if (myChip8.initialize(SDL_GetTicks(), rom_path))
	return 1;
    
    last_time = SDL_GetTicks();

#ifdef EMSCRIPTEN
    emscripten_set_main_loop(main_loop, 60, 1);
#else
    while (!quit)
    {
	main_loop();
	// Limit frame rate
	if (SDL_GetTicks() - start_time < minframetime)
	    SDL_Delay(minframetime - (SDL_GetTicks() - start_time));
    }
#endif

    stop_SDL();

    return 0;
}
