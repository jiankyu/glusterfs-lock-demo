#include "common.h"

void delay(int seconds) {
    sleep(seconds);
}

void random_delay(int min_seconds, int max_seconds) {
    srand(time(NULL));
    int delay = min_seconds + rand() % (max_seconds - min_seconds + 1);
    sleep(delay);
}

void prompt(const char* msg) {
    printf("\n%s\n", msg);
    printf("Press any key to continue...\n");
    getchar();
}

void tlog(const char* thread_name, const char* msg, ...) {
    char buf[256];
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);

    va_list args;
    va_start(args, msg);
    vsnprintf(buf, sizeof(buf), msg, args);
    va_end(args);
    printf("[%d-%02d-%02d %02d:%02d:%02d] [%s] %s\n",
        tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
        tm->tm_hour, tm->tm_min, tm->tm_sec,
        thread_name, buf);
}

void init_thread_data(struct thread_data* data, int fd,
                      const char* thread_name, const char* file_name) {
        data->fd = fd;
        snprintf(data->name, sizeof(data->name), "%s", thread_name);
        snprintf(data->filename, sizeof(data->filename), "%s", file_name);

        pthread_mutex_init(&data->mutex, NULL);
        pthread_cond_init(&data->cond, NULL);
}