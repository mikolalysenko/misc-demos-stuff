//  A simple filtering program using the FFTW

//Standard includes
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <time.h>

//FFTW
#include <fftw.h>

#ifndef __APPLE__

//OpenAL
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

#else

//Apple has a broken directory structure, needs to be handled separately

//OpenAL
#include <OpenAL/al.h>
#include <OpenAL/alc.h>

//For some reason, alut.h doesn't seem to load correctly on OS X.
extern "C" {
ALvoid alutInit(ALint *argc,ALbyte **argv);
ALvoid alutExit(ALvoid);
}

#endif

//Microphone sampling frequency / playback rate
#define SAMPLE_FREQUENCY    44100

//Number of source buffers
#define NUM_BUFFERS         32

//Buffer size parameters
#define KERNEL_SIZE         256

//Size of a sample buffer
#define BUFFER_SIZE         512

//Size of a window
#define WINDOW_SIZE         (2 * KERNEL_SIZE + BUFFER_SIZE)

//Size of the microphone buffer
#define MIC_BUFFER_SIZE     (2 * WINDOW_SIZE)

#define PITCH_SHIFT         0

//FFTW stuff
fftw_plan       fwd_plan, rev_plan;
fftw_complex    *fft_input,
                *fft_output,
                *fft_temp;

//Microphone data
static ALCdevice *mic = NULL;
static short mic_buffer[MIC_BUFFER_SIZE];
static short tmp_mic_buffer[MIC_BUFFER_SIZE];

//RUnning
bool running = true;

void kill_prog(int signum)
{
    running = false;
}

//Complex addition
fftw_complex operator+(fftw_complex a, fftw_complex b)
{
    fftw_complex r;
    r.re = a.re + b.re;
    r.im = a.im + b.im;
    return r;
}

//Complex multiplication
fftw_complex operator*(fftw_complex a, fftw_complex b)
{
    fftw_complex r;
    r.re = a.re * b.re - a.im * b.im;
    r.im = a.re * b.im + a.im * b.re;
    return r;
}


//Compute magnitude of a complex number
fftw_real mag(fftw_complex c)
{
    return c.re * c.re + c.im * c.im;
}

//Window function
fftw_real window(int n)
{
//    return 1.0;
    return (fftw_real)sin(M_PI * (fftw_real)n / (WINDOW_SIZE - 1));
}


//Process a chunk of sound
void process_sound(const short * input, short * output)
{
    //Convert to fourier series
    for(int i=0; i<WINDOW_SIZE; i++)
    {
        fft_input[i].re = (float)input[i];
        fft_input[i].im = 0.0;
        fft_temp[i].re = 
        fft_temp[i].im = 0.0;
    }
    
    //Execute FFT to get distribution
    fftw_one(fwd_plan, fft_input, fft_output);
    
    for(int i=PITCH_SHIFT+1; i<WINDOW_SIZE-1; i++)
    {
        float m[3];
        m[0] = mag(fft_output[i-1]);
        m[1] = mag(fft_output[i]);
        m[2] = mag(fft_output[i+1]);
        
        if(m[1] > m[0] && m[1] > m[2])
        {
            for(int j=0; j<3; j++)
            {
                int s = i + j - 1;
                int d = s - PITCH_SHIFT;
                
                fft_temp[d] = fft_output[s];
            }
        }
    }

    //Convert back to signal
    fftw_one(rev_plan, fft_temp, fft_input);
    
    //Put buffer back into regular format
    for(int i=0; i<BUFFER_SIZE; i++)
    {
        float f = fft_input[KERNEL_SIZE + i].re;
        
        if(f > 32767.0)
            f = 32767;
        else if(f < -32767.0)
            f = -32767.0;
        
        output[i] = f;
    }
}


// Returns number of samples recieved from microphone
int poll_mic()
{
    //Update song
    int samples = 0;
    alcGetIntegerv(mic, ALC_CAPTURE_SAMPLES, sizeof (samples), &samples);
    
    //Read samples into circular buffer
    if(samples > 0)
    {
        if(samples > BUFFER_SIZE)
        {
            samples = BUFFER_SIZE;
        }
        
        //Poll device
        alcCaptureSamples(mic, tmp_mic_buffer, samples);
        
        //Append samples to mic buffer
        memcpy(mic_buffer, &mic_buffer[samples], (MIC_BUFFER_SIZE - samples) * sizeof(short));
        memcpy(&mic_buffer[MIC_BUFFER_SIZE - samples], tmp_mic_buffer, samples * sizeof(short));
    }
    
    return samples;
}



