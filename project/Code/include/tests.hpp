//_79_column_check_line:#######################################################
#include <stdio.h>
#include <omp.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <random>

#include "locks.hpp"
#include "toolbox.hpp"

//#define DEBUG_LRU
//#define DEBUG_RNG
//#define DEBUG_LOG
//#define DEBUG_FCFS


/* Declaration */
void do_some_work(int workload, double randomness);

int test_fcfs(DW_Lock* test_lock, int num_threads,
	int num_turns, int workload, int cs_workload,
	double randomness,
	bool print_to_console = false, bool mutex_dw = false);
int test_mutex(Lock* test_lock, int num_threads,
	int num_turns, int workload, int cs_workload,
	double randomness,
	bool print_to_console = false);
bool test_fcfs_mutex_dw(DW_Lock* test_lock, int num_threads,
	int num_turns, int workload, int cs_workload,
	double randomness,
	bool print_to_console = false);


void get_RNG_seeds(int* seeds, int num_threads) {
	// little RNG to generate the seeds for threads RNGs
	// call this in non-parallel section (duh!)
	std::random_device device;
	for (int i = 0; i < num_threads; i++) {
		seeds[i] = device();
	}
#ifdef DEBUG_RNG
	printf("\nseeds for RNG\n");
	print_array(seeds, num_threads, 1);
#endif // DEBUG_RNG
}


void test_RNG(int num_threads, int num_turns) {
	omp_set_num_threads(num_threads);

	int min = 0;
	int max = 1000;
	int* rnd_seq = new int[num_turns*num_threads];
	int* seeds = new int[num_threads];

	get_RNG_seeds(seeds, num_threads);

#pragma omp parallel
	{
		int id = omp_get_thread_num();
		// thread creates their own RNG
		std::mt19937 generator(seeds[id]);
		std::uniform_int_distribution<int> distribution(min, max);
		//write some random number into the array
		for (int i = 0; i < num_turns; i++) {
			rnd_seq[i*num_threads + id] = distribution(generator);
		}
	}
	// print to screen
	print_array(rnd_seq, num_threads, num_turns);
}

/* Implementation */
void do_some_work(int workload, double randomness) {
	// does some work (workload * 3 flops)
	// randomness in [0,1] is the portion of the workload
	// that is randomized 0... full workload, 
	//					0.5... workload between 50-100%
	//					  1... workload between 0-100%

	assert((0 <= randomness) && (randomness <= 1));

	double randy = (rand() % 1000) / 1000.;
	int rand_wl = int(((1 - randomness) + randomness * randy) * workload);

	double dummy = 1.23;

#ifdef DEBUG_RNG
	int id = omp_get_thread_num();
	printf("id = %d, rand_wl = %d\n", id, rand_wl);
#endif

	for (int i = 0; i < rand_wl; i++) {
		dummy = dummy + dummy;
		dummy = dummy + 1e-10;
		dummy = dummy / 2;
	}
}

void do_some_work2(int workload, double randomness, std::mt19937 generator) {
	// 2nd version of do_some_work with thread safe random number generation
	// every thread passes their own RNG when calling this
	// does some work (workload * 3 flops)
	// randomness in [0,1] is the portion of the workload
	// that is randomized 0... full workload, 
	//					0.5... workload between 50-100%
	//					  1... workload between 0-100%

	assert((0 <= randomness) && (randomness <= 1));
	int rand_wl;
	if (randomness > 0){
		int min = 0;
		int max = 1000;
		std::uniform_int_distribution<int> distribution(min, max);

		double randy = distribution(generator) / 1000.;
		rand_wl = int(((1 - randomness) + randomness * randy) * workload);


#ifdef DEBUG_RNG
		int id = omp_get_thread_num();
		printf("id = %d, rand_wl = %d\n", id, rand_wl);
#endif
	}
	else rand_wl = workload;

	double dummy = 1.23;
	for (int i = 0; i < rand_wl; i++) {
		dummy = dummy + dummy;
		dummy = dummy + 1e-10;
		dummy = dummy / 2;
	}
}

