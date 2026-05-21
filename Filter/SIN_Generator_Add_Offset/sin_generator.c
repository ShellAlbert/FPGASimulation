//gcc sin_generator.c -lm
//./a.out
//./plot.sh
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
//Pi.
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define FREQ_HZ   50000.0  //50KHz.
#define AMPLITUDE 5.0  //Amplitude.
#define FS_HZ     1000000.0   /* 1 MHz sample rate */
#define NUM_SAMPLES 1024
int main(void)
{
    FILE *fp = fopen("sin_wave.csv", "w");
    FILE *fp2 = fopen("sin_wave_offset.csv", "w");
    FILE *fp3 = fopen("sin_wave_multify.csv", "w");
    if (!fp || !fp2 || !fp3) {
        perror("fopen");
        return -1;
    }

    double *samples = malloc(NUM_SAMPLES * sizeof(double));
    double *samples2 = malloc(NUM_SAMPLES * sizeof(double));
    double *samples3 = malloc(NUM_SAMPLES * sizeof(double));
    if (!samples || !samples2 || !samples3) {
        printf("malloc() failed\n");
        if(samples) free(samples);
        if(samples2) free(samples2);
        if(samples3) free(samples3);
        return -1;
    }

    //Asin(2πFt) 是描述‌正弦波（Sine Wave）‌的标准数学公式，广泛应用于物理学、工程学、信号处理和声学等领域。
    //它表示一个随时间 t 周期性变化的信号。
    //A (Amplitude，振幅)‌, 在电学中，AA 代表电压或电流的‌峰值‌（Peak Value）
    //F (Frequency，频率)‌, 单位时间（通常为1秒）内完成完整周期性变化的次数。
    //t (Time，时间)‌, 自变量，表示过程进行的时刻。
    //作用‌：随着 tt 的增加，相位角 2πFt2πFt 不断增大，驱动正弦函数值在 −1−1 到 11 之间循环变化。
    //正弦函数 sin⁡(θ)的自然周期是 2π弧度（即 360∘）。
    //当 Ft=1Ft=1（即经过一个周期 TT）时，角度增加 2π2π，函数值回到起点，完成一次循环。

    for (int n = 0; n < NUM_SAMPLES; n++) {
        //t (时间)‌：当前采样点对应的物理时间。 t=n/Fs
        //Fs=1000 Hz，当 n=5时，t=0.005秒。
        double t = (double)n / FS_HZ;
        samples[n] = AMPLITUDE * sin(2.0 * M_PI * FREQ_HZ * t);

        //Add DC offset to remove negative value.
        samples2[n] = AMPLITUDE * sin(2.0 * M_PI * FREQ_HZ * t) + AMPLITUDE;

        //Multify 2 times.
        samples3[n] = (AMPLITUDE * sin(2.0 * M_PI * FREQ_HZ * t))*2;
    }
    /* Example: print first 100 samples */
    for (int n = 0; n < NUM_SAMPLES; n++)
    {
        printf("%d: %.2f, Offset: %.2f, Multify x2:%.2f\n", n, samples[n],samples2[n],samples3[n]);
        fprintf(fp, "%d %.2f\n", n, samples[n]);
        fprintf(fp2, "%d %.2f\n", n, samples2[n]);
        fprintf(fp3, "%d %.2f\n", n, samples3[n]);
    }
    fclose(fp);
    fclose(fp2);
    fclose(fp3);
    free(samples);
    free(samples2);
    free(samples3);
    return 0;
}