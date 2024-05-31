#include "common.h"

void usage(const char* prog_name) {
    printf("Usage: %s <basename> <num_threads>\n", prog_name);
    printf("    Run the flock on specified number of files in specified directory\n");
    printf("    The files are created(if not present), with the name pattern\n");
    printf("        <basename>-1, <basename>-2, ..., as specified by <num_threads>\n");
    printf("    Each file lock is manipulated in a seperate thread.\n");

    printf("\n    Up to %d total threads are supported.\n\n", MAX_THREADS);

}

/**
 * holder thread locks the file and holds the lock, until signaled to unlock.
*/
void* lock_holder_thread(void *d) {
    struct thread_data* data = (struct thread_data*)d;

    // Set thread name here
    pthread_setname_np(pthread_self(), data->name);

    flock(data->fd, LOCK_EX);

    tlog(data->name, "locked fd %d (%s)", data->fd, data->filename);

    pthread_mutex_lock(&data->mutex);
    pthread_cond_wait(&data->cond, &data->mutex);

    flock(data->fd, LOCK_UN);

    tlog(data->name, "unlocked fd %d (%s)", data->fd, data->filename);
    
    return NULL;
}

/**
 * waiter thread tries to immediately lock the file, it will be blocked until
 * the holder thread unlocks the file.
 * waiter thread will unlock the file after a random delay, it doesn't need
 * the mutex and condition variable.
*/
void* lock_waiter_thread(void *d) {
    struct thread_data* data = (struct thread_data*)d;

    // Set thread name here
    pthread_setname_np(pthread_self(), data->name);

    /* man 2 flock, we have to open the file one more time */
    int fd = open(data->filename, O_RDWR);

    if (fd < 0) {
        tlog(data->name, "failed to open file %s", data->filename);
        return NULL;
    }

    tlog(data->name, "opened %s, fd=%d", data->filename, fd);

    tlog(data->name, "trying to lock fd %d (%s)...", fd, data->filename);

    flock(fd, LOCK_EX);

    tlog(data->name, "locked fd %d (%s)", fd, data->filename);


    random_delay(0, 2);

    flock(fd, LOCK_UN);

    tlog(data->name, "unlocked fd %d (%s)", fd, data->filename);

    close(fd);
    tlog(data->name, "closed %d (%s)", fd, data->filename);

    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t locker_threads[MAX_THREADS];
    struct thread_data locker_data[MAX_THREADS];

    pthread_t waiter_threads[MAX_THREADS];
    struct thread_data waiter_data[MAX_THREADS];

    int fd;

    char *basename;
    char filename[256];
    char thread_name[32];
    int num_threads;

    if (argc < 3) {
        usage(argv[0]);
        exit(-1);
    }

    basename = argv[1];
    num_threads = atoi(argv[2]);

    if (num_threads > MAX_THREADS) {
        printf("Too many threads, max: %d\n", MAX_THREADS);
        exit(-1);
    }

    pthread_setname_np(pthread_self(), "main");

    /* initialize thread data */
    for (int i = 0; i < num_threads; i++) {
        snprintf(filename, sizeof(filename), "%s-%d", basename, i + 1);
        fd = open(filename, O_CREAT | O_RDWR, 0644);
        if (fd < 0) {
            perror("open:");
            exit(-2);
        }

        tlog("main", "opened %s, fd=%d", filename, fd);

        snprintf(thread_name, sizeof(thread_name), "locker-%d", i + 1);
        init_thread_data(&locker_data[i], fd, thread_name, filename);
        
        snprintf(thread_name, sizeof(thread_name), "waiter-%d", i + 1);
        init_thread_data(&waiter_data[i], -1, thread_name, filename);

    }

    prompt("Preparation done, now going to start the locker threads");

    /* starts the locker threads to hold file lockers */
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&locker_threads[i], NULL, lock_holder_thread, &locker_data[i]);
    }

    delay(1);
    prompt("Now going to start the waiter threads");    

    /* starts the waiter threads */
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&waiter_threads[i], NULL, lock_waiter_thread, &waiter_data[i]);
    }

    delay(1);
    prompt("Check if the waiter threads are blocked, now going to reproduce the issue");

    /* reproduction: send SIGINT to all waiter threads, and tell all locker threads to unlock */
    for (int i = 0; i < num_threads; i++) {
        pthread_kill(waiter_threads[i], SIGINT);
        pthread_cond_signal(&locker_data[i].cond);
    }

    delay(1);
    prompt("All threads are signaled, now waiting for them to finish...");

    /* wait for all threads to finish */
    for (int i = 0; i < num_threads; i++) {
        pthread_join(locker_threads[i], NULL);
        close(locker_data[i].fd);
        tlog("main", "closed %d (%s)", locker_data[i].fd, locker_data[i].filename);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(waiter_threads[i], NULL);
    }

    printf("\nAll done\n\n");
    return 0;
}