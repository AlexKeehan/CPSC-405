#include <stdio.h>
#include <pthread.h>

void *thread_function(void *arg) {
    printf("Thread is attempting to join itself.\n");

    // Attempting to join the thread to itself (undefined behavior)
    if (pthread_join(pthread_self(), NULL) != 0) {
        fprintf(stderr, "Error joining thread to itself\n");
    }

    printf("Thread continues after attempting to join itself.\n");

    pthread_exit(NULL);
}

int main() {
    pthread_t thread;

    // Create the thread
    if (pthread_create(&thread, NULL, thread_function, NULL) != 0) {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }

    // Wait for the thread to finish
    if (pthread_join(thread, NULL) != 0) {
        fprintf(stderr, "Error joining thread\n");
        return 1;
    }

    printf("Main thread exiting\n");

    return 0;
}
