../../emscripten/emcc Chip8.cpp main_em.cpp -std=c++11 --preload-file beep.wav -o chip8.html $(for f in c8games/*; do echo "--preload-file $f"; done)
