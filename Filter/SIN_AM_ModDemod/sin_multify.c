//gcc sin_multify.c -lm
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

#define FREQ_SIGNAL   50000.0  //50KHz.
#define FREQ_CARRIER   1000000.0  //1MHz.
#define AMPLITUDE 5.0  //Amplitude.
#define FS_HZ     10000000.0   /* 10 MHz sample rate */
#define NUM_SAMPLES 1024


// 当你将一个50kHz信号（sin_50kHz.csv）与一个1MHz载波信号（sin_1MHz.csv）相乘时，会生成类似调幅（AM）的信号。
// 在调幅过程中，调制信号的频率成分由载波频率与调制信号频率的和频、差频以及载波频率本身构成。
// 数学上，若载波频率为fc，调制信号频率为fm，则调幅信号的频率成分分别为fc+fm、fc-fm和fc。
// 在你的案例中，fc=1MHz，fm=50kHz，因此频率成分分别为1MHz+50kHz、1MHz-50kHz和1MHz。

#include <stdio.h>
#include <stdlib.h>

// 滑动平均滤波器结构体定义 Moving Average Filter.
typedef struct {
    double *buffer;      // 保存窗口内的数据
    int size;         // 窗口长度
    int index;        // 当前写入位置
    double sum;         // 当前总和
    int count;        // 已存储的数据个数，用于判断是否填满
} MAFilter;

//Moving Average Filter.
MAFilter* MAFilter_init(int window_size) {
    MAFilter *filter = (MAFilter*)malloc(sizeof(MAFilter));
    if (filter == NULL) {
        return NULL;
    }
    filter->buffer = (double*)malloc(window_size * sizeof(double));
    if (filter->buffer == NULL) {
        free(filter);
        return NULL;
    }
    filter->size = window_size;
    filter->index = 0;
    filter->sum = 0;
    filter->count = 0;
    return filter;
}

//Moving Average Filter, remove an old value and add a new value.
void MAFilter_put(MAFilter *filter, double new_value) {
    if (filter->count < filter->size) {
        filter->count++;
    }
    double old_value = filter->buffer[filter->index];
    filter->sum -= old_value;
    filter->buffer[filter->index] = new_value;
    filter->sum += new_value;
    filter->index++;
    if (filter->index == filter->size) {
        filter->index = 0;
    }
}
//Output the average value of all data in the buffer.
double MAFilter_get(MAFilter *filter) {
    if (filter->count < filter->size) {
        return filter->sum / filter->count;
    }
    return filter->sum / filter->size;
}

void MAFilter_free(MAFilter *filter) {
    free(filter->buffer);
    free(filter);
}


int main(void)
{
    int i;

    MAFilter *filter = MAFilter_init(32);
    if (filter == NULL) {
        return 1;
    }


    FILE *fp = fopen("sin_50kHz.csv", "w");
    FILE *fp2 = fopen("sin_1MHz.csv", "w");
    FILE *fp3 = fopen("sin_modulated.csv", "w");
    FILE *fp4 = fopen("sin_demodulated.csv", "w");
    FILE *fp5 = fopen("sin_lpf.csv", "w");
    if (!fp || !fp2 || !fp3 || !fp4 || !fp5) {
        perror("fopen");
        return -1;
    }

    double *datSignal = malloc(NUM_SAMPLES * sizeof(double));
    double *datCarrier = malloc(NUM_SAMPLES * sizeof(double));
    double *datModulated = malloc(NUM_SAMPLES * sizeof(double));
    double *datDemodulated = malloc(NUM_SAMPLES * sizeof(double));
    double *datLPF = malloc(NUM_SAMPLES * sizeof(double));
    if (!datSignal || !datCarrier || !datModulated || !datDemodulated || !datLPF) {
        printf("malloc() failed\n");
        if(datSignal) free(datSignal);
        if(datCarrier) free(datCarrier);
        if(datModulated) free(datModulated);
        if(datDemodulated) free(datDemodulated);
        if(datLPF) free(datLPF);
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

        datSignal[n] = AMPLITUDE * sin(2.0 * M_PI * FREQ_SIGNAL * t);

        datCarrier[n] = AMPLITUDE * sin(2.0 * M_PI * FREQ_CARRIER * t);

        //Multify 50kHz*1MHz to generate modulated signal.
        datModulated[n] = datSignal[n]*datCarrier[n];

        //为了获得原始的50kHz信号，您可以使用更合适的AM解调技术，如同步解调。
        //在同步解调中，您将接收到的AM信号与本地载波信号相乘，该本地载波信号的频率和相位与用于调制的原始载波相同。
        //然后，经过低通滤波后，您可以恢复原始调制信号，而无需将其频率加倍。
        datDemodulated[n]=datModulated[n]*datCarrier[n];

        //low pass filter.
        MAFilter_put(filter, datDemodulated[n]);
        datLPF[n]=MAFilter_get(filter);
    }
    /* Example: print first 100 samples */
    for (int n = 0; n < NUM_SAMPLES; n++)
    {
        printf("%d: 50kHz:%.2f, 1MHz%.2f, Modulated:%.2f, Demodulated():%.2f, LPF:%.2f\n", n, datSignal[n],datCarrier[n],datModulated[n],datDemodulated[n],datLPF[n]);
        //add 0 to move curve above.
        fprintf(fp, "%d %.2f\n", n, datSignal[n]+0);
        //add +50 to move curve above.
        fprintf(fp2, "%d %.2f\n", n, datCarrier[n]+50);
        //add +100 to move curve above.
        fprintf(fp3, "%d %.2f\n", n, datModulated[n]+100);

        //subtract -50 to move curve below.
        fprintf(fp4, "%d %.2f\n", n, datDemodulated[n]-50);

        //subtract -100 to move curve below.
        //fprintf(fp5, "%d %.2f\n", n, datLPF[n]-100);
        //fprintf(fp5, "%d %.2f\n", n, datLPF[n]);
        fprintf(fp5, "%d %.2f\n", n, datLPF[n]/6);
    }
    fclose(fp);
    fclose(fp2);
    fclose(fp3);
    fclose(fp4);
    fclose(fp5);
    free(datSignal);
    free(datCarrier);
    free(datModulated);
    free(datDemodulated);
    free(datLPF);
    MAFilter_free(filter);
    return 0;
}