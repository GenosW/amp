//_79_column_check_line:#######################################################
#include <stdio.h>
#include <omp.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <vector>
#include "tests.hpp" // lock tests (mutex, fcfs)
#include "toolbox.hpp" // various helper functions
#include "locks.hpp" // basic lock interface + reference lock implementations
#include "lamport.hpp" // implementation of lamport bakery locks
#include "taubenfeld.hpp"	// implementation of taubenfeld (black/white) 
							// bakery locks
#include "aravind.hpp" // implementation of Aravind locks
#include "jayanti.hpp" // implementation of Jayanti locks


// compiler switch to turn some debug-messages on
//#define DEBUG
// compiler switch for more console output (usually only on desktop)
#define DESKTOP
#define DEBUG

/**
 * ./project2 num_threads num_turns num_tests workload cs_workload randomness
 */
int main(int argc, char *argv[]){
	double bm_runtime = omp_get_wtime();

	////--------------- Test switches ---------------////
	bool test_mutex_switch = true;
	bool test_fcfs_switch = true;
	bool test_lru_switch = true;
	//bool test_bt_switch = true; // Compiler says unused
	//bool comp_shared_counter_switch = false;
	//bool anc_switch = false; // average number of contenders (anc)
	bool thr_switch = true; // throughput
	bool thr_comp = true; // determine anc when measuring throughput

	////--------------- CL Input ---------------////
	int num_threads = convertTo<int>(1, 4, argc, argv);
	// how many times does every thread need to pass through critical section
	int num_turns = convertTo<int>(2, 1, argc, argv);
	int num_tests = convertTo<int>(3, 30, argc, argv);
	int workload = convertTo<int>(4, 40000, argc, argv);
	int cs_workload = convertTo<int>(5, 10000, argc, argv);
	double randomness = convertTo<int>(6, 0.5, argc, argv);
	
// possible events: 1: begin doorway
//					2: finish doorway
//					3: acquire lock
//					4: unlock
	int num_events = num_threads * num_turns * 4;

	////--------------- Locks ---------------////
	//// --- Lamport
	//Lamport_Lecture my_lock {num_threads};
	//Lamport_Lecture_fix my_lock{ num_threads };
	//Lamport_Lecture_atomic my_lock{ num_threads };
	//Lamport_Original my_lock{ num_threads };

	//// --- Taubenfeld
	//Taubenfeld my_lock{num_threads};
	//Taubenfeld_fix my_lock{num_threads};
	//Taubenfeld_atomic my_lock{num_threads};

	//// --- Aravind
	//Aravind my_lock{ num_threads };
	Aravind_fix my_lock{ num_threads };

	//// --- Jayanti
	//Jayanti my_lock{ num_threads };
	//Jayanti_BT my_lock{ num_threads };

	// C++ Reference Lock	
	//printf("calling my_test()\n");
	//my_lock.my_test();

	////--------------- RESULTS ---------------////
	int mutex_fail_count, fcfs_fail_count, lru_fail_count;
	double average_time_el1, average_time_el2; //, anc;
	// throughput with det_anc
	double result_thp[num_tests][3] = { -1 };
	//struct bm_results results[num_tests]{ {my_lock.name, num_threads,num_turns, num_tests, num_events} };
	std::vector<struct bm_results> results(num_tests, {my_lock.name, num_threads,num_turns, num_tests, num_events});
	double* time_el1;
	double* time_el2;


	// Quick print to console that shows the configuration of the benchmark
#ifdef DESKTOP
	printf("\nTesting lock 2: %s\n", my_lock.name.c_str());
	printf("Performing mutex test: %d\n", test_mutex_switch);
	printf("Performing FCFS test: %d\n", test_fcfs_switch);
	printf("num_threads = %d\n", num_threads);
	printf("num_turns = %d\n", num_turns);
	printf("num_tests = %d\n", num_tests);
	printf("num_events = %d\n", num_events);
#endif

	////--------------- TESTS ---------------////
	mutex_fail_count = -1;
	if (test_mutex_switch) {
#ifdef DESKTOP
		printf("\n######################\n");
		printf("#     MUTEX TEST     #");
		printf("\n######################\n");
#endif
		for (int i = 0; i < num_tests; i++) {
			mutex_fail_count = test_mutex(&my_lock,
										num_threads,
										num_turns,
										workload, 
										cs_workload,
										randomness
										);
			results[i].mutex_fail_count = mutex_fail_count;
		}
#ifdef DESKTOP
		printf("\nPassed: %i\n", !(bool)mutex_fail_count);
#endif
	}

	fcfs_fail_count = -1;
	if (test_fcfs_switch) {
#ifdef DESKTOP
		printf("\n----------------------\n");
		printf("\n######################\n");
		printf("#      FCFS TEST     #");
		printf("\n######################\n");
#endif
		for (int i = 0; i < num_tests; i++) {
			fcfs_fail_count = test_fcfs(&my_lock,
									num_threads,
									num_turns,
									workload, 
									cs_workload,
									randomness
									);
			results[i].fcfs_fail_count = fcfs_fail_count;
		}
#ifdef DESKTOP
		printf("\nPassed: %i\n", !(bool)fcfs_fail_count);
#endif
	}

	lru_fail_count = -1;
	if (test_lru_switch) {
#ifdef DESKTOP
		printf("\n----------------------\n");
		printf("\n######################\n");
		printf("#       LRU TEST     #");
		printf("\n######################\n");
#endif
		for (int i = 0; i < num_tests; i++) {
			lru_fail_count = test_lru(&my_lock,
									num_threads,
									num_turns,
									workload,
									cs_workload,
									randomness
									);
			results[i].lru_fail_count = lru_fail_count;
		}
#ifdef DESKTOP
		printf("\nPassed: %i\n", !(bool)lru_fail_count);
#endif
	}

	if (thr_comp) {
#ifdef DESKTOP
		printf("\n######################\n");
		printf("#     THROUGHPUT     #");
		printf("\n######################\n");
#endif
		// runtime with dedicated throughput function
		printf("\ndesignated throughput test\n");
		printf("---------------------------------------------------\n");
		time_el1 = new double[num_tests]{0};
		for (int i = 0; i < num_tests; i++) {
			throughput(&my_lock,
					&result_thp[i][0],
					num_threads,
					num_turns,
					workload,
					cs_workload,
					randomness,
					true);
			time_el1[i] = *result_thp[0];
			results[i].thp_runtime_wanc = *result_thp[0];
			results[i].thp_wanc = *result_thp[1];
			results[i].anc = *result_thp[2];
		}
#ifdef DESKTOP
		printf("runtime (s) = %.4f\n", *result_thp[0]);
		printf("throughput (acq/s) = %.4f\n", *result_thp[1]);
		printf("average number of \"other\" contenders (#thr) = %.4f\n", 
				*result_thp[2]);
		printf("average number of contenders (#thr) = %.4f\n", *result_thp[2]+1);
#endif
		average_time_el1 = array_average(time_el1, num_tests);
#ifdef DESKTOP
 		printf("> Average time elapsed = %f s\n", average_time_el1);
#endif
		if (thr_switch) {
#ifdef DESKTOP
			// runtime with record_event_log without logging
			printf("\ntime measurement from record_event_log (no logging)\n");
			printf("---------------------------------------------------\n");
#endif
			int* event_log2 = new int[num_events * 2];
			time_el2 = new double[num_tests];
			for (int i = 0; i < num_tests; i++) {
				// time_el2[i] = record_event_log(event_log2,
				// 	&my_lock,
				// 	num_threads,
				// 	num_turns,
				// 	workload,
				// 	cs_workload,
				// 	randomness,
				// 	0);
				throughput(&my_lock,
						&result_thp[i][0],
						num_threads,
						num_turns,
						workload,
						cs_workload,
						randomness,
						false);
				
				time_el1[i] = *result_thp[0];
				results[i].thp_runtime_ref = *result_thp[0];
				results[i].thp_ref = *result_thp[1];
// #ifdef DESKTOP
// 			printf("time elapsed in seconds = %.5f\n", time_el2[i]);
// #endif
			}
			average_time_el2 = array_average(time_el2, num_tests);
#ifdef DESKTOP
			printf("> Average time elapsed = %f s\n", average_time_el2);
#endif
			delete[] event_log2;
		}
	}

#ifdef DESKTOP
	//log_results()
	// Everything below will eventually be moved into log_results(...), see toolbox
	printf("\n----------------------\n");
	printf("\n######################\n");
	printf("#       RESUMÉ       #");
	printf("\n######################\n");
	printf("\nBenchmark parameters:\n");
	printf("Lock name (attribute): %s\n", my_lock.name.c_str());
	printf("num_threads = %d\n", num_threads);
	printf("num_turns = %d\n", num_turns);
	printf("num_tests = %d\n", num_tests);
	printf("num_events = %d\n", num_events);

	printf("\nBenchmark results:\n");
	printf("mutex_fail_count = %d\n", mutex_fail_count);
	printf("fcfs_fail_count = %d\n", fcfs_fail_count);
	printf("lru_fail_count = %d\n", lru_fail_count);
	//if (anc_switch) printf("anc (anc test): %f\n", anc);
	printf("runtime (s) = %.4f\n", *result_thp[0]);
	printf("throughput (acq/s) = %.4f\n", *result_thp[1]);
	printf("average number of \"other\" contenders (#thr) = %.4f\n", 
			*result_thp[2]);
	printf("average number of contenders (#thr) = %.4f\n", *result_thp[2]+1);
	bm_runtime = omp_get_wtime() - bm_runtime;
	int mins, secs, milisecs;
	seconds_to_m_s_ms(bm_runtime, mins, secs, milisecs);
	printf("Total benchmark runtime (min:sec:msecs) = %d:%d:%d\n", mins, secs, milisecs);
	printf("Total benchmark runtime (sec) = %.2f\n", bm_runtime);

	printf("\n\nGarbage collection...\n");
#endif

	std::string filepath = "results/results_" + my_lock.name + "_" + std::to_string(num_threads) + ".csv";
	for (int i = 0; i < num_tests; i++) {
		results[i].bm_runtime = bm_runtime;
		std::ios_base::openmode flag;
		if (i==0) flag = std::fstream::trunc;
		else flag = std::fstream::app;
		log_results(results[i], filepath, flag);
	}

	if (thr_switch)
	{
		delete[] time_el1;
		delete[] time_el2;
	}
	return 0;	        
}

