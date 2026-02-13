// ============================================s
// buffer.h - Shared definitions
// Name:	    Leonardo Saltysiak
// Class:	   CS 3302/Section W03
// Term:		  Spring 2026
// Instructor:    Prof. Regan
// Assignment:    A2
// IDE Name:	  VSCode
// ============================================
#ifndef BUFFER_H
#define BUFFER_H

// Required includes for both producer and consumer
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <time.h>

// constants for shared memory and semaphores
#define BUFFER_SIZE 10
#define SHM_KEY 0x1234
#define SEM_MUTEX "/sem_mutex"
#define SEM_EMPTY "/sem_empty"
#define SEM_FULL "/sem_full"

// Item stored in buffer
typedef struct {
    int value;
    int producer_id;
} item_t;

// Shared circular buffer
typedef struct {
    item_t buffer[BUFFER_SIZE];
    int head;   // Next write position
    int tail;   // Next read position
    int count;  // current number of items
} shared_buffer_t;

#endif
