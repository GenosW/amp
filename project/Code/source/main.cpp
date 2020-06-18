//_79_column_check_line:#######################################################
#include  <stdio.h>
#include <omp.h>
#include <math.h>
#include <time.h>
// compiler switch to turn some debug-messages on
//#define DEBUG
#include <assert.h>
#include "locks.hpp" // basic lock interface + reference lock implementations
#include "lamport.hpp" // implementation of lamport bakery locks
#include "taubenfeld.hpp"	// implementation of taubenfeld (black/white) 
							// bakery locks
#include "aravind.hpp" // implementation of Aravind locks
#include "jayanti.hpp" // implementation of Jayanti locks
#include "tests.hpp" // lock tests (mutex, fcfs)
#include "toolbox.hpp" // various helper functions

/**
 * ./project2 num_threads num_turns num_tests 
 */
int main(int argc, char *argv[]){

	int num_threads = convertTo<int>(1, 4, argc, argv);
	int num_turns = convertTo<int>(2, 2, argc, argv);
	int num_tests = convertTo<int>(3, 1, argc, argv);
	int workload = convertTo<int>(4, 2, argc, argv);
	int cs_workload = convertTo<int>(5, 1, argc, argv);
	double randomness = convertTo<double>(6, 0.5, argc, argv);

	// amount of work in the critical/noncritical section
	// int workload = int(0.);
	// int cs_workload = int(0.);
	// // how random the wl in the noncritical section should be
	// double randomness = 0.;

	//// --- Lamport
	//Lamport_Lecture my_lock {num_threads};
	//Lamport_Lecture_fix my_lock{ num_threads };
	//Lamport_Lecture_atomic my_lock{ num_threads };
	//Lamport_Original my_lock{ num_threads };

	//// --- Taubenfeld
	Taubenfeld my_lock{num_threads};
	//Taubenfeld_fix my_lock{num_threads};
	//Taubenfeld_atomic my_lock{num_threads};

	//// --- Jayanti
	//Jayanti my_lock{ num_threads };
	//Jayanti_BT my_lock{ num_threads };

	//// --- Szymansky

	//// --- Aravind
	//Aravind my_lock{ num_threads };
	//Aravind_fix my_lock{ num_threads };
	
	//printf("calling my_test()\n");
	//my_lock.my_test();
	
	bool test_mutex_switch = true;
	bool test_fcfs_switch = true;
	bool test_lru_switch = false;
	bool test_bt_switch = false;
	bool comp_shared_counter_switch = false;
	bool anc_switch = false; // average number of contenders
	bool thr_switch = true; // throughput


	// Quick print to console that shows the configuration of the benchmark
	printf("\nTesting lock: %s\n", my_lock.name.c_str());
	printf("Performing mutex test: %d\n", test_mutex_switch);
	printf("Performing FCFS test: %d\n", test_fcfs_switch);
	printf("num_threads = %d\n", num_threads);
	printf("num_turns = %d\n", num_turns);
	printf("num_tests = %d\n", num_tests);

	// * Make sure to comment this out for the cluster! *
	char cont;
	printf("Press ENTER to start tests...");
	scanf("%c", &cont);
	// * Make sure to comment this out for the cluster! *

	double start, stop;

	start = omp_get_wtime();

	int mutex_fail_count = -1;
	if (test_mutex_switch) {
		printf("\n######################\n");
		printf("#     MUTEX TEST     #");
		printf("\n######################\n");
		mutex_fail_count = test_mutex(&my_lock,
										num_threads,
										num_turns,
										workload, 
										cs_workload,
										randomness,
										false);
	}

/*
	if (test_mutex_switch) {
		mutex_fail_count = 0;
		for (int i = 0; i < num_tests; i++) {
			mutex_fail_count += test_mutex(&my_lock,
				num_threads,
				num_turns,
				workload, cs_workload,
				randomness,
				false);
		}
	}
	*/
	stop = omp_get_wtime();
	double mutex_time = stop - start;
	printf("time elapesed in seconds = %.5f\n", mutex_time);
	start = stop;

	/*
	if (lock_class == "Jayanti") {
		Jayanti* jaya_lock = &my_lock;
		printf("\nJayanti X = %i\n", jaya_lock->read_X());
	}
	*/
	start = stop;


	int fcfs_fail_count = -1;
	if (test_fcfs_switch) {
		printf("\n----------------------\n");
		printf("\n######################\n");
		printf("#      FCFS TEST     #");
		printf("\n######################\n");

		fcfs_fail_count = test_fcfs(&my_lock,
									num_threads,
									num_turns,
									workload, 
									cs_workload,
									randomness,
									false);
	}
/*
	if (test_fcfs_switch) {
		fcfs_fail_count = 0;
		for (int i = 0; i < num_tests; i++) {
			fcfs_fail_count += test_fcfs(&my_lock,
				num_threads,
				num_turns,
				workload, cs_workload,
				randomness,
				false);
		}
	}
	*/

	stop = omp_get_wtime();
	double fcfs_time = stop - start;
	printf("time elapesed in seconds = %.5f\n", fcfs_time);
	start = stop;


	int lru_fail_count = -1;
	if (test_lru_switch) {
		printf("\n----------------------\n");
		printf("\n######################\n");
		printf("#       LRU TEST     #");
		printf("\n######################\n");

		lru_fail_count = test_lru(&my_lock,
								num_threads,
								num_turns,
								workload,
								cs_workload,
								randomness,
								false);
	}
/*
	if (test_lru_switch) {
		lru_fail_count = 0;
		for (int i = 0; i < num_tests; i++) {
			lru_fail_count += test_lru(&my_lock,
				num_threads,
				num_turns,
				workload, cs_workload,
				randomness,
				false);
		}
	}
	*/
	stop = omp_get_wtime();
	double lru_time = stop - start;
	printf("time elapesed in seconds = %.5f\n", lru_time);

	// test shared counter
	if (comp_shared_counter_switch) {
		printf("\n######################\n");
		printf("#    SHAR CNT CMP    #");
		printf("\n######################\n");

		int num_events = num_threads * num_turns * 4;
		int* event_log = new int[num_events * 2];
		// test_case = 1 (shared counter but no logging)
		start = omp_get_wtime();
		record_event_log2(event_log,
			&my_lock, 
			num_threads, 
			num_turns,
			workload, 
			cs_workload, 
			randomness, 
			false,
			1);
		stop = omp_get_wtime();
		printf("time with shared counter in seconds = %.5f\n", 
				stop - start);

		// test_case = 2 (no shared counter, no logging)
		start = omp_get_wtime();
		record_event_log2(event_log,
			&my_lock,
			num_threads,
			num_turns,
			workload,
			cs_workload,
			randomness,
			false,
			2);
		stop = omp_get_wtime();
		printf("time without shared counter in seconds = %.5f\n",
			stop - start);
	}

	// determine average number of contenders
	if (anc_switch) {
		printf("\n######################\n");
		printf("#         ANC        #");
		printf("\n######################\n");

		int num_events = num_threads * num_turns * 4;
		int* event_log = new int[num_events * 2];
		// test_case = 1 (shared counter but no logging)
		start = omp_get_wtime();
		record_event_log2(event_log,
			&my_lock,
			num_threads,
			num_turns,
			workload,
			cs_workload,
			randomness,
			false);
		stop = omp_get_wtime();
		printf("time elapsed in seconds = %.5f\n",
			stop - start);

		double anc = avg_num_contenders(event_log, num_threads, num_turns);
		printf("average number of contenders = %.4f\n\n", anc);
	}

	if (thr_switch) {
		printf("\n######################\n");
		printf("#     THROUGHPUT     #");
		printf("\n######################\n");
		start = omp_get_wtime();
		double tp = throughput(&my_lock, num_threads, num_turns,
			workload, cs_workload, randomness);
		stop = omp_get_wtime();
		printf("time elapsed in seconds = %.5f\n",
			stop - start);
		printf("Throughput = %.4e acquisitions per second\n\n", tp);
	}



	// throughput = num acquisitions/unlocks (=num acq) per time
	// should increase with num_treads
	// must keep balance: keep work in cs low but contention high
	// ...or: measure and subtract single thread time for total cs work

	printf("\n----------------------\n");
	printf("\n######################\n");
	printf("#       RESUMÉ       #");
	printf("\n######################\n");
	printf("\nLock name (attribute): %s\n", my_lock.name.c_str());
	printf("num_threads = %d\n", num_threads);
	printf("num_turns = %d\n", num_turns);
	printf("mutex_fail_count = %d\n", mutex_fail_count);
	printf("fcfs_fail_count = %d\n", fcfs_fail_count);
	printf("lru_fail_count = %d\n", lru_fail_count);
	

	//printf("\n testing RNG\n");
	//test_RNG(num_threads, num_turns);
	//test_random_workload(30,1e5,.9);

	bm_results results;
	results.lock_name = my_lock.name.c_str();
	results.num_threads = num_threads;
	results.num_turns = num_turns;
	results.num_tests = num_tests;
	results.mutex_fail_count = mutex_fail_count;
	results.fcfs_fail_count = fcfs_fail_count;
	results.lru_fail_count = lru_fail_count;
	results.mutex_time = mutex_time;
	results.fcfs_time = fcfs_time;
	results.lru_time = lru_time;
	string path = log_results(results, "data.csv");
	printf("\nSaved results in file: %s", path.c_str());

	// I coded the destructors to print a message when they are being called...
	// just to make sure that and where they are being called.
	// These messages should ONLY show after the following statement
	printf("\n\nGarbage collection...\n");

	return 0;	        
}

//_79_column_check_line:#######################################################
