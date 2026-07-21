/*
 * ASK / OOK modulator and demodulator
 *   message : 50 kHz sine (signal to restore)
 *   carrier : 1 MHz cosine
 *   Fs      : 10 MHz
 *
 * OOK : s(t) = b(t) * Ac * cos(2*pi*fc*t),  b = 1 if sin>=0 else 0
 * ASK : s(t) = m(t) * Ac * cos(2*pi*fc*t),  m = Ac*(1+sin(wm*t))/2
 *
 * Demod (both): coherent detection s*cos(fc*t), MA-5 removes 2*fc ripple,
 *               scale I channel: sin ~= (4*I/Ac) - 1
 *
 * Build: gcc ask_ook.c -lm -o ask_ook
 * Run  : ./ask_ook
 */
 #define _USE_MATH_DEFINES
 #include <math.h>
 #include <stdio.h>
 #include <string.h>
 
 #ifndef M_PI
 #define M_PI 3.14159265358979323846
 #endif
 
 #define FM      50000.0
 #define FC      1000000.0
 #define FS      10000000.0
 #define AMP     10.0
 #define N       2048
 
 /* After mixing, average out 2*fc (period = 5 samples at Fs=10M, fc=1M) */
 #define LPF_CARRIER_LEN  5
 #define LPF_SMOOTH_LEN   11
 
 typedef struct {
     double buf[64];
     int len;
     int idx;
     int fill;
     double sum;
 } LPF;
 
 static void lpf_init(LPF *f, int len)
 {
     memset(f, 0, sizeof(*f));
     f->len = len;
 }
 
 static double lpf(LPF *f, double x)
 {
     if (f->fill < f->len)
         f->fill++;
     f->sum -= f->buf[f->idx];
     f->buf[f->idx] = x;
     f->sum += x;
     f->idx = (f->idx + 1) % f->len;
     return f->sum / f->fill;
 }
 
 static double message_sin(int n)
 {
     double t = (double)n / FS;
     return AMP * sin(2.0 * M_PI * FM * t);
 }
 
 static double carrier_cos(int n)
 {
     double t = (double)n / FS;
     return AMP * cos(2.0 * M_PI * FC * t);
 }
 
 /* ----- OOK mod: carrier ON/OFF from sign of message ----- */
 static double ook_mod(int n)
 {
     double bit = (message_sin(n) >= 0.0) ? 1.0 : 0.0;
     return bit * carrier_cos(n);
 }
 
 /* ----- ASK mod: amplitude follows non-negative envelope ----- */
 static double ask_envelope(int n)
 {
     double t = (double)n / FS;
     return AMP * (1.0 + sin(2.0 * M_PI * FM * t)) / 2.0;
 }
 
 static double ask_mod(int n)
 {
     return ask_envelope(n) * cos(2.0 * M_PI * FC * (double)n / FS);
 }
 
 typedef struct {
     LPF lpf_carrier;
     LPF lpf_smooth;
 } DemodState;
 
 static void demod_init(DemodState *d)
 {
     lpf_init(&d->lpf_carrier, LPF_CARRIER_LEN);
     lpf_init(&d->lpf_smooth, LPF_SMOOTH_LEN);
 }
 
 /* Coherent demod: I = LPF(s*cos(fc*t)) ~ envelope/2, recover sin from envelope */
 static double coherent_demod(DemodState *d, int n, double s)
 {
     double t = (double)n / FS;
     double i = s * cos(2.0 * M_PI * FC * t);
     i = lpf(&d->lpf_carrier, i);
 
     /* envelope m in [0,AMP] -> sin in [-1,1]: sin = 2*m/AMP - 1 = 4*i/AMP - 1 */
     double recovered = ((4.0 * i / AMP) - 1.0) * AMP;
     return lpf(&d->lpf_smooth, recovered);
 }
 
 int main(void)
 {
     DemodState ook_dem, ask_dem;
     demod_init(&ook_dem);
     demod_init(&ask_dem);
 
     FILE *f_msg   = fopen("sin_message.csv", "w");
     FILE *f_ook_m = fopen("ook_modulated.csv", "w");
     FILE *f_ook_d = fopen("ook_demodulated.csv", "w");
     FILE *f_ask_m = fopen("ask_modulated.csv", "w");
     FILE *f_ask_d = fopen("ask_demodulated.csv", "w");
     if (!f_msg || !f_ook_m || !f_ook_d || !f_ask_m || !f_ask_d) {
         perror("fopen");
         return 1;
     }
 
     for (int n = 0; n < N; n++) {
         double msg   = message_sin(n);
         double ook_m = ook_mod(n);
         double ask_m = ask_mod(n);
         double ook_d = coherent_demod(&ook_dem, n, ook_m);
         double ask_d = coherent_demod(&ask_dem, n, ask_m);
 
         fprintf(f_msg,   "%d %.6f\n", n, msg);
         fprintf(f_ook_m, "%d %.6f\n", n, ook_m + 50.0);
         fprintf(f_ook_d, "%d %.6f\n", n, ook_d - 50.0);
         fprintf(f_ask_m, "%d %.6f\n", n, ask_m + 100.0);
         fprintf(f_ask_d, "%d %.6f\n", n, ask_d - 100.0);
 
         if (n < 10)
             printf("%4d  msg=%.3f  ook_dem=%.3f  ask_dem=%.3f\n", n, msg, ook_d, ask_d);
     }
 
     fclose(f_msg);
     fclose(f_ook_m);
     fclose(f_ook_d);
     fclose(f_ask_m);
     fclose(f_ask_d);
     printf("Wrote sin_message.csv, ook_*.csv, ask_*.csv\n");
     return 0;
 }
 