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
// Add balance checking 
// Add error handling
void transfer_deadlock(int from_id, int to_id, double amount)
{
    // Basic error handling
    if (from_id < 0 || from_id >= NUM_ACCOUNTS || to_id < 0 || to_id >= NUM_ACCOUNTS)
    {
        printf(" Thread %ld : Invalid account id(s) (%d -> %d)\n", pthread_self(), from_id, to_id);
        return;
    }
    if (from_id == to_id)
    {
        printf(" Thread %ld : Ignoring transfer to same account %d\n", pthread_self(), from_id);
        return;
    }
    if (amount <= 0)
    {
        printf(" Thread %ld : Invalid amount %.2f\n", pthread_self(), amount);
        return;
    }

    // Lock source account
    pthread_mutex_lock(&accounts[from_id].lock);
    // Coffman #1 Mutual Exclusion: mutex protects this account
    // Coffman #2 Hold and Wait: thread holds from_id while waiting for to_id
    printf(" Thread %ld : Locked account %d\n", pthread_self(), from_id);

    usleep(100);

    // Try to lock destination account
    printf(" Thread %ld : Waiting for account %d\n", pthread_self(), to_id);
    pthread_mutex_lock(&accounts[to_id].lock); // DEADLOCK HERE 
    // Coffman #3 No Preemption: cannot force unlock, thread blocks
    // Coffman #4 Circular Wait: occurs when another thread holds to_id and waits for from_id

    // ===== CRITICAL SECTION (locked) =====
    if (accounts[from_id].balance < amount)
    {
        printf(" Thread %ld : Insufficient funds in account %d (have %.2f, need %.2f)\n",
               pthread_self(), from_id, accounts[from_id].balance, amount);
    }
    else
    {
        accounts[from_id].balance -= amount;
        accounts[to_id].balance += amount;
        accounts[from_id].transaction_count++;
        accounts[to_id].transaction_count++;

        // Record progress time
        pthread_mutex_lock(&progress_lock);
        last_progress_time = time(NULL);
        pthread_mutex_unlock(&progress_lock);

        printf(" Thread %ld : Transferred $%.2f from %d -> %d\n",
               pthread_self(), amount, from_id, to_id);
    }
    // ================================================

    pthread_mutex_unlock(&accounts[to_id].lock);
    pthread_mutex_unlock(&accounts[from_id].lock);
}

// TODO 2: Create threads that will deadlock
// Thread 1: transfer (0 , 1 , amount ) // Locks 0 , wants 1
// Thread 2: transfer (1 , 0 , amount ) // Locks 1 , wants 0
// Result : Circular wait !
void *deadlock_thread_01(void *arg)
{
    (void)arg;

    usleep(50);

    transfer_deadlock(0, 1, 100.0);
    return NULL;
}

void *deadlock_thread_10(void *arg)
{
    (void)arg;

    usleep(50);

    transfer_deadlock(1, 0, 100.0);
    return NULL;
}

// TODO 3: Implement deadlock detection
// Add timeout counter in main ()
// If no progress for 5 seconds , report suspected deadlock
// Reference : time ( NULL ) for simple timing
int main()
{
    printf("=== Phase 3: Create Deadlock Demo ===\n\n");

    initialize_accounts();

    printf("Initial State:\n");
    for (int i = 0; i < NUM_ACCOUNTS; i++)
    {
        printf("Account %d : $%.2f\n", i, accounts[i].balance);
    }
    printf("\n");

    pthread_t t1, t2;

    pthread_create(&t1, NULL, deadlock_thread_01, NULL);
    pthread_create(&t2, NULL, deadlock_thread_10, NULL);

    //Deadlock detection loop
    // If no progress for 5 seconds, report suspected deadlock and exit
    while (1)
    {
        sleep(1);

        time_t now = time(NULL);
        time_t last;

        pthread_mutex_lock(&progress_lock);
        last = last_progress_time;
        pthread_mutex_unlock(&progress_lock);

        if (now - last >= 5)
        {
            printf("\nDEADLOCK SUSPECTED\n");
            printf("No successful transfers recorded for 5 seconds.\n");
            printf("Threads are likely stuck waiting on each other's locks.\n\n");
            break;
        }
    }

    // Cleanup mutexes 
    for (int i = 0; i < NUM_ACCOUNTS; i++)
    {
        pthread_mutex_destroy(&accounts[i].lock);
    }
    pthread_mutex_destroy(&progress_lock);

    return 0;
}

// TODO 4: Document the Coffman conditions
// In your report , identify WHERE each condition occurs
// Create resource allocation graph showing circular wait