//_79_column_check_line:#######################################################

// Unused stuff --> delete
	// determine average number of contenders
// 	if (anc_switch) {
// #ifdef DESKTOP
// 		printf("\n######################\n");
// 		printf("#         ANC        #");
// 		printf("\n######################\n");
// #endif
// 		int* event_log = new int[num_events * 2];
// 		record_event_log(event_log,
// 							&my_lock,
// 							num_threads,
// 							num_turns,
// 							workload,
// 							cs_workload,
// 							randomness
// 							);
// 		anc = avg_num_contenders(event_log, num_threads, num_turns);
// #ifdef DESKTOP
// 		printf("average number of contenders = %.4f\n\n", anc);
// #endif
// 	}


// From throughput
// #ifdef DEBUG
// 		printf("will now run throughput\n");
// #endif // DEBUG

// 		throughput(&my_lock,
// 					&result_thp[0],
// 					num_threads,
// 					num_turns,
// 					workload,
// 					cs_workload,
// 					randomness,
// 					det_anc);
// #ifdef DESKTOP
// 		printf("runtime (s) = %.4f\n",result_thp[0]);
// 		printf("throughput (acq/s) = %.4f\n", result_thp[1]);
// 		printf("average number of \"other\" contenders (#thr) = %.4f\n", 
// 				result_thp[2]);
// 		printf("average number of contenders (#thr) = %.4f\n", result_thp[2]+1);
// #endif



