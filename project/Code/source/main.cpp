#include  <stdio.h>
#include <omp.h>
#include <math.h>
#include <time.h>
// compiler switch to turn some debug-messages on
//#define DEBUG
#include <assert.h>
#include "locks.hpp" // basic lock interface + reference lock implementations
#include "lamport.hpp" // implementation of lamport bakery locks
#include "taubenfeld.hpp" // implementation of taubenfeld (black/white) bakery locks
#include "tests.hpp" // lock tests (mutex, fcfs)
#include "toolbox.hpp" // various helper functions

/**
 * ./project2 num_threads num_turns num_tests 
 */
int main(int argc, char *argv[]){

	int num_threads = convertTo<int>(1, 4, argc, argv);
	// how many times does every thread need to pass through critical section
	int num_turns = convertTo<int>(2, 2, argc, argv);
	int num_tests = convertTo<int>(3, 10, argc, argv);

	// amount of work in the critical/noncritical section
	int workload = int(1e3);
	// how random the wl in the noncritical section should be
	double randomness = 0.5; 

	// i fooled around with different ways to instanciate locks
	//auto my_lock = new Lamport_Lecture(num_threads);
	//Lamport_Lecture* my_lock = new Lamport_Lecture{num_threads};
	//DW_Lock* my_lock = new Lamport_Lecture{num_threads};

	// The 2 strings hold information on which Lock is being tested
	string lock_class="<not-set>", lock_version="<not-set>";
	//// --- Lamport
	// lock_class = "Lamport";
	// Lamport_Lecture my_lock {num_threads}; lock_version = "Lecture";
	// Lamport_Lecture_fix my_lock{ num_threads }; lock_version = "Fix1";
	// Lamport_Original my_lock{ num_threads }; lock_version = "Original";

	//// --- Taubenfeld
	// lock_class = "Taubenfeld";
	// Taubenfeld my_lock{num_threads}; lock_version = "Paper v1";
	Taubenfeld_fix my_lock{num_threads}; lock_version = "Fix1";

	//// --- Szymansky
	//lock_class = "Szymansky";

	//// --- Aravind
	//lock_class = "Aravind";

	// C++ Reference Lock
	//lock_class = "Reference";
	//Reference_Lock my_lock; lock_version = "C++11"
	

	bool test_mutex_switch = true;
	bool test_fcfs_switch = true;

	// Quick print to console that shows the configuration of the benchmark
	// printf("\nTesting lock: %s %s\n", lock_class.c_str(), lock_version.c_str());
	printf("\nTesting lock 2: %s\n", my_lock.name.c_str());
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

	printf("\n######################\n");
	printf("#     MUTEX TEST     #");
	printf("\n######################\n");

	int mutex_fail_count = -1;
	if (test_mutex_switch) {
		mutex_fail_count = 0;
		for (int i = 0; i < num_tests; i++) {
			mutex_fail_count += (1 - test_mutex(&my_lock,
				num_threads,
				num_turns,
				workload,
				randomness,
				true)
				);
		}
	}

	printf("\n----------------------\n");
	printf("\n######################\n");
	printf("#     FCFS TEST      #");
	printf("\n######################\n");

	int fcfs_fail_count = -1;
	if (test_fcfs_switch) {
		fcfs_fail_count = 0;
		for (int i = 0; i < num_tests; i++) {
			fcfs_fail_count += (1 - test_fcfs(&my_lock,
				num_threads,
				num_turns,
				workload,
				randomness,
				true)
				);
		}
	}

	printf("\n----------------------\n");
	printf("\n######################\n");
	printf("#       RESUMÉ       #");
	printf("\n######################\n");
	//printf("Lock: %s %s\n", lock_class.c_str(), lock_version.c_str());
	printf("\nLock name (attribute): %s\n", my_lock.name.c_str());
	printf("mutex_fail_count = %d\n", mutex_fail_count);
	printf("fcfs_fail_count = %d\n", fcfs_fail_count);

	//test_random_workload(30,1e5,.9);

	// I coded the destructors to print a message when they are being called...
	// just to make sure that and where they are being called.
	// These messages should ONLY show after the following statement
	printf("\n\nGarbage collection...\n");

	return 0;	        
}

