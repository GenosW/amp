#include <stdio.h>
#include <omp.h>
#include <math.h>
#include <time.h>
#include <assert.h>

#include "locks.hpp"

void do_some_work(int workload, double randomness){
	// does some work (workload * 3 flops)
	// randomness in [0,1] is the portion of the workload
	// that is randomized 0... full workload, 
	//					0.5... workload between 50-100%
	//					  1... workload between 0-100%

	assert((0<=randomness) && (randomness <= 1));

	double randy = (rand()%1000) / 1000.;
	int rand_wl = int(((1-randomness) + randomness*randy) * workload);

	double dummy = 1.23;

	#ifdef DEBUG
		int id = omp_get_thread_num();
		printf("id = %d, rand_wl = %d\n",id,rand_wl);
	#endif

	for (int i = 0; i < rand_wl; i++){
		dummy = dummy + dummy;
		dummy = dummy + 1e-10;
		dummy = dummy / 2;
	}
}


// all tests should end up in their own module
// ----------------------------------------------
bool test_mutex(Lock* test_lock, int num_threads, 
					int num_turns, int workload, double randomness, 
					bool print_to_console = false)
{
	// this tests the mutual exclusion property of a lock
	// returns true if mutual exclusion held true, false otherwise
	// test_lock ... lock object, to be tested
	// num_threads ... number of threads for the test
	// num_turns ... number of times each thread executes the CS
	// randomness ... number in [0,1] to randomize workload of 
	//					work in non-CS
	// print_to_console ... console output on/off

	omp_set_num_threads(num_threads);

	// counter keeps track of the logging position
	int counter = 0;
	// an event is acquisition of a lock or unlocking
	int num_events = num_threads * num_turns * 2;
	int* event_log = new int[num_events];

	//initialize logging array
	for (int i = 0; i < num_events; i++) event_log[i] = -1;

	// our lock for testing the test_lock object
	std::atomic_flag lock_stream = ATOMIC_FLAG_INIT;	

	// seeds for randomizing the workload
	int* random_seeds = new int[num_threads];
	srand( time(NULL) );
	for (int i = 0; i < num_threads;i++){
		random_seeds[i] = rand();
	}

	// do the test
	//----------------
	#pragma omp parallel
	{
		int thread_id = omp_get_thread_num();
		
		// create a thread-specific random sequence 
		srand( random_seeds[thread_id] );

		for (int i = 0; i < num_turns; i++){

			test_lock->lock();

			// atomic logging, that the cs has been entered
			while ( lock_stream.test_and_set() ) {}
			event_log[counter] = 1; // cs has been entered
			counter++;
			lock_stream.clear();

			do_some_work(int(0.01*workload),0);// critical section

			// atomic releasing and logging, cs has been left
			while ( lock_stream.test_and_set() ) {}
			test_lock->unlock();		// unlock
			event_log[counter] = 0;		// cs has been left
			counter++;
			lock_stream.clear();

			do_some_work(workload,randomness);// noncritical section
		}
	}

	// evaluate event_log
	bool mutex = true;
	// make sure that it is a sequence of entering and leaving the cs
	for (int i = 0; i < num_events; i += 2) {
		if ((event_log[i] != 1) || (event_log[i + 1] != 0))
		{
			mutex = false;
		}
	}

	//output results
	//----------------
	if (print_to_console) {
		printf("lock acquisitions + lock releases = %d\n", counter);
		print_array(event_log, num_threads * 2, num_turns);
		printf("mutex = %d\n", mutex);
	}
	return mutex;
}


bool test_fcfs(DW_Lock* test_lock, int num_threads, 
				int num_turns, int workload, double randomness, 
				bool print_to_console = false)
{
	// this tests the fcfs property of a lock
	// returns true if fcfs held true, false otherwise
	// test_lock ... DW_Lock object, to be tested
	// num_threads ... number of threads for the test
	// num_turns ... number of times each thread executes the CS
	// randomness ... number in [0,1] to randomize workload of 
	//					work in non-CS
	// print_to_console ... console output on/off

	// FCFS according to Herlihy: whenever a thread A completes its
	// doorway before B starts its doorway, A gets in the CS before B.
	// to test this, the doorway section is made mutually exclusive

	//  initialize
	//----------------
	omp_set_num_threads(num_threads);
	// keep track of doorway completing threads
	int dw_completed = 0;
	int* dw_completers = new int[num_threads*num_turns];
	// keep track of lock acquiring threads
	int acquired = 0;
	int* acquirers = new int[num_threads*num_turns];
	
	// the whole doorway is made mutually exclusive
	Reference_Lock mutex_doorway;
	
	// seeds for randomizing the workload
	int* random_seeds = new int[num_threads];
	srand( time(NULL) );
	for (int i = 0; i < num_threads;i++){
		random_seeds[i] = rand();
	}

	// do the test
	//----------------
	#pragma omp parallel
	{
		int thread_id = omp_get_thread_num();
		
		// create a thread-specific random sequence 
		srand( random_seeds[thread_id] );

		for (int i = 0; i < num_turns; i++){

			mutex_doorway.lock();

			test_lock->doorway();
			dw_completers[dw_completed] = thread_id;
			dw_completed += 1;
			mutex_doorway.unlock();

			test_lock->wait();// waiting section
			acquirers[acquired] = thread_id;
			acquired += 1;
			
			do_some_work(int(0.01*workload),0);// critical section
			test_lock->unlock();		// unlock

			do_some_work(workload,randomness);// noncritical section
		}
	}

	// evaluate
	bool fcfs = isequal(dw_completers, acquirers, num_threads*num_turns);

	//output results
	//----------------
	if (print_to_console) {
		printf("dw_completed = %d\n", dw_completed);
		printf("acquired = %d\n", acquired);
		printf("dw_completers = \n");
		print_array(dw_completers, num_threads, num_turns);
		printf("acquirers = \n");
		print_array(acquirers, num_threads, num_turns);
		printf("FCFS = %d\n", fcfs);
	}
	return fcfs;
}