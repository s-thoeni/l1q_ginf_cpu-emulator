/*Develop Games
because it's fun
Random Name Generators in C++ »
Beep sound with SDL
Posted Thursday, February 4th, 2010 at 01:15 in Random.

The existence of music and sound effects in a game can make huge difference. One of the simplest sound effects is a beep. By playing several beeps in sequence it’s possible to get quite complicated sound effects. Many of the old games used the PC speaker to produce this kind of sound. There is a beep function for the Windows API that uses the PC speakers but not all new Windows support this function. On other platforms there are other ways to use the PC speakers but it is often complicated and not always possible. If all we want to do is to produce a beep we can instead use the loudspeakers (I have to admit it’s not as nostalgic as the PC speakers). I will now show how to use the cross-platform library SDL to generate beeps.

First we take a look at the Windows Beep function:
*/

BOOL WINAPI Beep(
  __in  DWORD dwFreq,
  __in  DWORD dwDuration
);

/*
The first parameter is in what frequency (Hz) the beep should have. Different frequency will give different sound. The second parameter decides how many milliseconds the beep should last. We want something similar to this!

We start by defining some constants.
*/
const int AMPLITUDE = 28000;
const int FREQUENCY = 44100;
/*
AMPLITUDE is the amplitude of the sound waves. Higher amplitude results in higher sound volume. FREQUENCY is the frequency the sound samples are generated (number of bytes per second).
*/
struct BeepObject
{
    double freq;
    int samplesLeft;
};
/*
BeepObject is just a structure to store one beep. It contains the frequency and the number of samples left to generate before we start play the next beep.
*/
class Beeper
{
private:
    double v;
    std::queue<BeepObject> beeps;
public:
    Beeper();
    ~Beeper();
    void beep(double freq, int duration);
    void generateSamples(Sint16 *stream, int length);
    void wait();
};
/*
Instead of having a lot of stuff laying around in the global scope we put our beep functionality inside a class called Beeper. The Beeper contains a queue of the current and future beeps. It also contains a variable v that is used in the generateSamples function explained later.
*/
Beeper::Beeper()
{
    SDL_AudioSpec desiredSpec;

    desiredSpec.freq = FREQUENCY;
    desiredSpec.format = AUDIO_S16SYS;
    desiredSpec.channels = 1;
    desiredSpec.samples = 2048;
    desiredSpec.callback = audio_callback;
    desiredSpec.userdata = this;

    SDL_AudioSpec obtainedSpec;

    // you might want to look for errors here
    SDL_OpenAudio(&desiredSpec, &obtainedSpec);

    // start play audio
    SDL_PauseAudio(0);
}

