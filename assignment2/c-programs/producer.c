// ============================================
// producer.c - Producer process starter
// Name:	    Leonardo Saltysiak
// Class:	   CS 3302/Section W03
// Term:		  Spring 2026
// Instructor:    Prof. Regan
// Assignment:    A2
// IDE Name:	  VSCode
// ============================================
#include "buffer.h"
#include <errno.h>

// g
shared_buffer_t* buffer = NULL;
sem_t* mutex = NULL;
sem_t* empty = NULL;
sem_t* full = NULL;
int shm_id = -1;

void cleanup() {
    if (buffer != NULL) {
        shmdt(buffer);
    }
    
    // Close semaphores 
    if (mutex != SEM_FAILED) sem_close(mutex);
    if (empty != SEM_FAILED) sem_close(empty);
    if (full != SEM_FAILED) sem_close(full);
}

void signal_handler(int sig) {
    printf("\nProducer: Caught signal %d, cleaning up...\n", sig);
    cleanup();
    exit(0);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <producer_id> <num_items>\n", argv[0]);
        exit(1);
    }
    
    int producer_id = atoi(argv[1]);
    int num_items = atoi(argv[2]);
    
    // Sets up signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    srand(time(NULL) + producer_id);
    
    // Attach to shared memory 
    int created = 0;

    shm_id = shmget(SHM_KEY, sizeof(shared_buffer_t), IPC_CREAT | IPC_EXCL | 0666);
    if (shm_id == -1) {
        if (errno == EEXIST) {
            shm_id = shmget(SHM_KEY, sizeof(shared_buffer_t), 0666);
            if (shm_id == -1) {
                    perror("shmget");
            exit(1);
            }
        } else {
            perror("shmget");
            exit(1);
        }
    } else {
        created = 1;
    }

    buffer = (shared_buffer_t*) shmat(shm_id, NULL, 0);
    if (created) {
    buffer->head = 0;
    buffer->tail = 0;
    buffer->count = 0;
    }
    if (buffer == (void*) -1) {
        perror("shmat");
        exit(1);
    }

    // Open semaphores 
    mutex = sem_open(SEM_MUTEX, O_CREAT, 0644, 1);
    empty = sem_open(SEM_EMPTY, O_CREAT, 0644, BUFFER_SIZE);
    full  = sem_open(SEM_FULL,  O_CREAT, 0644, 0);

    if (mutex == SEM_FAILED || empty == SEM_FAILED || full == SEM_FAILED) {
        perror("sem_open");
        cleanup();
        exit(1);
    }

    printf("Producer %d: Starting to produce %d items\n", producer_id, num_items);
    
    //Main production loop
    for (int i = 0; i < num_items; i++) {

        // ceate item
        item_t item;
        item.value = producer_id * 1000 + i;
        item.producer_id = producer_id;

        // Wait for empty slot
        sem_wait(empty);

        // Enter critical section
        sem_wait(mutex);

        // Add item to buffer
        buffer->buffer[buffer->head] = item;
        buffer->head = (buffer->head + 1) % BUFFER_SIZE;
        buffer->count++;

        printf("Producer %d: Produced value %d\n", producer_id, item.value);

        // Exit critical section
        sem_post(mutex);

        // signal that item is available
        sem_post(full);

        // simulate production time
        usleep(rand() % 100000);
    }
    
    printf("Producer %d: Finished producing %d items\n", producer_id, num_items);
    cleanup();
    return 0;
}
