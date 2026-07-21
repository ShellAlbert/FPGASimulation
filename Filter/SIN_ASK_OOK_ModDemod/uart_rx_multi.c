/*
 * Receive data from three UART devices in parallel using pthreads.
 *   /dev/ttyUSB0 -> uart_usb0.bin
 *   /dev/ttyUSB1 -> uart_usb1.bin
 *   /dev/ttyUSB2 -> uart_usb2.bin
 *
 * Serial: 4 Mbps, 8 data bits, no parity, 1 stop bit (8N1)
 *
 * Build: gcc uart_rx_multi.c -pthread -o uart_rx_multi
 * Run  : ./uart_rx_multi
 * Stop : Ctrl+C
 */
#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <asm/termbits.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#define BAUD_RATE   4000000
#define READ_BUF_SZ 4096

typedef struct {
    const char *device;
    const char *outfile;
    int         index;
} uart_thread_args_t;

static volatile sig_atomic_t g_running = 1;

static void on_sigint(int sig)
{
    (void)sig;
    g_running = 0;
}

static int open_uart(const char *device)
{
    int fd = open(device, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        fprintf(stderr, "open(%s): %s\n", device, strerror(errno));
        return -1;
    }

    struct termios2 tty;
    if (ioctl(fd, TCGETS2, &tty) != 0) {
        fprintf(stderr, "TCGETS2(%s): %s\n", device, strerror(errno));
        close(fd);
        return -1;
    }

    tty.c_cflag &= ~(CSIZE | PARENB | CSTOPB | CRTSCTS);
    tty.c_cflag |= CS8;

    tty.c_cflag &= ~CBAUD;
    tty.c_cflag |= BOTHER;
    tty.c_ispeed = BAUD_RATE;
    tty.c_ospeed = BAUD_RATE;

    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR |
                     IGNCR | ICRNL | IXON | IXOFF | IXANY);
    tty.c_oflag &= ~OPOST;
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);

    tty.c_cc[VMIN]  = 1;
    tty.c_cc[VTIME] = 0;

    if (ioctl(fd, TCSETS2, &tty) != 0) {
        fprintf(stderr, "TCSETS2(%s, %d bps): %s\n",
                device, BAUD_RATE, strerror(errno));
        close(fd);
        return -1;
    }

    return fd;
}

static void *uart_rx_thread(void *arg)
{
    uart_thread_args_t *cfg = arg;
    unsigned char buf[READ_BUF_SZ];
    size_t total = 0;

    int uart_fd = open_uart(cfg->device);
    if (uart_fd < 0)
        return (void *)(intptr_t)-1;

    FILE *out = fopen(cfg->outfile, "wb");
    if (!out) {
        fprintf(stderr, "fopen(%s): %s\n", cfg->outfile, strerror(errno));
        close(uart_fd);
        return (void *)(intptr_t)-1;
    }

    printf("[thread %d] %s -> %s (4 Mbps 8N1)\n",
           cfg->index, cfg->device, cfg->outfile);

    while (g_running) {
        ssize_t n = read(uart_fd, buf, sizeof(buf));
        if (n < 0) {
            if (errno == EINTR)
                continue;
            fprintf(stderr, "read(%s): %s\n", cfg->device, strerror(errno));
            break;
        }
        if (n == 0)
            continue;

        if (fwrite(buf, 1, (size_t)n, out) != (size_t)n) {
            fprintf(stderr, "fwrite(%s): %s\n", cfg->outfile, strerror(errno));
            break;
        }
        fflush(out);
        total += (size_t)n;
    }

    fclose(out);
    close(uart_fd);
    printf("[thread %d] stopped, wrote %zu bytes to %s\n",
           cfg->index, total, cfg->outfile);
    return NULL;
}

int main(void)
{
    static uart_thread_args_t configs[] = {
        { "/dev/ttyUSB0", "uart_usb0.bin", 0 },
        { "/dev/ttyUSB1", "uart_usb1.bin", 1 },
        { "/dev/ttyUSB2", "uart_usb2.bin", 2 },
    };

    pthread_t threads[3];
    int i;

    signal(SIGINT, on_sigint);
    signal(SIGTERM, on_sigint);

    printf("Starting 3 UART receive threads (Ctrl+C to stop)...\n");

    for (i = 0; i < 3; i++) {
        if (pthread_create(&threads[i], NULL, uart_rx_thread, &configs[i]) != 0) {
            fprintf(stderr, "pthread_create(thread %d): %s\n", i, strerror(errno));
            g_running = 0;
            while (--i >= 0)
                pthread_join(threads[i], NULL);
            return 1;
        }
    }

    for (i = 0; i < 3; i++)
        pthread_join(threads[i], NULL);

    printf("All threads finished.\n");
    return 0;
}