void record_event_log(int* event_log, DW_Lock* test_lock,  int num_threads, 
	int num_turns, int workload, int cs_workload, double randomness,
	bool mutex_dw) {

	omp_set_num_threads(num_threads);

	// counter keeps track of the logging position
	int counter = 0;
	// an event is begin, finish, acquisition and unlock
	int num_events = num_threads * num_turns * 4;

	//initialize logging array
	for (int i = 0; i < num_events * 2; i++) event_log[i] = -1;

	// our lock for testing the test_lock object
	std::atomic_flag lock_stream = ATOMIC_FLAG_INIT;

	// seeds for randomizing the workload
	int* random_seeds = new int[num_threads];
	get_RNG_seeds(random_seeds, num_threads);


#pragma omp parallel
	{
		int thread_id = omp_get_thread_num();

		// create a thread-specific random sequence 
		//srand(random_seeds[thread_id]);

		//create RNGs
		std::mt19937 rng(random_seeds[thread_id]);
		//std::uniform_int_distribution<int> distribution(min, max);

#pragma omp barrier
		// ready... get set... go!

		for (int i = 0; i < num_turns; i++) {

			// atomically write, that you are about to begin the doorway
			while (lock_stream.test_and_set()) {}
			event_log[counter] = thread_id;
			event_log[counter + 1] = 1;
			counter += 2;
			lock_stream.clear();

			test_lock->doorway();

			// atomically write, that you finished the doorway
			while (lock_stream.test_and_set()) {}
			event_log[counter] = thread_id;
			event_log[counter + 1] = 2;
			counter += 2;
			lock_stream.clear();

			test_lock->wait();

			// atomically write, that you acquired the lock
			while (lock_stream.test_and_set()) {}
			event_log[counter] = thread_id;
			event_log[counter + 1] = 3;
			counter += 2;
			lock_stream.clear();

			// critical section
			do_some_work2(cs_workload, 0, rng);

			// atomically write that, you are about to unlock
			while (lock_stream.test_and_set()) {}
			test_lock->unlock();
			event_log[counter] = thread_id;
			event_log[counter + 1] = 4;
			counter += 2;
			lock_stream.clear();

			// noncritical section
			do_some_work2(workload, randomness, rng);
		}
	}

}

void record_event_log2(int* event_log, DW_Lock* test_lock, int num_threads,
	int num_turns, int workload, int cs_workload, double randomness,
	bool mutex_dw) {

	omp_set_num_threads(num_threads);

	// counter keeps track of the logging position
	atomic <int> counter;
	counter.store(0);
	// an event is begin, finish, acquisition and unlock
	// this is the number of events for each thread
	int num_events = num_turns * 4;

	//initialize logging array
	for (int i = 0; i < num_events*num_threads*2; i++) event_log[i] = -1;

	// our lock for testing the test_lock object
	std::atomic_flag lock_stream = ATOMIC_FLAG_INIT;

	// seeds for randomizing the workload
	int* random_seeds = new int[num_threads];
	get_RNG_seeds(random_seeds, num_threads);


#pragma omp parallel
	{
		int thread_id = omp_get_thread_num();
		int tl_counter = 0; // thread local counter
		int counter_value; // value read from global counter
		// an event is begin, finish, acquisition and unlock
		// num_events*2 because 2 elements are written per event
		int* tl_event_log = new int[num_events * 2]; // thread local log

		//initialize logging array
		for (int i = 0; i < num_events * 2; i++) tl_event_log[i] = -1;

		// create a thread-specific random sequence 
		//srand(random_seeds[thread_id]);

		//create RNGs
		std::mt19937 rng(random_seeds[thread_id]);
		//std::uniform_int_distribution<int> distribution(min, max);

#pragma omp barrier
		// ready... get set... go!

		for (int i = 0; i < num_turns; i++) {

			// (1) begin doorway
			// atomically read/increas global counter
			counter_value = counter.fetch_add(1);
#ifdef DEBUG_LOG
			printf("thread: %i, event: %i, global counter: %i\n",
				thread_id, 1, counter_value);
#endif
			tl_event_log[tl_counter] = counter_value; // log counter value
			tl_event_log[tl_counter + 1] = 1;	// log event type
			tl_counter += 2;

			test_lock->doorway();

			// (2) finished doorway
			// atomically read/increas global counter
			counter_value = counter.fetch_add(1);
#ifdef DEBUG_LOG
			printf("thread: %i, event: %i, global counter: %i\n",
				thread_id, 2, counter_value);
#endif
			tl_event_log[tl_counter] = counter_value; // log counter value
			tl_event_log[tl_counter + 1] = 2;	// log event type
			tl_counter += 2;

			test_lock->wait();

			// (3) acquired lock
			// atomically read/increas global counter
			counter_value = counter.fetch_add(1);
#ifdef DEBUG_LOG
			printf("thread: %i, event: %i, global counter: %i\n",
				thread_id, 3, counter_value);
#endif
			tl_event_log[tl_counter] = counter_value; // log counter value
			tl_event_log[tl_counter + 1] = 3;	// log event type
			tl_counter += 2;

			// critical section
			do_some_work2(cs_workload, 0, rng);

			// (4) unlock
			// atomically read/increas global counter
			counter_value = counter.fetch_add(1);
#ifdef DEBUG_LOG
			printf("thread: %i, event: %i, global counter: %i\n",
				thread_id, 4, counter_value);
#endif
			tl_event_log[tl_counter] = counter_value; // log counter value
			tl_event_log[tl_counter + 1] = 4;	// log event type
			tl_counter += 2;
			test_lock->unlock();

			// noncritical section
			do_some_work2(workload, randomness, rng);
		}
#pragma omp barrier

		// join thread local arrays to global event_log
		int global_counter;
		int event_type;

		while (lock_stream.test_and_set()) {}
		for (int i = 0; i < num_events; i++) {
			// 2*i, because on each turn 2 elements are written
			global_counter = tl_event_log[2*i];
			event_type = tl_event_log[2*i + 1];

			// 2* global counter: because each event only increases
			// the counter by 1; but 2 elements are written per event
			event_log[2*global_counter] = thread_id;
			event_log[2*global_counter + 1] = event_type;
		}
		lock_stream.clear();
	}
}



