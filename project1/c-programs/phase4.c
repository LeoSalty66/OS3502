#define _POSIX_C_SOURCE 199309L
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
// Configuration - experiment with different values !
#define NUM_ACCOUNTS 2
#define NUM_THREADS 4
#define TRANSACTIONS_PER_THREAD 10
#define INITIAL_BALANCE 1000.0

// Updated Account structure with mutex ( GIVEN )
typedef struct
{
    int account_id;
    double balance;
    int transaction_count;
    pthread_mutex_t lock; 
} Account;

Account accounts [ NUM_ACCOUNTS ];

// Global variables and tracking mutex as per option 2 (tracking withdrawalas and deposits)
double total_deposits = 0.0;
double total_withdrawals = 0.0;
pthread_mutex_t tracking_lock = PTHREAD_MUTEX_INITIALIZER;

void cleanup_mutexes(); //fix implicit definition error

// GIVEN : Example of mutex initialization
void initialize_accounts()
{
    for (int i = 0; i < NUM_ACCOUNTS; i++)
    {
        accounts[i].account_id = i;
        accounts[i].balance = INITIAL_BALANCE;
        accounts[i].transaction_count = 0;
        // Initialize the mutex
        pthread_mutex_init(&accounts[i].lock, NULL);
    }
}

// GIVEN : Example deposit function WITH proper protection
void deposit_safe(int account_id, double amount)
{
    // Acquire lock BEFORE accessing shared data
    pthread_mutex_lock(&accounts[account_id].lock);

    // ===== CRITICAL SECTION =====
    accounts[account_id].balance += amount;
    accounts[account_id].transaction_count++;
    // ============================

    pthread_mutex_unlock(&accounts[account_id].lock);

    //use tracking
    pthread_mutex_lock(&tracking_lock);
    total_deposits += amount;
    pthread_mutex_unlock(&tracking_lock);
}

// TODO 1: Implement withdrawal_safe () with mutex protection
// Reference : Follow the pattern of deposit_safe () above
// Remember : lock BEFORE accessing data , unlock AFTER
void withdrawal_safe(int account_id, double amount)
{
    // Acquire lock BEFORE accessing shared data
    pthread_mutex_lock(&accounts[account_id].lock);

    // ===== CRITICAL SECTION =====
    accounts[account_id].balance -= amount;
    accounts[account_id].transaction_count++;
    // ============================

    pthread_mutex_unlock(&accounts[account_id].lock);

    //use tracking
    pthread_mutex_lock(&tracking_lock);
    total_withdrawals += amount;
    pthread_mutex_unlock(&tracking_lock);
}

/* STRATEGY 1: Lock Ordering ( RECOMMENDED )
*
* ALGORITHM :
* To prevent circular wait , always acquire locks in consistent order
*
* Step 1: Identify which account ID is lower
* Step 2: Lock lower ID first
* Step 3: Lock higher ID second
* Step 4: Perform transfer
* Step 5: Unlock in reverse order
*
* WHY THIS WORKS :
* - Thread 1: transfer (0 ,1) locks 0 then 1
* - Thread 2: transfer (1 ,0) locks 0 then 1 ( SAME ORDER !)
* - No circular wait possible
*
* WHICH COFFMAN CONDITION DOES THIS BREAK ?
* Answer in your report !
*/
// TODO : Implement safe_transfer_ordered ( from , to , amount )
// Use the algorithm description above
// Hint : int first = ( from < to ) ? from : to ;
int safe_transfer_ordered ( int from , int to , double amount )
{
    if (from < 0 || from >= NUM_ACCOUNTS || to < 0 || to >= NUM_ACCOUNTS)
    {
        return -1;
    }
    if (from == to)
    {
        return 0;
    }
    if (amount <= 0)
    {
        return -1;
    }

    int first = ( from < to ) ? from : to ;
    int second = ( from < to ) ? to : from ;

    pthread_mutex_lock ( &accounts[first].lock ) ;
    pthread_mutex_lock ( &accounts[second].lock ) ;

    int success = 0;

    if (accounts[from].balance < amount)
    {
        success = 0;
    }
    else
    {
        accounts[from].balance -= amount;
        accounts[to].balance += amount;
        accounts[from].transaction_count++;
        accounts[to].transaction_count++;
        success = 1;
    }

    pthread_mutex_unlock ( &accounts[second].lock ) ;
    pthread_mutex_unlock ( &accounts[first].lock ) ;

    return success;
}

