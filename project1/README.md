CS3502 P1 Leo Saltysiak

This project demonstrates severally concurrency concepts using threads in C.

Phase 1 - Race Conditions:
Multiple threads perform deposits and withdrawals on shared accounts without synchronization. This demonstrates race conditions and non-deterministic behavior.

Phase 2 - Mutex:
Mutex locks are added to protect shared account data, which eliminates race conditions. Transfers are tracked to test if there is any deviation from amount of expected money.

Phase 3 - Deadlock:
Two threads perform transfers between accounts while locking resources in opposite order. This intentionally creates a deadlock scenario and demonstrates the 4 Coffman conditions.

Phase 4 - Design Solution:
Deadlock is prevented using the lock ordering strategy. Threads always acquire account locks in a consistent order to break the circular wait condition.

Required Headers:
# include < pthread .h >
# include < stdio .h >
# include < stdlib .h >
# include < unistd .h >
# include < time .h >
# include < errno .h >
# include < string .h >

Compilation:
gcc - Wall - Wextra - pthread phase1 . c -o phase1
gcc - Wall - Wextra - pthread phase2 . c -o phase2
gcc - Wall - Wextra - pthread phase3 . c -o phase3
gcc - Wall - Wextra - pthread phase4 . c -o phase4

