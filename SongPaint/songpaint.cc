//-------------------------------------------------------------------
// SongPaint v 1.0
// Copyright Mikola Lysenko 2008
//   Feel free to redistribute and reuse portions of this code, as long as it is correctly attributed.
// Email: mikolalysenko@gmail.com
// Homepage: http://0fps.wordpress.com
//
// Voice recording songpaint demo.  Uses FFTW to determine the pitch of your voice and paint a cursor.
//
// Requires:
//  OpenGL, OpenAL, FFTW
//
// Compile (Linux):
//  g++ songpaint.cc -O -lglut -lalut -lfftw
//
// Compile (Mac OS X):
//	g++ songpaint.cc -O -lfftw -framework OpenGL -framework GLUT -framework OpenAL -framework Foundation
//
// Compile (Windows):
//   Not sure, but should be possible.
//
//-------------------------------------------------------------------


//Standard includes
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <time.h>

//FFTW
#include <fftw.h>

#ifndef __APPLE__

//OpenGL
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

//OpenAL
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

#else

//Apple has a broken directory structure, needs to be handled separately

//OpenGL
#include <GLUT/glut.h>

//OpenAL
#include <OpenAL/al.h>
#include <OpenAL/alc.h>

//For some reason, alut.h doesn't seem to load correctly on OS X.
extern "C" {
#pragma export on
ALvoid alutInit(ALint *argc,ALbyte **argv);
ALvoid alutExit(ALvoid);
#pragma export off
}

#endif



//Microphone sampling frequency
#define SAMPLE_FREQUENCY    44100

//Window size and number of samples
#define SAMPLES             1024

//Sets the movement rate for the brush
#define VEL                 0.2
#define ANG_VEL             0.5

//Number of history samples
#define HISTORY             20

//Amount of time for short sample (filters out noisy dots)
#define MIN_DURATION        0.01

//Delay time for a beat
#define BEAT_DELAY          0.2
#define BEAT_THRESHOLD      10

//Threshold for volume
#define VOLUME_THRESHOLD    2.0


//FFTW stuff
fftw_plan       plan;
fftw_complex    *fft_input,
                *fft_output;

//OpenAL stuff
static ALCdevice *mic = NULL;
short mic_buffer[SAMPLES];

//Cursor properties
float point_x = 0.0f, point_y = 0.0f, point_angle = 0.0f;
int resized = 1;
float cred = 0.0f, cgreen = 0.0f, cblue = 0.0f;
float tred = 0.0f, tgreen = 0.0f, tblue = 0.0f;

//Screen dimensions
int xRes = 500, yRes = 500;


//Sound properties
float base_frequency = 0.0f;
float duration = 0.0f;
float silence = 0.0f;
float tempo = 1.0f;
bool  has_voice = false;
float voice_frequency = 0.0f;
float voice_volume = 0.0f;
int   prev_freq[HISTORY];
float prev_vol[HISTORY];
float prev_weight[HISTORY];

//The sampling window function
fftw_real window_func(int n)
{
    //Hann window:
    return (fftw_real)sin(M_PI * (fftw_real)n / (SAMPLES - 1));
}

void update_signal()
{
    //Update song
    int samples = 0;
    alcGetIntegerv(mic, ALC_CAPTURE_SAMPLES, sizeof (samples), &samples);
    
    //Change in frequency
    float delta_freq = 0.0, delta_time = (float)samples / (float)SAMPLE_FREQUENCY;
    
    //Read samples into circular buffer
    if(samples > 0)
    {
        //Check for overflow
        if(samples > SAMPLES)
        {
            printf("OVERFLOW\n");
            samples = SAMPLES;
        }
        
        //Poll device
        static short tmp_buffer[SAMPLES];
        alcCaptureSamples(mic, tmp_buffer, samples);
        
        //Append samples to mic buffer
        memcpy(mic_buffer, &mic_buffer[samples], (SAMPLES - samples) * sizeof(short));
        memcpy(&mic_buffer[SAMPLES-samples], tmp_buffer, samples * sizeof(short));
        
        //Prepare input buffer
        for(int i=0; i<SAMPLES; i++)
        {
            fft_input[i].re = (fftw_real)mic_buffer[i] / 65536.0 * window_func(i);
            fft_input[i].im = 0.0;
        }
        
        //Execute FFT to get distribution
        fftw_one(plan, fft_input, fft_output);
        
        //Compute instantaneous frequency and volume from distribution
        int max_freq = 0;
        fftw_real max_vol = 0;
        
        for(int i=0; i<SAMPLES; i++)
        {
            fftw_real v = sqrtf((fft_output[i].re * fft_output[i].re + 
                                 fft_output[i].im * fft_output[i].im) * SAMPLES);
            
            if(v > max_vol)
            {
                max_freq = i;
                max_vol = v;
            }
        }
        
        //Update history buffer
        for(int i=0; i<HISTORY-1; i++)
        {
            prev_freq[i]   = prev_freq[i+1];
            prev_vol[i]    = prev_vol[i+1];
            prev_weight[i] = prev_weight[i+1];
        }
        prev_freq[HISTORY-1]   = max_freq;
        prev_vol[HISTORY-1]    = max_vol;
        prev_weight[HISTORY-1] = samples;
        
        //Compute best weight
        float best_weight = 0;
        
        for(int i=HISTORY-1; i>=0; i--)
        {
            float weight = 0.0, vol = 0.0;
            
            for(int j=0; j<HISTORY; j++)
            if(prev_freq[i] == prev_freq[j])
            {
                weight  += prev_weight[j];
                vol     += prev_vol[j] * prev_weight[j];
            }
            
            if(weight > best_weight)
            {
                best_weight = weight;
                voice_frequency = prev_freq[i];
                voice_volume = log(vol / weight) + 1.0;
            }
        }
        
        printf("Base: %f, Freq: %f, Vol: %f, DeltaT: %f\n", base_frequency, voice_frequency, voice_volume, delta_time);
        
        
        //Check for volume
        if(voice_volume <= VOLUME_THRESHOLD)
        {
            silence += delta_time;
            duration = 0.0;
            
            //Drop signal if silence is too large
            if(silence > BEAT_DELAY)
                has_voice = false;
        }
        else
        {
            static float prev_frequency = 0.;
            
            if(!has_voice)
            {
                base_frequency = voice_frequency;
                prev_frequency = voice_frequency;
                duration = 0.;
                tred = drand48();
                tgreen = drand48();
                tblue = drand48();
            }
            
            if(abs(voice_frequency - prev_frequency) >= BEAT_THRESHOLD)
            {
                duration = 0.;
                tred = rand()%2;
                tgreen = rand()%2;
                tblue = rand()%2;
            }
            
            duration += delta_time;
            silence = 0.0;
            if(duration > MIN_DURATION)
                has_voice = true;
            
            tempo = 1. +  3.5 / (1. + duration);
        }
    }
}


