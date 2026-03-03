#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// Configuration - experiment with different values !
#define NUM_ACCOUNTS 2
#define INITIAL_BALANCE 1000.0

// Updated Account structure with mutex ( GIVEN )
typedef struct
{
    int account_id;
    double balance;
    int transaction_count;
    pthread_mutex_t lock;
} Account;

Account accounts[NUM_ACCOUNTS];

//Deadlock detection
pthread_mutex_t progress_lock = PTHREAD_MUTEX_INITIALIZER;
time_t last_progress_time = 0;

void initialize_accounts()
{
    for (int i = 0; i < NUM_ACCOUNTS; i++)
    {
        accounts[i].account_id = i;
        accounts[i].balance = INITIAL_BALANCE;
        accounts[i].transaction_count = 0;
        pthread_mutex_init(&accounts[i].lock, NULL);
    }

    pthread_mutex_lock(&progress_lock);
    last_progress_time = time(NULL);
    pthread_mutex_unlock(&progress_lock);
}

// GIVEN : Conceptual example showing HOW deadlock occurs
void transfer_deadlock_example(int from_id, int to_id, double amount)
{
    // This code WILL cause deadlock !
    // Lock source account
    pthread_mutex_lock(&accounts[from_id].lock);
    printf(" Thread % ld : Locked account % d \n ", pthread_self(), from_id);
    // Simulate processing delay
    usleep(100);
    // Try to lock destination account
    printf(" Thread % ld : Waiting for account % d \n ", pthread_self(), to_id);
    pthread_mutex_lock(&accounts[to_id].lock); // DEADLOCK HERE !
    // Transfer ( never reached if deadlocked )
    accounts[from_id].balance -= amount;
    accounts[to_id].balance += amount;
    pthread_mutex_unlock(&accounts[to_id].lock);
    pthread_mutex_unlock(&accounts[from_id].lock);
}
// TODO 1: Implement complete transfer function
// Use the example above as reference
// Add balance checking ( sufficient funds ?)
// Add error handling
// TODO 2: Create threads that will deadlock
// Thread 1: transfer (0 , 1 , amount ) // Locks 0 , wants 1
// Thread 2: transfer (1 , 0 , amount ) // Locks 1 , wants 0
// Result : Circular wait !
// TODO 3: Implement deadlock detection
// Add timeout counter in main ()
// If no progress for 5 seconds , report suspected deadlock
// Reference : time ( NULL ) for simple timing
// TODO 4: Document the Coffman conditions
// In your report , identify WHERE each condition occurs
// Create resource allocation graph showing circular wait