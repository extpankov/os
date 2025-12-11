#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define NUM_READERS 10
#define BUFFER_LENGTH 64

static char global_buffer[BUFFER_LENGTH];
static pthread_mutex_t sync_lock = PTHREAD_MUTEX_INITIALIZER;
static int sequence_number = 0;

void* producer_routine(void* unused) {
    (void)unused;
    while (1) {
        pthread_mutex_lock(&sync_lock);
        snprintf(global_buffer, BUFFER_LENGTH, "Record #%d", sequence_number++);
        pthread_mutex_unlock(&sync_lock);
        usleep(600000);
    }
    return NULL;
}

void* consumer_routine(void* arg) {
    unsigned long tid = (unsigned long)pthread_self();
    (void)arg;
    
    while (1) {
        pthread_mutex_lock(&sync_lock);
        printf("Consumer %lu: %s\n", tid, global_buffer);
        pthread_mutex_unlock(&sync_lock);
        usleep(300000);
    }
    return NULL;
}

int main(void) {
    pthread_t writer;
    pthread_t readers[NUM_READERS];
    
    snprintf(global_buffer, BUFFER_LENGTH, "Initial value");
    
    if (pthread_create(&writer, NULL, producer_routine, NULL) != 0) {
        perror("Failed to create writer thread");
        return EXIT_FAILURE;
    }
    
    for (long i = 0; i < NUM_READERS; ++i) {
        if (pthread_create(&readers[i], NULL, consumer_routine, NULL) != 0) {
            perror("Failed to create reader thread");
            return EXIT_FAILURE;
        }
    }
    
    pthread_join(writer, NULL);
    for (int i = 0; i < NUM_READERS; ++i) {
        pthread_join(readers[i], NULL);
    }
    
    return 0;
}
