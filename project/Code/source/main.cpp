//_79_column_check_line:#######################################################
#include <stdio.h>
#include <omp.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include "locks.hpp" // basic lock interface + reference lock implementations
#include "lamport.hpp" // implementation of lamport bakery locks
#include "taubenfeld.hpp"	// implementation of taubenfeld (black/white) 
							// bakery locks
#include "aravind.hpp" // implementation of Aravind locks
#include "jayanti.hpp" // implementation of Jayanti locks
#include "tests.hpp" // lock tests (mutex, fcfs)
#include "toolbox.hpp" // various helper functions


// compiler switch to turn some debug-messages on
//#define DEBUG
// compiler switch for more console output (usually only on desktop)
#define DESKTOP

/**
 * ./project2 num_threads num_turns num_tests workload cs_workload randomness
 */
int main(int argc, char *argv[]){

	int num_threads = convertTo<int>(1, 4, argc, argv);
	// how many times does every thread need to pass through critical section
	int num_turns = convertTo<int>(2, 2, argc, argv);
	int num_tests = convertTo<int>(3, 10, argc, argv);
	int workload = convertTo<int>(4, 1000, argc, argv);
	int cs_workload = convertTo<int>(5, 100, argc, argv);
	double randomness = convertTo<int>(6, 0.2, argc, argv);
	

// possible events: 1: begin doorway
//					2: finish doorway
//					3: acquire lock
//					4: unlock
	int num_events = num_threads * num_turns * 4;

	//// --- Lamport
	Lamport_Lecture my_lock {num_threads};
	//Lamport_Lecture_fix my_lock{ num_threads };
	//Lamport_Lecture_atomic my_lock{ num_threads };
	//Lamport_Original my_lock{ num_threads };

	//// --- Taubenfeld
	//Taubenfeld my_lock{num_threads};
	//Taubenfeld_fix my_lock{num_threads};

	//// --- Aravind
	//Aravind my_lock{ num_threads };
	//Aravind_fix my_lock{ num_threads };

	//// --- Jayanti
	//Jayanti my_lock{ num_threads };
	//Jayanti_BT my_lock{ num_threads };

	// C++ Reference Lock	
	//printf("calling my_test()\n");
	//my_lock.my_test();
	
	bool test_mutex_switch = false;
	bool test_fcfs_switch = false;
	bool test_lru_switch = false;
	bool test_bt_switch = false;
	bool comp_shared_counter_switch = true;
	bool anc_switch = false; // average number of contenders
	bool thr_switch = false; // throughput


	// Quick print to console that shows the configuration of the benchmark
#ifdef DESKTOP
	printf("\nTesting lock 2: %s\n", my_lock.name.c_str());
	printf("Performing mutex test: %d\n", test_mutex_switch);
	printf("Performing FCFS test: %d\n", test_fcfs_switch);
	printf("num_threads = %d\n", num_threads);
	printf("num_turns = %d\n", num_turns);
	printf("num_tests = %d\n", num_tests);
#endif

	int mutex_fail_count = -1;
	if (test_mutex_switch) {
#ifdef DESKTOP
		printf("\n######################\n");
		printf("#     MUTEX TEST     #");
		printf("\n######################\n");
#endif
		mutex_fail_count = test_mutex(&my_lock,
										num_threads,
										num_turns,
										workload, 
										cs_workload,
										randomness
										);
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

	int fcfs_fail_count = -1;
	if (test_fcfs_switch) {
#ifdef DESKTOP
		printf("\n----------------------\n");
		printf("\n######################\n");
		printf("#      FCFS TEST     #");
		printf("\n######################\n");
#endif
		fcfs_fail_count = test_fcfs(&my_lock,
									num_threads,
									num_turns,
									workload, 
									cs_workload,
									randomness
									);
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

	int lru_fail_count = -1;
	if (test_lru_switch) {
#ifdef DESKTOP
		printf("\n----------------------\n");
		printf("\n######################\n");
		printf("#       LRU TEST     #");
		printf("\n######################\n");
#endif
		lru_fail_count = test_lru(&my_lock,
								num_threads,
								num_turns,
								workload,
								cs_workload,
								randomness
								);
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

	// test shared counter
	if (comp_shared_counter_switch) {
#ifdef DESKTOP
		printf("\n######################\n");
		printf("#    SHAR CNT CMP    #");
		printf("\n######################\n");
#endif
		int* event_log = new int[num_events * 2];
		// runtime with record_event_log without logging
#ifdef DESKTOP
		printf("\ntime measurement from record_event_log (no logging)\n");
		printf("---------------------------------------------------\n");
#endif
		int* event_log2 = new int[num_events * 2];
		double* time_el1 = new double[num_tests];
		for (int i = 0; i < num_tests; i++) {
			time_el1[i] = record_event_log(event_log2,
				&my_lock,
				num_threads,
				num_turns,
				workload,
				cs_workload,
				randomness,
				2);
#ifdef DESKTOP
			printf("time elapsed in seconds = %.5f\n", time_el1[i]);
#endif
		}

#ifdef DESKTOP
		// runtime with full record_event_log
		printf("\ntime measurement from record_event_log (full logging)\n");
		printf("---------------------------------------------------\n");
#endif
		double* time_el2 = new double[num_tests];
		for (int i = 0; i < num_tests; i++) {
			time_el2[i] = record_event_log(event_log2,
				&my_lock,
				num_threads,
				num_turns,
				workload,
				cs_workload,
				randomness,
				0);
#ifdef DESKTOP
			printf("time elapsed in seconds = %.5f\n", time_el2[i]);
#endif
		}
		
	}

	// determine average number of contenders
	if (anc_switch) {
#ifdef DESKTOP
		printf("\n######################\n");
		printf("#         ANC        #");
		printf("\n######################\n");
#endif
		int* event_log = new int[num_events * 2];
		double start = omp_get_wtime();
		record_event_log(event_log,
							&my_lock,
							num_threads,
							num_turns,
							workload,
							cs_workload,
							randomness
							);
		double stop = omp_get_wtime();
		double anc = avg_num_contenders(event_log, num_threads, num_turns);
#ifdef DESKTOP
		printf("average number of contenders = %.4f\n\n", anc);
#endif
	}

	if (thr_switch) {
#ifdef DESKTOP
		printf("\n######################\n");
		printf("#     THROUGHPUT     #");
		printf("\n######################\n");
#endif
		/*
		// runtime with dedicated throughput function
		printf("\ndesignated throughput test\n");
		printf("---------------------------------------------------\n");
		double* tp = new double[num_tests];
		for (int i = 0; i < num_tests; i++) {
			tp[i] = throughput(&my_lock, num_threads, num_turns,
				workload, cs_workload, randomness);
			printf("Throughput = %.4e acquisitions per second\n", tp[i]);
			printf("calculated time = %.5f\n", 
				num_threads*num_turns / tp[i]);
		}
		*/
#ifdef DESKTOP
		// runtime with record_event_log without logging
		printf("\ntime measurement from record_event_log (no logging)\n");
		printf("---------------------------------------------------\n");
#endif
		int* event_log2 = new int[num_events * 2];
		double* time_el1 = new double[num_tests];
		for (int i = 0; i < num_tests; i++) {
			time_el1[i] = record_event_log(event_log2,
				&my_lock,
				num_threads,
				num_turns,
				workload,
				cs_workload,
				randomness,
				0);
#ifdef DESKTOP
			printf("time elapsed in seconds = %.5f\n", time_el1[i]);
#endif
		}
	}

#ifdef DESKTOP
	//log_results()
	// Everything below will eventually be moved into log_results(...), see toolbox
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
#endif	

	//printf("\n testing RNG\n");
	//test_RNG(num_threads, num_turns);
	//test_random_workload(30,1e5,.9);

#ifdef DESKTOP
	printf("\n\nGarbage collection...\n");
#endif
	return 0;	        
}

//_79_column_check_line:#######################################################