int test_fcfs(DW_Lock* test_lock, int num_threads,
	int num_turns, int workload, int cs_workload,
	double randomness,
	bool print_to_console, bool mutex_dw)
{
/*
		This tests the first-come-first-served property of the 
		The Idea of this testing algorithm
		###################################

		-) some definitions:
		A,B,C ... threads
		begin ... the event of entering a doorway section
		finish ... the event of finishing a doorway section
		acquire ... the event of acquiring a lock
		the "corresponding finish" to an (A, acquire): the latest (A, finish)
		before (A,acquire).
		the "corresponding begin" to an(A, finish): the latest(A, begin)
		before(A, finish).
		the "corresponding begin" to an(A, acquire): 
		the corresponding (A, begin) to the corresponding (A,finish)

		-) Algorithm in words:
		for every (A,acquire), go backwards through the array until 
		the corresponding (A,finish). Whenever finding a (B,acquire), check, 
		if the corresponding (B,begin) is after the (A,finish). If it is,
		fcfs is violated. (A was overtaken). If no thread is ever overtaken,
		fcfs holds.
*/

	// declare the array where the events are being logged
	// an event is begin, finish, acquisition and unlock
	int num_events = num_threads * num_turns * 4;
	// times two because thread_id is also written
	int* event_log = new int[num_events*2]; 


	// run the test
	// ---------------
	double start, stop;
	bool display_time = true;
	if (display_time) {
		printf("running test... \n");
		start = omp_get_wtime();
	}

	//record_event_log(event_log, test_lock, num_threads, num_turns,
	//	workload, cs_workload, randomness, mutex_dw);
	record_event_log2(event_log, test_lock, num_threads, num_turns,
		workload, cs_workload, randomness, mutex_dw);

	if (display_time) {
		stop = omp_get_wtime();
		printf("...time elapesed in seconds = %.5f\n", stop - start);
	}


	if (print_to_console) {
		printf("logging for fcfs completed: here the whole log\n");
		print_log(event_log, num_threads * 8, num_turns);
	}

	// evaluate results
	// -----------------
	if (display_time) {
		printf("evaluating event log... \n");
		start = omp_get_wtime();
	}

	bool fcfs = true;
	
	int thread_A = -1;
	int corr_finish = -1;
	int thread_B = -1;
	int fcfs_violations = 0;

	for (int i = 0; i < num_events * 2; i += 2) {
		if (event_log[i + 1] == 3) // if acquisition
		{
			thread_A = event_log[i]; // the acquiring thread A
#ifdef DEBUG_FCFS
			printf("checking acquisition at pos = %i\n", i);
#endif
			 // find corresponding finish
			for (int j = i; j > -1; j -= 2)
			{
				if ((event_log[j + 1] == 2) // if finish
					&& (event_log[j] == thread_A)) // and its A
				{
					corr_finish = j; 
#ifdef DEBUG_FCFS
					printf("corr_finish at pos = %i\n", corr_finish);
#endif
					break;
				}
			}
			// check if someone wrongfully overtook A
			for (int j = i; j > corr_finish; j -= 2)
			{
				if ((event_log[j + 1] == 3) // if acquisition
					&& (event_log[j] != thread_A)) // and its not A
				{
					thread_B = event_log[j];
					// make sure B didnt begin his doorway 
					// after A finished its doorway
					for (int k = j; k > corr_finish; k -= 2)
					{
						if ((event_log[k] == thread_B)
							&& (event_log[k + 1] == 1))
						{
							fcfs = false;
							if (print_to_console)
							{
								printf("FCFS violated:\n"
									"thread %i wrongfully overtook "
									"thread %i\n",
									thread_B, thread_A);
								printf("Thread %i doorway finish: %i\n",
									thread_A, corr_finish);
								printf("Thread %i doorway begin: %i\n",
									thread_B, k);
								printf("Thread %i lock acquisition: %i\n",
									thread_A, i);
								printf("Thread %i lock acquisition: %i\n",
									thread_B, j);

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
							//return fcfs;
							fcfs_violations++;
							break; // every acquisition only counts once
						}
					}
				}
			}
		}
	}
	if (display_time) {
		stop = omp_get_wtime();
		printf("...time elapesed in seconds = %.5f\n", stop - start);
	}

	//return fcfs;
	return fcfs_violations;
}


int test_mutex(Lock* test_lock, int num_threads, 
	int num_turns, int workload, int cs_workload,
	double randomness,
	bool print_to_console)
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

	// make sure that it is a sequence of entering and leaving the cs

	/* this was the old evaluation
	bool mutex = true;
	for (int i = 0; i < num_events; i += 2) {
		if ((event_log[i] != 1) || (event_log[i + 1] != 0))
		{
			mutex = false;
		}
	}
	*/
	// these days we count the number of threads entering the cs,
	// when there is already a thread in there
	int mutex_violations = 0;
	for (int i = 0; i < num_events; i ++) {
		if ((event_log[i] == 1) && (event_log[i + 1] == 1))
		{
			mutex_violations++;
		}
	}


	//output results
	//----------------
	if (print_to_console) {
		printf("lock acquisitions + lock releases = %d\n", counter);
		print_array(event_log, num_threads * 2, num_turns);
		printf("mutex_violations = %d\n", mutex_violations);
	}
//	return mutex;
	return mutex_violations;
}