Beeper::~Beeper()
{
    SDL_CloseAudio();
}
/*
The Beeper constructor sets up the audio device and the destructor closes it. Things to note are that we use 16bit audio format and we set the callback function to be the function audio_callback which will be explained below. We also send a pointer to the Beeper object to callback function by setting the userdata field to this.
*/
void audio_callback(void *_beeper, Uint8 *_stream, int _length)
{
    Sint16 *stream = (Sint16*) _stream;
    int length = _length / 2;
    Beeper* beeper = (Beeper*) _beeper;

    beeper->generateSamples(stream, length);
}
/*
The callback function doesn’t do any real job by itself, but instead it calls generateSamples on the beeper object to fill the sample stream.
*/
void Beeper::generateSamples(Sint16 *stream, int length)
{
    int i = 0;
    while (i < length) {

        if (beeps.empty()) {
            while (i < length) {
                stream[i] = 0;
                i++;
            }
            return;
        }
        BeepObject& bo = beeps.front();

        int samplesToDo = std::min(i + bo.samplesLeft, length);
        bo.samplesLeft -= samplesToDo - i;

        while (i < samplesToDo) {
            stream[i] = AMPLITUDE * std::sin(v * 2 * M_PI / FREQUENCY);
            i++;
            v += bo.freq;
        }

        if (bo.samplesLeft == 0) {
            beeps.pop();
        }
    }
}
/*
generateSamples fills the sample stream with data. When one beep is finished it starts generate the next beep and so on. If there is no beeps left the stream is filled with zeros (silence). Lets look closer at the following loop:

        while (i < samplesToDo) {
            stream[i] = AMPLITUDE * std::sin(v * 2 * M_PI / FREQUENCY);
            i++;
            v += bo.freq;
        }

As you can see the sound waves are generated by a sine function. It is here the variable v mentioned above is used. v is changed depending on the beep frequency and makes the sine to oscillate and the samples pending from AMPLITUDE to -AMPLITUDE. This behaviour is necesary to create sound waves. We could have used something else than a sine function only it oscillates.
*/
void Beeper::beep(double freq, int duration)
{
    BeepObject bo;
    bo.freq = freq;
    bo.samplesLeft = duration * FREQUENCY / 1000;

    SDL_LockAudio();
    beeps.push(bo);
    SDL_UnlockAudio();
}
/*
The beep function takes the frequency, and the duration in milliseconds and adds a BeepObjects to the queue to be played. Note that we use SDL_LockAudio() and SDL_UnlockAudio() to make sure that the callback thread is not manipulating the beep queue at the same time which can be dangerous.
*/
void Beeper::wait()
{
    int size;
    do {
        SDL_Delay(20);
        SDL_LockAudio();
        size = beeps.size();
        SDL_UnlockAudio();
    } while (size > 0);

}
/*
The wait function is only a convenient help to make the program wait until the all beeps in the queue has been played. It works by checking the size of the beep queue and if it’s not empty sleep 20 ms and check again and so on until the queue is empty.

Now it’s time to test what this can do.
*/
int main(int argc, char *argv[])
{
    // init SDL Audio
    SDL_Init(SDL_INIT_AUDIO); 

    // Here we use the same duration for all beeps
    int duration = 140;

    double D = 293.665;
    double E = 329.628;
    double F = 349.228;
    double G = 391.995;
    double A = 440.000;
    double B = 493.883;
    double c = 554.365;
    double d = 587.330;

    Beeper b;

    b.beep(G, duration);
    b.beep(G, duration);
    b.beep(A, duration);
    b.beep(A, duration);
    b.beep(B, duration);
    b.beep(A, duration);
    b.beep(G, duration);
    b.beep(F, duration);
    b.beep(A, duration);
    b.beep(G, duration);
    b.beep(F, duration);
    b.beep(G, duration);
    b.beep(G, duration);
    b.beep(G, duration);
    b.beep(A, duration);
    b.beep(A, duration);
    b.beep(B, duration);
    b.beep(A, duration);
    b.beep(G, duration);
    b.beep(F, duration);

    b.beep(B, duration);
    b.beep(d, duration);
    b.beep(c, duration);
    b.beep(B, duration);
    b.beep(A, duration);
    b.beep(B, duration);
    b.beep(d, duration);
    b.beep(d, duration);
    b.beep(c, duration);
    b.beep(B, duration);
    b.beep(A, duration);
    b.beep(B, duration);
    b.beep(B, duration);
    b.beep(A, duration);
    b.beep(G, duration);
    b.beep(A, duration);
    b.beep(G, duration);
    b.beep(F, duration);
    b.beep(B, duration);
    b.beep(d, duration);
    b.beep(c, duration);
    b.beep(B, duration);
    b.beep(A, duration);
    b.beep(B, duration);

    b.beep(G, duration);
    b.beep(G, duration);
    b.beep(A, duration);
    b.beep(A, duration);
    b.beep(B, duration);
    b.beep(A, duration);
    b.beep(G, duration);
    b.beep(F, duration);
    b.beep(G, duration);
    b.beep(B, duration);
    b.beep(A, duration);
    b.beep(G, duration);
    b.beep(F, duration);
    b.beep(D, duration);
    b.beep(E, duration);
    b.beep(F, duration);
    b.beep(G, duration);
    b.beep(G, duration);
    b.beep(F, duration);
    b.beep(E, duration);
    b.beep(F, duration);
    b.beep(G, duration);

    // wait for music to end
    b.wait();
}
/*
I’m sure you can compose better music than I can. The example above was created by translating musical notation I found on the web using a notes frequency table. I’m no musician so it’s not perfect but at least it shows what we can do with this.

Note that making the beep function non-blocking is useful to be able to do other things during that time like drawing the to the screen or handling input. But this implementation is far from perfect. It’s not a good idea to have more than one Beeper. Maybe it’s better to make it a singleton. We only allow one sound volume but you might want to be able to have one individual volume for each beep. In that case it’s probably best to add an extra parameter for the amplitude to the beep function and also add an amplitude field to the BeepObject so you can read it from the sampling function. You can only play one beep at one time, for more complicated music it might be necessary to be able to play multiple beeps at one time but this can easily be quite complicated.

Complete Source
Leave a Reply
Click here to cancel reply.

Name

Mail (will not be published)

Website

may the source be with you
Develop Games is powered by WordPress
*/