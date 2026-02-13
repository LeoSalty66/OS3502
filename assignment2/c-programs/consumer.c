// ============================================
// consumer.c - Consumer process starter
// ============================================
#include "buffer.h"

// g
shared_buffer_t* buffer = NULL;
sem_t* mutex = NULL;
sem_t* empty = NULL;
sem_t* full = NULL;
int shm_id = -1;

void cleanup() {
    // Detach shared memory
    if (buffer != NULL) {
        shmdt(buffer);
    }
    
    // Close semaphores
    if (mutex != SEM_FAILED) sem_close(mutex);
    if (empty != SEM_FAILED) sem_close(empty);
    if (full != SEM_FAILED) sem_close(full);
}

void signal_handler(int sig) {
    printf("\nConsumer: Caught signal %d, cleaning up...\n", sig);
    cleanup();
    exit(0);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <consumer_id> <num_items>\n", argv[0]);
        exit(1);
    }
    
    int consumer_id = atoi(argv[1]);
    int num_items = atoi(argv[2]);
    
    // Set up the signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    srand(time(NULL) + consumer_id * 100);
    
    // Attach to the shared memory 
    shm_id = shmget(SHM_KEY, sizeof(shared_buffer_t), 0666);
    if (shm_id == -1) {
        perror("shmget");
        exit(1);
    }

    buffer = (shared_buffer_t*) shmat(shm_id, NULL, 0);
    if (buffer == (void*) -1) {
        perror("shmat");
        exit(1);
    }
    
    // Open semaphores 
    mutex = sem_open(SEM_MUTEX, 0);
    empty = sem_open(SEM_EMPTY, 0);
    full  = sem_open(SEM_FULL, 0);

    if (mutex == SEM_FAILED || empty == SEM_FAILED || full == SEM_FAILED) {
        perror("sem_open");
        cleanup();
        exit(1);
    }
    
    printf("Consumer %d: Starting to consume %d items\n", consumer_id, num_items);
    
    // Main consumption loop
    for (int i = 0; i < num_items; i++) {

        // wait for an available item
        sem_wait(full);

        // enter critical section
        sem_wait(mutex);

        // remove the item from the buffer
        item_t item = buffer->buffer[buffer->tail];
        buffer->tail = (buffer->tail + 1) % BUFFER_SIZE;
        buffer->count--;

        printf("Consumer %d: Consumed value %d from Producer %d\n",
               consumer_id, item.value, item.producer_id);

        // Exit critical section
        sem_post(mutex);

        // Signal empty 
        sem_post(empty);

        // Simulate the consumption time
        usleep(rand() % 100000);
    }
    printf("Consumer %d: Finished consuming %d items\n", consumer_id, num_items);
    cleanup();
    return 0;
}