// 	// test shared counter
// 	if (comp_shared_counter_switch) {
// #ifdef DESKTOP
// 		printf("\n######################\n");
// 		printf("#    SHAR CNT CMP    #");
// 		printf("\n######################\n");
// 		// runtime with record_event_log without logging
// 		printf("\ntime measurement from record_event_log (no logging)\n");
// 		printf("---------------------------------------------------\n");
// #endif
// 		time_el_sha_cnt1 = new double[num_tests];
// 		int* event_log1 = new int[num_events * 2];
// 		for (int i = 0; i < num_tests; i++) {
// 			time_el_sha_cnt1[i] = record_event_log(event_log1,
// 				&my_lock,
// 				num_threads,
// 				num_turns,
// 				workload,
// 				cs_workload,
// 				randomness,
// 				2);
// #ifdef DESKTOP
// 			printf("time elapsed = %.5f s\n", time_el_sha_cnt1[i]);
// #endif
// 		}
// #ifdef DESKTOP
// 		average_time_el_sha_cnt1 = array_average(time_el_sha_cnt1, num_tests);
// 		printf("> Average time elapsed = %f s\n", average_time_el_sha_cnt1);


// 		// runtime with full record_event_log
// 		printf("\ntime measurement from record_event_log (full logging)\n");
// 		printf("---------------------------------------------------\n");
// #endif
// 		time_el_sha_cnt2 = new double[num_tests];
// 		int* event_log2 = new int[num_events * 2];
// 		for (int i = 0; i < num_tests; i++) {
// 			time_el_sha_cnt2[i] = record_event_log(event_log2,
// 				&my_lock,
// 				num_threads,
// 				num_turns,
// 				workload,
// 				cs_workload,
// 				randomness,
// 				0);
// #ifdef DESKTOP
// 			printf("time elapsed in seconds = %.5f\n", time_el_sha_cnt2[i]);
// #endif
// 		}
// #ifdef DESKTOP
// 		average_time_el_sha_cnt2 = array_average(time_el_sha_cnt2, num_tests);
// 		printf("> Average time elapsed = %.5f s\n", average_time_el_sha_cnt2);
// #endif
// 		delete[] event_log1;
// 		delete[] event_log2;		
// 	}