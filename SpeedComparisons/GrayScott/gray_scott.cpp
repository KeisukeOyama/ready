/*

A port of part of Greg Turk's reaction-diffusion code, from:
http://www.cc.gatech.edu/~turk/reaction_diffusion/reaction_diffusion.html

See README.txt for more details.

*/

// stdlib:
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// local:
#include "defs.h"
#include "display.h"

void init(float a[X][Y],float b[X][Y]);

void compute(float a[X][Y],float b[X][Y],
             float da[X][Y],float db[X][Y],
             float r_a,float r_b,float f,float k,
             float speed,
             bool parameter_space);

int main()
{
    // Here we implement the Gray-Scott model, as described here:
    // http://www.cc.gatech.edu/~turk/bio_sim/hw3.html
    // http://arxiv.org/abs/patt-sol/9304003

    // -- parameters --
    float r_a = 0.082f;
    float r_b = 0.041f;

    // for spots:
    float k = 0.064f;
    float f = 0.035f;
    // for stripes:
    //float k = 0.06f;
    //float f = 0.035f;
    // for long stripes
    //float k = 0.065f;
    //float f = 0.056f;
    // for dots and stripes
    //float k = 0.064f;
    //float f = 0.04f;
    // for spiral waves:
    //float k = 0.0475f;
    //float f = 0.0118f;
    float speed = 1.0f;
    // ----------------
    
    // these arrays store the chemical concentrations:
    float a[X][Y], b[X][Y];
    // these arrays store the rate of change of those chemicals:
    float da[X][Y], db[X][Y];

    // put the initial conditions into each cell
    init(a,b);
    
    clock_t start,end;

    const int N_FRAMES_PER_DISPLAY = 100;
    int iteration = 0;
    while(true) 
    {
        start = clock();

        // compute:
        for(int it=0;it<N_FRAMES_PER_DISPLAY;it++)
        {
            compute(a,b,da,db,r_a,r_b,f,k,speed,false);
            iteration++;
        }

        end = clock();

        char msg[1000];
        sprintf(msg,"GrayScott - %0.2f fps",N_FRAMES_PER_DISPLAY / ((end-start)/(float)CLOCKS_PER_SEC));

        // display:
        if(display(a,a,a,iteration,false,200.0f,2,10,msg)) // did user ask to quit?
            break;
    }
}

// return a random value between lower and upper
float frand(float lower,float upper)
{
    return lower + rand()*(upper-lower)/RAND_MAX;
}

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

void init(float a[X][Y],float b[X][Y])
{
    srand((unsigned int)time(NULL));
    
    // figure the values
    for(int i = 0; i < X; i++) {
        for(int j = 0; j < Y; j++) {
            
            //if(hypot(i%50-25/*-X/2*/,j%50-25/*-Y/2*/)<=frand(2,5))
            if(hypot(i-X/2,(j-Y/2)/1.5)<=frand(2,5)) // start with a uniform field with an approximate circle in the middle
            {
                a[i][j] = 0.0f;
                b[i][j] = 1.0f;
            }
            else {
                a[i][j] = 1;
                b[i][j] = 0;
            }
            //float v = frand(0.0f,1.0f);
            //a[i][j] = v;
            //b[i][j] = 1.0f-v;
            //a[i][j] += frand(-0.01f,0.01f);
            //b[i][j] += frand(-0.01f,0.01f);
        }
    }
}

void compute(float a[X][Y],float b[X][Y],
             float da[X][Y],float db[X][Y],
             float r_a,float r_b,float f,float k,float speed,
             bool parameter_space)
{
    //const bool toroidal = false;

    //int iprev,inext,jprev,jnext;

    // compute change in each cell
    for(int i = 0; i < X; i++) {
        int iprev,inext;
        /*if(toroidal) {
            iprev = (i + X - 1) % X;
            inext = (i + 1) % X;
        }
        else*/ {
            iprev = max(0,i-1);
            inext = min(X-1,i+1);
        }

        for(int j = 0; j < Y; j++) {
            int jprev,jnext;
            /*if(toroidal) {
                jprev = (j + Y - 1) % Y;
                jnext = (j + 1) % Y;
            }
            else*/ {
                jprev = max(0,j-1);
                jnext = min(Y-1,j+1);
            }

            float aval = a[i][j];
            float bval = b[i][j];

            if(parameter_space)	{
                const float kmin=0.045f,kmax=0.07f,fmin=0.00f,fmax=0.14f;
                // set f and k for this location (ignore the provided values of f and k)
                k = kmin + i*(kmax-kmin)/X;
                f = fmin + j*(fmax-fmin)/Y;
            }
 
            // compute the Laplacians of a and b
            float dda = a[i][jprev] + a[i][jnext] + a[iprev][j] + a[inext][j] - 4*aval;
            float ddb = b[i][jprev] + b[i][jnext] + b[iprev][j] + b[inext][j] - 4*bval;

            // compute the new rate of change of a and b
            da[i][j] = r_a * dda - aval*bval*bval + f*(1-aval);
            db[i][j] = r_b * ddb + aval*bval*bval - (f+k)*bval;
        }
    }

    // effect change
    for(int i = 0; i < X; i++) 
    {
        for(int j = 0; j < Y; j++) 
        {
            a[i][j] += speed * da[i][j];
            b[i][j] += speed * db[i][j];
            // kill denormals by adding a teeny tiny something (http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.20.1348&rank=4)
            a[i][j] += 1e-10f;
            b[i][j] += 1e-10f;
        }
    }
}