bool test_fcfs_mutex_dw(DW_Lock* test_lock, int num_threads, 
				int num_turns, int workload, int cs_workload,
				double randomness, 
				bool print_to_console)
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


void find_contenders_lru(bool* is_contender, int* event_log, 
					int acquire_pos, int num_threads) {
	// finds the contenders of an acquisition (A,acquire)
	// needed for the lru_test
	/*
	1.) Definition: "Contender of an acquisition"
	thread B is a contender of (A, acquire)_k if and only if
	there is an l in N such that
	(B, finish)_l -> (A, acquire)_k -> (B, acquire)_l
	in words : B has finished his doorway and is trying to acquire
	the lock, when A acquires.
	*/

	// the thread that looks for their contenders (thread A)
	int thread_A = event_log[acquire_pos];

	// array that stores, if a thread has had an acquisition after their
	// last doorway finish and is therefore not a contender

	for (int thread_B = 0; thread_B < num_threads; thread_B++) {
		// check if thread_B is a contender
		
		// skip thread_A
		if (thread_B == thread_A) {
			thread_B++;
			if (thread_B == num_threads) { break; }
		}

		is_contender[thread_B] = false;
		for (int j = acquire_pos; j > 0; j = j - 2) {
			// idea: if we find an acquisition first: not is_contender
			// if we find a finish first: is_contender

			// if thread_B had an acquisition => is no contender
			if ((event_log[j] == thread_B) && (event_log[j+1] == 3)) {
				break;
			}
			// if thread_B had a finish => is a contender
			if ((event_log[j] == thread_B) && (event_log[j + 1] == 2)) {
				is_contender[thread_B] = true;
			}
		}
	}
}

int check_acquisition_lru(int* event_log,
	int acquire_pos, int num_threads) {
	/*
	2.) Definition: "LRU"
	in a history LRU holds if and only if
	for each (A, acquire)_k : for each contender B :
	(A, acquire)_(k - 1) -> (B, acquire)_(l - 1)
	with l as in the definition of "contender" and k > 1
	in words : A can only acquire, if his last acquisition was before
	each contenders last acquisition
	*/

	bool* is_contender = new bool[num_threads];
	int thread_A = event_log[acquire_pos];
	int num_violations = 0;
	find_contenders_lru(is_contender, event_log, acquire_pos, num_threads);

	for (int thread_B = 0; thread_B < num_threads; thread_B++) {
		// for each contender
		if (is_contender[thread_B]) {
#ifdef DEBUG_LRU
			printf("thread %d is contender...\n", thread_B);
#endif

			// the contender must have had an acquisition after thread A
			// so if we find (A,acquire)_(k-1) first: lru is violated
			// if we find (B,acquire)_(l-1) first: lru holds
			for (int j = acquire_pos-2; j > 0; j = j - 2) {
				if ((event_log[j] == thread_B) && (event_log[j + 1] == 3)) {
#ifdef DEBUG_LRU
					printf("... but priority was lower\n");
#endif
					break;
				}
				if ((event_log[j] == thread_A) && (event_log[j + 1] == 3)) {
#ifdef DEBUG_LRU
					printf("... and priority was higher\n");
#endif
					//return false;
					num_violations++;
					break;
				}
			}
		}
	}
	return num_violations;
}

