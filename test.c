#include <stdlib.h>
 #include "SDL.h"
 int main(int argc, char *argv[])
 {
    SDL_Surface *screen;
    if (SDL_Init(SDL_INIT_VIDEO) == -1) {
        printf("Can't init SDL:  %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);
    screen = SDL_SetVideoMode(200, 100, 16, SDL_HWSURFACE);
    if (screen == NULL) {
        printf("Can't set video mode: %s\n", SDL_GetError());
        exit(1);
    }   
    SDL_Delay(3000);
    SDL_Quit();
    return 0;
 }
