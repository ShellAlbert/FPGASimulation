/*
 * ASK (Amplitude Shift Keying) modulator
 *   message : 50 kHz sine
 *   carrier : 2 MHz cosine
 *   Fs      : 10 MHz
 *
 * s[n] = m[n] * cos(2*pi*fc*n/Fs)
 * m[n] = Ac * (1 + sin(2*pi*fm*n/Fs)) / 2   envelope in [0, Ac]
 *
 * Build: gcc ask_mod.c -lm -o ask_mod
 * Run  : ./ask_mod
 */
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define FM      50000.0       /* 50 kHz message */
#define FC      2000000.0     /* 2 MHz carrier */
#define FS      10000000.0    /* 10 MHz sample rate */
#define AMP     10.0          /* peak carrier amplitude */
#define N       4096

static double message_sin(int n)
{
    double nTime = (double)n /1000000.0;
    //sin(2*pi*w*t), range: (-1,+1)
    //sin(0~360), sin(360)=0, sin(720)=sin(360+360)=0.
    //double sinValue=sin(2*M_PI*FM * nTime);
    double sinValue=sin(FM * nTime);
    double sinAMP=AMP * sinValue;
    printf("num=%d,SampleFreq=%f,tick=%.8f,sin(%.8f)=%.8f, * %.f=%.8f,\n", ///<
    n,FS,nTime,(2.0*M_PI*FM*nTime),sinValue,AMP,sinAMP);
    return sinAMP;
}

/* Non-negative envelope from 50 kHz sine: range [0, AMP] */
static double ask_envelope(int n)
{
    double t = (double)n / FS;
    //return AMP * (1.0 + sin(2.0 * M_PI * FM * t)) / 2.0;
    return AMP * sin(2.0 * M_PI * FM * t) + AMP;
}

/* ASK modulated RF: envelope x carrier */
static double ask_modulate(int n)
{
    double t = (double)n / FS;
    double env = ask_envelope(n);
    //return env * cos(2.0 * M_PI * FC * t);
    return env * sin(2.0 * M_PI * FC * t);
}



static double carrier_cos(int n)
{
    double t = (double)n / FS;
    return AMP * cos(2.0 * M_PI * FC * t);
}

int main(void)
{
    FILE *f_msg = fopen("sin_50kHz.csv", "w");
    FILE *f_env = fopen("ask_envelope.csv", "w");
    FILE *f_car = fopen("sin_2MHz.csv", "w");
    FILE *f_mod = fopen("ask_modulated.csv", "w");
    if (!f_msg || !f_env || !f_car || !f_mod) {
        perror("fopen");
        return 1;
    }

    for (int n = 0; n < N; n++) {
        double msg = message_sin(n);
        double env = ask_envelope(n);
        double car = carrier_cos(n);
        double mod = ask_modulate(n);

        fprintf(f_msg, "%d %.6f\n", n, msg);
        fprintf(f_env, "%d %.6f\n", n, env + 20.0);
        fprintf(f_car, "%d %.6f\n", n, car + 40.0);
        fprintf(f_mod, "%d %.6f\n", n, mod + 60.0);

        if (n < 8)
            printf("%4d  msg=%.3f  env=%.3f  mod=%.3f\n", n, msg, env, mod);
    }

    fclose(f_msg);
    fclose(f_env);
    fclose(f_car);
    fclose(f_mod);
    printf("Wrote sin_50kHz.csv, ask_envelope.csv, sin_2MHz.csv, ask_modulated.csv\n");
    return 0;
}