// TODO 2: Update teller_thread to use safe functions
// Change : deposit_unsafe -> deposit_safe
// Change : withdrawal_unsafe -> withdrawal_safe
void * teller_thread ( void * arg ) {
    int teller_id = *( int *) arg ; // GIVEN : Extract thread ID

    unsigned int seed = time ( NULL ) ^ pthread_self () ;

    for ( int i = 0; i < TRANSACTIONS_PER_THREAD ; i ++) {

        int from_idx = rand_r ( & seed ) % NUM_ACCOUNTS ;
        int to_idx = rand_r ( & seed ) % NUM_ACCOUNTS ;

        while (to_idx == from_idx) {
            to_idx = rand_r ( & seed ) % NUM_ACCOUNTS ;
        }

        double amount = ( rand_r ( & seed ) % 100 ) + 1 ;

        int ok = safe_transfer_ordered ( from_idx , to_idx , amount ) ;

        if ( ok == 1 ) {
            printf ( " Teller % d : Transferred $ %.2f from Account % d to Account % d \n " ,
                teller_id , amount , from_idx , to_idx ) ;
        } else if ( ok == 0 ) {
            printf ( " Teller % d : Transfer FAILED (insufficient funds) $ %.2f from Account % d to Account % d \n " ,
                teller_id , amount , from_idx , to_idx ) ;
        } else {
            printf ( " Teller % d : Transfer ERROR $ %.2f from Account % d to Account % d \n " ,
                teller_id , amount , from_idx , to_idx ) ;
        }
    }
    return NULL ;
}

// TODO 3: Add performance timing
// Reference : Section 7.2 " Performance Measurement "
// Hint : Use clock_gettime ( CLOCK_MONOTONIC , & start ) ;

// TODO 4: Add mutex cleanup in main ()
// Reference : man pthread_mutex_destroy
// Important : Destroy mutexes AFTER all threads complete !
int main () {

    printf ( "=== Phase 4: Deadlock Prevention (Lock Ordering) Demo ===\n\n" ) ;

    // Initialize accounts wth mutexes
    initialize_accounts();

    // Display initial state
    printf ( "Initial State:\n" ) ;
    for ( int i = 0; i < NUM_ACCOUNTS ; i ++ ) {
        printf ( "Account %d : $%.2f\n" , i , accounts[i].balance ) ;
    }

    double expected_total = NUM_ACCOUNTS * INITIAL_BALANCE ;
    printf ( "\nExpected total : $%.2f\n\n" , expected_total ) ;

    pthread_t threads[NUM_THREADS] ;
    int thread_ids[NUM_THREADS] ;

    struct timespec start , end ;

    // TODO 3: START TIMING
    clock_gettime ( CLOCK_MONOTONIC , & start ) ;

    // Create threads
    for ( int i = 0; i < NUM_THREADS ; i ++ ) {
        thread_ids[i] = i ;
        pthread_create ( &threads[i] , NULL , teller_thread , &thread_ids[i] ) ;
    }

    // Wait for all threads to complete
    for ( int i = 0; i < NUM_THREADS ; i ++ ) {
        pthread_join ( threads[i] , NULL ) ;
    }

    // TODO 3: END TIMING 
    clock_gettime ( CLOCK_MONOTONIC , & end ) ;

    double elapsed = ( end.tv_sec - start.tv_sec ) +
        ( end.tv_nsec - start.tv_nsec ) / 1e9 ;

    printf ( "\nExecution Time : %.6f seconds\n" , elapsed ) ;

    // Display final results
    printf ( "\n=== Final Results ===\n" ) ;
    double actual_total = 0.0 ;

    for ( int i = 0; i < NUM_ACCOUNTS ; i ++ ) {
        printf ( "Account %d : $%.2f (%d transactions)\n" ,
            i , accounts[i].balance , accounts[i].transaction_count ) ;
        actual_total += accounts[i].balance ;
    }

    printf ( "\nExpected total : $%.2f\n" , expected_total ) ;
    printf ( "Actual total : $%.2f\n" , actual_total ) ;
    printf ( "Difference : $%.2f\n" , actual_total - expected_total ) ;

    if ( actual_total != expected_total ) {
        printf ( "\nSynchronization error detected!\n" ) ;
    } else {
        printf ( "\nAll balances consistent. No deadlock, and totals preserved.\n" ) ;
    }

    // TODO 4: CLEANUP MUTEXES
    cleanup_mutexes();

    return 0;
}

void cleanup_mutexes()
{
    for (int i = 0; i < NUM_ACCOUNTS; i++)
    {
        pthread_mutex_destroy(&accounts[i].lock);
    }

    //destroy tracking
    pthread_mutex_destroy(&tracking_lock);
}