// Display call back from GLUT
void display()
{
    //Compute new time delta
    static float prev_time = 0.0f;
    float cur_time = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    float delta_time = cur_time - prev_time;
    prev_time = cur_time;
    
    //Update the voice tracking
    update_signal();
    
    //Clear out color and depth buffers
    if(resized)
    {
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        resized = 0;
    }
    else
        glClear(GL_DEPTH_BUFFER_BIT);
    
    //Set up view matrices
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0f, 1.0f, -1.0f, 1.0f);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    //Turn on point smoothing
    glEnable(GL_POINT_SMOOTH);
    
    //Draw stuff
    if(has_voice)
    {
        //Compute relative frequency
        fftw_real relative_frequency = voice_frequency - base_frequency;

        //Move brush
        point_angle += relative_frequency * ANG_VEL * delta_time;
        point_x += cos(point_angle) * tempo * VEL * delta_time;
        point_y += sin(point_angle) * tempo * VEL * delta_time;
        
        if(point_x < -1.)
        {
            point_x = -1.;
            point_angle = drand48() * M_PI * 2.;
        }
        else if(point_x > 1.)
        {
            point_x = 1.;
            point_angle = drand48() * M_PI * 2.;
        }
        
        if(point_y < -1.)
        {
            point_y = -1.;
            point_angle = drand48() * M_PI * 2.;
        }
        else if(point_y > 1.)
        {
            point_y = 1.;
            point_angle = drand48() * M_PI * 2.;
        }
        
        //Update color interpolation
        cred   += (tred - cred) * delta_time;
        cgreen += (tgreen - cgreen) * delta_time;
        cblue  += (tblue - cblue) * delta_time;
        
        //Draw brush
        glPointSize(voice_volume * 1.2);
        glBegin(GL_POINTS);
            glColor3f(cred, cgreen, cblue);
            glVertex2f(point_x, point_y);
        glEnd();
    }
    else
    {
        //Randomize brush location
        point_angle = drand48() * M_PI * 2.0;
        point_x = drand48() - 0.5;
        point_y = drand48() - 0.5;        
        cred = drand48();
        cgreen = drand48();
        cblue = drand48();
    }

    //Copy result to screen
    glFlush();
    glutSwapBuffers();
    glutPostRedisplay();
}

//Handle key presses
void keyboard(unsigned char key, int x, int y)
{
    switch(key)
    {
        case 27:
            alutExit();
            exit(1);
        break;

        
        case ' ':
            resized = true;
        break;

        default: break;
    }
}

//Resize window
void resize(int w, int h)
{
    resized = 1;
    xRes = w;
    yRes = h;
    glViewport(0, 0, w, h);
}

//Program entry point
int main(int argc, char** argv)
{
    //Clear out sample history
    for(int i=0; i<HISTORY; i++)
    {
        prev_freq[i] = 0;
        prev_vol[i] = 0.0;
        prev_weight[i] = 0.0;
    }
    
    //Randomize timer
    srand(time(NULL));
    
    //Set colors
    cred = drand48();
    cgreen = drand48();
    cblue = drand48();
    
    //Initialize FFTW
    plan = fftw_create_plan(SAMPLES, FFTW_FORWARD, FFTW_ESTIMATE);    
    fft_input = (fftw_complex*)fftw_malloc(SAMPLES * sizeof(fftw_complex));
    fft_output = (fftw_complex*)fftw_malloc(SAMPLES * sizeof(fftw_complex));

    //Initialize ALUT
    alutInit(&argc, argv);
    
    //Create microphone
    mic = alcCaptureOpenDevice(
        NULL, 
//        "Internal microphone",        //Uncomment to force microphone choice on MacBook Pro
        SAMPLE_FREQUENCY, 
        AL_FORMAT_MONO16, 
        sizeof(short) * SAMPLES);
    
    //Failed to create microphone
    if (mic == NULL)
    {
        fprintf(stderr, "Couldn't find microphone.\n");
        alutExit();
        return -1;
    }
    
    //Initialize glut
	glutInit(&argc, argv);

    //Create display stuff
    glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
    glutInitWindowSize(xRes, yRes);
	glutCreateWindow("SongPaint");

    
    //Set glut calbacks
    glutDisplayFunc(&display);
    glutKeyboardFunc(&keyboard);
    glutReshapeFunc(&resize);
    
    //Initialize program
    alcCaptureStart(mic);
	glutMainLoop();
    alcCaptureStop(mic);
    
    //Close alut
    alcCaptureCloseDevice(mic);
    alutExit();
}