//Program entry point
int main(int argc, char** argv)
{
    signal(SIGINT, kill_prog);
    
    //OpenAL variables for play back
    static short buffer_data[NUM_BUFFERS][BUFFER_SIZE];
    ALuint source, buffers[NUM_BUFFERS];
    int cur_buffer = 0;
    
    //Clear out buffers
    memset(buffer_data[0], 0, sizeof(short) * BUFFER_SIZE);
    memset(mic_buffer, 0, sizeof(short) * MIC_BUFFER_SIZE);
    
    //Randomize timer
    srand(time(NULL));

    //Initialize FFTW
    fwd_plan = fftw_create_plan(WINDOW_SIZE, FFTW_FORWARD, FFTW_ESTIMATE);    
    rev_plan = fftw_create_plan(WINDOW_SIZE, FFTW_BACKWARD, FFTW_ESTIMATE);    
    fft_input = (fftw_complex*)fftw_malloc(WINDOW_SIZE * sizeof(fftw_complex));
    fft_output = (fftw_complex*)fftw_malloc(WINDOW_SIZE * sizeof(fftw_complex));
    fft_temp = (fftw_complex*)fftw_malloc(WINDOW_SIZE * sizeof(fftw_complex));
    
    //Initialize ALUT
    alutInit(&argc, argv);
    
    //Create buffers
    alGenBuffers(NUM_BUFFERS, buffers);
    
    //Create source
    alGenSources(1, &source);
    
    //Set source properties
    alSource3f(source, AL_POSITION,        0.0, 0.0, 0.0);
    alSource3f(source, AL_VELOCITY,        0.0, 0.0, 0.0);
    alSource3f(source, AL_DIRECTION,       0.0, 0.0, 0.0);
    alSourcef (source, AL_ROLLOFF_FACTOR,  0.0          );
    alSourcei (source, AL_SOURCE_RELATIVE, AL_TRUE      );    
        
    //Create microphone
    mic = alcCaptureOpenDevice(
//        NULL,
//        "Internal microphone",
        "Line In", 
        SAMPLE_FREQUENCY, 
        AL_FORMAT_MONO16, 
        sizeof(short) * MIC_BUFFER_SIZE);
    
    //Failed to create microphone
    if (mic == NULL)
    {
        fprintf(stderr, "Couldn't find microphone.\n");
        alutExit();
        return -1;
    }
    
    //Start capture
    alcCaptureStart(mic);
    
    //Fill the mic buffer
    for(int i=0; i<MIC_BUFFER_SIZE; i+=poll_mic()) {}
    
    //Start the capture loop
    for(int samples = 0; running; samples += poll_mic())
    {
        //printf("samples = %d\n", samples);
        
        //Clear out audio buffers
        int processed;
        ALuint tmp_buffers[NUM_BUFFERS];
        alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
        if(processed > 0)
        {
            alSourceUnqueueBuffers(source, processed, tmp_buffers);
        }
        
        for( ; samples > BUFFER_SIZE; samples -= BUFFER_SIZE)
        {
            int w_start = MIC_BUFFER_SIZE - samples - KERNEL_SIZE;
            
            if(w_start < 0)
            {
                printf("Sound buffer overflow, skipping samples\n");
                continue;
            }
            
            //Process sound window
            process_sound(&mic_buffer[w_start], buffer_data[cur_buffer]);
            
            //Send data to speaker
            alBufferData(
                buffers[cur_buffer], 
                AL_FORMAT_MONO16, 
                buffer_data[cur_buffer], 
                BUFFER_SIZE * sizeof(short), 
                SAMPLE_FREQUENCY);
            
            alSourceQueueBuffers(source, 1, &buffers[cur_buffer]);
            
            cur_buffer = (cur_buffer + 1) % NUM_BUFFERS;
            
            //Check if source is playing
            ALenum state;
            alGetSourcei(source, AL_SOURCE_STATE, &state);
            
            if(state != AL_PLAYING)
            {
                alSourcePlay(source);
            }
        }
    }    
    
    printf("Done\n");
    
    //Close alut
    alSourceStop(source);
    alcCaptureStop(mic);
    alcCaptureCloseDevice(mic);
    alutExit();
}
