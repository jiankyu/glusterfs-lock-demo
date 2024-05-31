#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/file.h>
#include <pthread.h>

#define MAX_THREADS 16

/* controlled thread data, flock/unlock on a file in a controlled way */
struct thread_data {
    int fd;
    char name[32];
    char filename[256];
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

void init_thread_data(struct thread_data* data, int fd,
                      const char* thread_name, const char* file_name);

void delay(int seconds);
void random_delay(int min_seconds, int max_seconds);

void prompt(const char* msg);

/* enhanced printf, with timestamp and thread id */
void tlog(const char* thread_name, const char* msg, ...);
