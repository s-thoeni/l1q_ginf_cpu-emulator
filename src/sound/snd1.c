/*

  sample SDL test
  compile with:
  gcc -g -W -Wall -Isrc/include -o snd1 src/sound/snd1.c `sdl-config --cflags --libs`

*/

//#include <iostream>

//#include <cmath>
#include <math.h>

#include <SDL/SDL.h>
//#include "SDL/SDL_main.h"

/* linker options: -lmingw32 -lSDLmain -lSDL -mwindows */

//using namespace std;

unsigned int sampleFrequency = 0;
unsigned int audioBufferSize = 0;
unsigned int outputAudioBufferSize = 0;

unsigned int freq1 = 1000;
unsigned int fase1 = 0;
unsigned int freq2 = 5000;
unsigned int fase2 = 0;

void example_mixaudio(void *unused, Uint8 *stream, int len) {

    unsigned int bytesPerPeriod1 = sampleFrequency / freq1;
    unsigned int bytesPerPeriod2 = sampleFrequency / freq2;

	int i;
	int channel1, channel2, outputValue;
	
    for (i=0;i<len;i++) {
	  channel1 = (int)(150*sin(fase1*6.28/bytesPerPeriod1));
	  channel2 = (int)(150*sin(fase2*6.28/bytesPerPeriod2));

        outputValue = channel1 + channel2;           // just add the channels
        if (outputValue > 127) outputValue = 127;        // and clip the result
        if (outputValue < -128) outputValue = -128;      // this seems a crude method, but works very well

        stream[i] = outputValue;

        fase1++;
        fase1 %= bytesPerPeriod1;
        fase2++;
        fase2 %= bytesPerPeriod2;
    }
}

int main(int argc, char *argv[])
{

    

	 /* Initialize SDL's video and audio subsystems.
       Video is necessary to receive events. */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
	printf("Unable to initialize SDL: %s\n", SDL_GetError());
	return 1;
    }

    /* Make sure SDL_Quit gets called when the program exits. */
    atexit(SDL_Quit);

    /* We also need to call this before we exit. SDL_Quit does
       not properly close the audio device for us. */
    atexit(SDL_CloseAudio);

	
    /* setup audio */
    SDL_AudioSpec *desired, *obtained;

    /* Allocate a desired SDL_AudioSpec */
    desired = (SDL_AudioSpec *) malloc(sizeof(SDL_AudioSpec));

    /* Allocate space for the obtained SDL_AudioSpec */
    obtained = (SDL_AudioSpec *) malloc(sizeof(SDL_AudioSpec));

    /* choose a samplerate and audio-format */
    desired->freq = 44100;
    desired->format = AUDIO_S8;

    /* Large audio buffers reduces risk of dropouts but increases response time.
     *
     * You should always check if you actually GOT the audiobuffer size you wanted,
     * note that not hardware supports all buffer sizes (< 2048 bytes gives problems with some
     * hardware). Older versions of SDL had a bug that caused many configuration to use a
     * buffersize of 11025 bytes, if your sdl.dll is approx. 1 Mb in stead of 220 Kb, download
     * v1.2.8 of SDL or better...)
     */
    desired->samples = 4096;

    /* Our callback function */
    desired->callback=example_mixaudio;
    desired->userdata=NULL;

    desired->channels = 1;
printf("Now opening?\n");
    /* Open the audio device and start playing sound! */
    if ( SDL_OpenAudio(&desired, &obtained) < 0 ) {
	  	printf("what the hell?\n");
	  fprintf(stderr, "AudioMixer, Unable to open audio: %s\n", SDL_GetError());
        exit(1);
    }
	  	printf("so what?\n");
    audioBufferSize = obtained->samples;
    sampleFrequency = obtained->freq;

    /* if the format is 16 bit, two bytes are written for every sample */
    if (obtained->format==AUDIO_U16 || obtained->format==AUDIO_S16) {
        outputAudioBufferSize = 2*audioBufferSize;
    } else {
        outputAudioBufferSize = audioBufferSize;
    }

	printf("where is screen?\n");
    SDL_Surface *screen = SDL_SetVideoMode(200,200, 16, SDL_SWSURFACE);
    SDL_WM_SetCaption("Audio Example",0);

    SDL_PauseAudio(0);

	printf("what happend?\n");
    int running = 0;

    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            /* GLOBAL KEYS / EVENTS */
            switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    running = 1;
                    break;
                default: break;
                }
                break;
            case SDL_QUIT:
                running = 1;
                break;
            }
            SDL_Delay(1);
        }
        SDL_Delay(1);
    }
    SDL_Quit();
    return EXIT_SUCCESS;
}
