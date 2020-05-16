# Project 2: Bounded-timestamp register locks

## Algorithms

* Szymanski’s solution (Boleslaw K. Szymanski: A simplesolution to Lamport’s concurrent programming problem with linear wait. ICS 1988: 621-626)
* Jayanti et al.’s solution (Prasad Jayanti, King Tan, Gregory Friedland, Amir Katz: Bounding Lamport’s Bakery Algorithm. SOFSEM 2001: 261-270)
* Aravind’s solution (Alex A. Aravind: Yet Another Simple Solution for the Concurrent Programming Control Problem. IEEE Trans. Parallel Distrib. Syst. 22(6): 1056-1063, 2011)
* Black-white Bakery aka "Taubenfeld" (Gadi Taubenfeld: The Black-White, Bakery Algorithm and Related Bounded-Space, Adaptive, Local-Spinning and FIFO Algorithms. DISC 2004: 56-70)
* Lamport’s Bakery (lecture version or original)

## Tasks

* Implement Taubenfeld, Lamport, and two out of the other three; verify (make plausible) with performance counters
and asertions that time stamps are within bounds.

* Which is better? For baseline performance, compare to the
following locks: pthreads or native C11 locks, simple test-and-set
lock, simple test-and-test-and-set lock

* Challenge: Memory behavior. Ensure that memory (register)
updates become visible in required order!