int test_lru(DW_Lock* test_lock, int num_threads,
	int num_turns, int workload, int cs_workload,
	double randomness,
	bool print_to_console)
{
	// returns the number of acquisitions that violated the LRU property
	/*
			This tests the least-recently-used property of test_lock
			The Idea of this testing algorithm
			###################################

			Aravind 2011 does not explicitly define LRU so it is done
			here
			1.) Definition: "Contender of an acquisition"
			thread B is a contender of (A,acquire)_k if and only if
			there is an l in N such that
			(B,finish)_l -> (A,acquire)_k 
				&& not( (B,acquire)_l -> (A,acquire)_k )
			in words: B has finished his doorway but has not acquired the lock
			yet, when A acquires.

			2.) Definition: "LRU"
			in a history LRU holds if and only if
			for each (A,acquire)_k: for each contender B:
			(A,acquire)_(k-1) -> (B,acquire)_(l-1)
			with l as in the definition of "contender" and k>1
			in words: A can only acquire, if his last acquisition was before
			each contenders last acquisition

			Description of the testing algorithm in words:
			at every (A,acquire)_k for k>1 we look at all 
			contenders and make sure, that going back to (A,acquire)_(k-1) 
			we find an acquisition of each contender.

			This can be violated, if a contender came really late though.
			I think to deal with this, we might need memory fences. 
			But that is too advanced for me. How can i measure it? 
			Chances are, it wont happen, because if you come later, you 
			probably got the lock later.

			Heres the real thing: if some thread B finishes their doorway
			after thread A starts their last spin in the wait() before
			the acquisition, it is possible, that (A,acquire) even though 
			there is a contender (B) with higher priority.
			This is (a) unlikely and (b) can be kinda fixed, by putting another
			test in stage 1: if the set of contenders has changed between
			you entering stage 1 and you checking for the last time before
			you acquire, you go back to spinning. This still holds the 
			possibility for a thread to finish their doorway just between
			you testing and you acquiring the lock. But it is even less likely.



	*/


	omp_set_num_threads(num_threads);

	// counter keeps track of the logging position
	int counter = 0;
	// an event is begin, finish, acquisition and unlock
	int num_events = num_threads * num_turns * 4;
	// times two because thread_id is also written
	int* event_log = new int[num_events * 2];

	//initialize logging array
	for (int i = 0; i < num_events * 2; i++) event_log[i] = -1;

	// our lock for testing the test_lock object
	std::atomic_flag lock_stream = ATOMIC_FLAG_INIT;

	// seeds for randomizing the workload
	int* random_seeds = new int[num_threads];
	srand(time(NULL));
	for (int i = 0; i < num_threads; i++) {
		random_seeds[i] = rand();
	}

	// send threads through the lock and log the events
	record_event_log2(event_log, test_lock, num_threads, num_turns,
		workload, cs_workload, randomness, false);

	if (print_to_console) {
		printf("logging for lru completed: here the whole log\n");
		print_log(event_log, num_threads * 8, num_turns);
	}


	// evaluate event_log
	//----------------------

	// keep a list of threads, that have already acquired at least once
	bool* has_ac_once = new bool[num_threads];
	for (int i = 0; i < num_threads; i++) {
		has_ac_once[i] = false;
	}
	bool lru;
	int lru_violations = 0;
	for (int i = 0; i < num_events; i = i + 2) {
		// if it is an acquisition
		if (event_log[i + 1] == 3) {
			// if the acquisition is not the first one of this thread
			if (has_ac_once[event_log[i]]) {
				// check this acquisition...
#ifdef DEBUG_LRU
				printf("checking acquisition at pos %d\n", i);
#endif
				//lru = check_acquisition_lru(event_log, i, num_threads);
#ifdef DEBUG_LRU
				printf("lru = %d\n", lru);
#endif
				//if (not(lru)) { return false; }
				//if (not(lru)) { lru_violations++; }
				lru_violations += 
					check_acquisition_lru(event_log, i, num_threads);

			}
			// if it was the first one, mark it so the next one is not
			// the first one
			else { has_ac_once[event_log[i]] = true; }
		}
	}
	// if lru was at no point violated...
	//return true;
	// these days, however, we rather return the number of violations
	return lru_violations;
}

//_79_column_check_line:#######################################################