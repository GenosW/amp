
#include  <stdio.h>
#include <omp.h>
#include <math.h>
#include <time.h>

#include "locks.cpp" // lock implementations
//#include "toolbox.cpp" // helper functions are already included in locks.cpp

#include <assert.h>

// compiler switch to turn some debug-messages on
//#define DEBUG

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


bool test_fcfs(DW_Lock* test_lock, int num_threads,
	int num_turns, int workload, int cs_workload,
	double randomness,
	bool print_to_console = false, bool mutex_dw = false)
{
/*
	wia mocht ma des....
		einer schreibt mit
		begin
		finish
		acquire

		das sind 3*nt*nturns einträge * 2 weil thread_id auch immer schreiben

		ausserdem... alias? könnt ma machen.

		the corresponding finish to an (A, acquire): the latest (A, finish)
		before (A,acquire).
		the corresponding begin to an(A, finish): the latest(A, begin)
		before(A, finish).
		the corresponding begin to an(A, acquire): 
		the corresponding (A, begin) to the corresponding (A,finish)

		for every (A,acquire), go backwards through the array until 
		the corresponding (A,finish). Whenever finding a (B,acquire), check, 
		if the corresponding (B,begin) is after the (A,finish). If it is,
		fcfs is violated. (A was overtaken). If no thread is ever overtaken,
		fcfs holds.
*/

	omp_set_num_threads(num_threads);

	// counter keeps track of the logging position
	int counter = 0;
	// an event is begin, finish and acquisition of a lock
	int num_events = num_threads * num_turns * 4;
	// times two because thread_id is also written
	int* event_log = new int[num_events*2]; 

	//initialize logging array
	for (int i = 0; i < num_events*2; i++) event_log[i] = -1;

	// our lock for testing the test_lock object
	std::atomic_flag lock_stream = ATOMIC_FLAG_INIT;

	// seeds for randomizing the workload
	int* random_seeds = new int[num_threads];
	srand(time(NULL));
	for (int i = 0; i < num_threads; i++) {
		random_seeds[i] = rand();
	}

	// do the test
	//----------------
#pragma omp parallel
	{
		int thread_id = omp_get_thread_num();

		// create a thread-specific random sequence 
		srand(random_seeds[thread_id]);

		for (int i = 0; i < num_turns; i++) {

			// atomically write that you are about to begin the doorway
			while (lock_stream.test_and_set()) {}
			event_log[counter] = thread_id;
			event_log[counter+1] = 1;
			counter += 2;
			lock_stream.clear();

			test_lock->doorway();

			// atomically write that you finished the doorway
			while (lock_stream.test_and_set()) {}
			event_log[counter] = thread_id;
			event_log[counter + 1] = 2;
			counter += 2;
			lock_stream.clear();

			test_lock->wait();

			// atomically write that you acquired the lock
			while (lock_stream.test_and_set()) {}
			event_log[counter] = thread_id;
			event_log[counter + 1] = 3;
			counter += 2;
			lock_stream.clear();

			do_some_work(cs_workload, 0);// critical section

			// atomically write that you unlocked the lock
			while (lock_stream.test_and_set()) {}
			test_lock->unlock();
			event_log[counter] = thread_id;
			event_log[counter + 1] = 4;
			counter += 2;
			lock_stream.clear();

			do_some_work(workload, randomness);// noncritical section
		}
	}
	if (print_to_console) {
		printf("logging for fcfs completed: here the whole log\n");
		print_array(event_log, num_threads * 8, num_turns);
	}


	// evaluate results
	// -----------------
	bool fcfs = true;
	
	int my_thread = -1;
	int corr_finish = -1;
	int other_thread = -1;

	for (int i = 0; i < num_events * 2; i += 2) {
		if (event_log[i + 1] == 3) // if acquisition
		{
			my_thread = event_log[i]; // the acquiring thread "me"
#ifdef DEBUG
			printf("checking acquisition at pos = %i\n", i);
#endif
			 // find corresponding finish
			for (int j = i; j > -1; j -= 2)
			{
				if ((event_log[j + 1] == 2) // if finish
					&& (event_log[j] == my_thread)) // and its me
				{ 
					corr_finish = j; 
#ifdef DEBUG
					printf("corr_finish at pos = %i\n", corr_finish);
#endif
					break;
				}
			}
			// check if someone wrongfully overtook me
			for (int j = i; j > corr_finish; j -= 2)
			{
				if ((event_log[j + 1] == 3) // if acquisition
					&& (event_log[j] != my_thread)) // and its not me
				{
					other_thread = event_log[j];
					// make sure this guy didnt begin his doorway 
					// after I finished my doorway
					for (int k = j; k > corr_finish; k -= 2)
					{
						if ((event_log[k] == other_thread)
							&& (event_log[k + 1] == 1))
						{
							fcfs = false;
							if (print_to_console)
							{
								printf("FCFS violated:\n"
									"thread %i wrongfully overtook "
									"thread %i\n",
									other_thread, my_thread);
								printf("Thread %i doorway finish: %i\n",
									my_thread, corr_finish);
								printf("Thread %i doorway begin: %i\n",
									other_thread, k);
								printf("Thread %i lock acquisition: %i\n",
									my_thread, i);
								printf("Thread %i lock acquisition: %i\n",
									other_thread, j);

								int num_lines = int((i-corr_finish) 
												/ (num_threads * 8)) + 1;
								printf("here the sequence starting at %i\n",
									corr_finish);
								printf("(number of columns = %i)\n",
									num_threads*8);

								print_log(&event_log[corr_finish],
									num_threads * 8,
									num_lines);
							}
							return fcfs;
						}
					}
				}
			}
		}
	}
	return fcfs;
}


// all tests should end up in their own module
// ----------------------------------------------
bool test_mutex(Lock* test_lock, int num_threads, 
	int num_turns, int workload, int cs_workload,
	double randomness,
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

			do_some_work(cs_workload, 0);// critical section

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


bool test_fcfs_mutex_dw(DW_Lock* test_lock, int num_threads, 
				int num_turns, int workload, int cs_workload,
				double randomness, 
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
			
			do_some_work(cs_workload, 0);// critical section
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

void test_reference(Reference_Lock* lock1, Reference_Lock* lock2) {
	// after reading on the internet, i got worried that there can 
	// not be several instances of the atomic_flag class
	// this function is there to make sure, you can have several
	// atomic_flag objects
	omp_set_num_threads(4);
#pragma omp parallel
	{
		int i = omp_get_thread_num();
		printf("thread %i tries to get into CS1\n",i);
		lock1->lock();
		printf("thread %i is in CS1\n", i);
		printf("thread %i tries to get into CS2\n", i);
		lock2->lock();
		printf("thread %i is in CS2\n", i);
		lock1->unlock();
		lock2->unlock();
	}
}


void test_atomic_flag() {
	// after reading on the internet, i got worried that there can 
	// not be several instances of the atomic_flag class
	// this function is there to make sure, you can have several
	// atomic_flag objects
	std::atomic_flag guard1 = ATOMIC_FLAG_INIT;
	std::atomic_flag guard2 = ATOMIC_FLAG_INIT;

	omp_set_num_threads(4);
#pragma omp parallel
	{
		int i = omp_get_thread_num();
		printf("thread %i tries to get into CS1\n", i);
		while ( guard1.test_and_set() ) {}

		printf("thread %i is in CS1\n", i);
		printf("thread %i tries to get into CS2\n", i);

		while (guard2.test_and_set()) {}
		printf("thread %i is in CS2\n", i);
		guard1.clear();
		guard2.clear();
	}
}



int main(){

	int num_threads = 4;
	// how many times does every thread need to pass through critical section
	int num_turns = 100;

	// amount of work in the noncritical section
	int workload = int(1e3);
	// workload in critical section
	int cs_workload = int(1*workload);
	// how random the wl in the noncritical section should be
	double randomness = 0.5; 

	// i fooled around with different ways to instanciate locks
	//auto my_lock = new Lamport_Lecture(num_threads);
	//Lamport_Lecture* my_lock = new Lamport_Lecture{num_threads};
	//DW_Lock* my_lock = new Lamport_Lecture{num_threads};

	// this is now my preferred way to instanciate locks
	//Lamport_Lecture my_lock {num_threads};
	Lamport_Lecture_fix my_lock{ num_threads };
	//Lamport_Original my_lock{ num_threads };
	//Reference_Lock my_lock{};
	//Reference_Lock my_lock2{};

	//test_reference(&my_lock, &my_lock2);
	//test_atomic_flag();

	int num_tests = 100;
	bool test_mutex_switch = true;
	bool test_fcfs_medw_switch = true;
	bool test_fcfs_switch = true;

	printf("num_threads = %d\n", num_threads);
	printf("num_turns = %d\n", num_turns);
	printf("num_tests = %d\n", num_tests);

	if (test_mutex_switch) {
		int mutex_fail_count = 0;
		for (int i = 0; i < num_tests; i++) {
			mutex_fail_count += (1 - test_mutex(&my_lock,
				num_threads,
				num_turns,
				workload,
				cs_workload,
				randomness)
				);
		}
		printf("mutex_fail_count = %d\n", mutex_fail_count);
	}

	if (test_fcfs_medw_switch) {
		int fcfs_medw_fail_count = 0;
		for (int i = 0; i < num_tests; i++) {
			fcfs_medw_fail_count += (1 - test_fcfs_mutex_dw(&my_lock,
				num_threads,
				num_turns,
				workload,
				cs_workload,
				randomness)
				);
		}
		printf("fcfs_medw_fail_count = %d\n", fcfs_medw_fail_count);
	}

	if (test_fcfs_switch) {
		int fcfs_fail_count = 0;
		for (int i = 0; i < num_tests; i++) {
			fcfs_fail_count += (1 - test_fcfs(&my_lock,
				num_threads,
				num_turns,
				workload,
				cs_workload,
				randomness,
				false,false));
		}
		printf("fcfs_fail_count = %d\n", fcfs_fail_count);
	}


	return 0;	        
}

////////////////////////////////////////////////////////////////////////////79
