//_79_column_check_line:#######################################################
#pragma once
#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iterator>
#include <numeric>
#include <string>
#include <tuple>
#include "locks.hpp"

void print_log(int* array, int cols, int rows);

template<typename T>
T convertTo(const int position, const T init, int argc, char *argv[], int debug=0);

template <typename myT>
bool isequal(myT *array1, myT *array2, int length);

void print_array(int *array, int cols, int rows);

// Lexicographic comparisions

template <typename myT>
bool lex_lesser_than(myT *a, myT *b, int size);

template <typename myT, typename myT2>
bool lex_lesser_than2(myT a1, myT2 a2, myT b1, myT2 b2);

/** (a1, a2, a3) >= (b1, b2, b3)
 * 
 * lexicographic comparison for 2 tuples 
 */
template <typename myT, typename myT2, typename myT3>
bool lex_geq(myT a1, myT2 a2, myT3 a3, myT b1, myT2 b2, myT3 b3);

/* Implementation */

void print_log(int* array, int cols, int rows) {
	// prints a lock log to the console
	// cols/rows ... number of columns/rows
	for (int j = 0; j != rows; ++j)
	{
		for (int i = 0; i < cols; i += 2)
		{
			printf("%d,", array[i + cols * j]);
			if ((array[i + 1 + cols * j]) == 1)	printf("b, ");
			if ((array[i + 1 + cols * j]) == 2)	printf("f, ");
			if ((array[i + 1 + cols * j]) == 3)	printf("a, ");
			if ((array[i + 1 + cols * j]) == 4)	printf("u, ");
		}
		printf("\n");
	}
}

template<typename T>
T convertTo(const int position, const T init, int argc, char *argv[], int debug) {
  if (argc <= position) 
	{
	if (debug)
		{
		std::cout
			<< "Conversion of argument " << position
			<< " failed, not enough parameters, using default parameter: "
			<< init << std::endl;
		}
	return init;
	}
	T arg;
	std::istringstream tmp(argv[position]);
	tmp >> arg;
	// tmp >> arg ?  (std::cout << "Conversion of argument " << position << "  successfull: " << arg)
	//               : (std::cout << "Conversion of argument " << position
	//                            << "  failed");

	return arg;
}

template <typename myT>
bool isequal(myT *array1, myT *array2, int length)
{
	// checks if two arrays are the same (pointwise)
	for (int i = 0; i < length; i++)
	{
		if (array1[i] != array2[i])
			return false;
	}
	return true;
}

// For some reason type-templating didn't work here...
void print_array(int* array, int cols, int rows)
{
	// prints an [array] to the console
	// cols/rows ... number of columns/rows
	for (int j = 0; j != rows; ++j)
	{
		for (int i = 0; i != cols; ++i)
		{
			printf("%d,", array[i + cols * j]);
		}
		printf("\n");
	}
}
// For some reason type-templating didn't work here...
void print_array(double* array, int cols, int rows)
{
	// prints an [array] to the console
	// cols/rows ... number of columns/rows
	for (int j = 0; j != rows; ++j)
	{
		for (int i = 0; i != cols; ++i)
		{
			printf("%f,", array[i + cols * j]);
		}
		printf("\n");
	}
}

template <typename T>
double array_average(T *array, int num_elements){
	assert(num_elements > 0);
	double sum = std::accumulate(array, (array + num_elements), 0.);
	//printf("Sum from array_average: %f\n", sum);
	return sum/double(num_elements);
}

template <typename myT>
bool lex_lesser_than(myT *a, myT *b, int size)
{
	// lexicographic comparison for 2 arrays
	for (int i = 0; i < size; i++)
	{
		if (a[i] > b[i])
			return false;
		if (a[i] < b[i])
			return true;
	}
	return false;
}

template <typename myT, typename myT2>
bool lex_lesser_than2(myT a1, myT2 a2, myT b1, myT2 b2)
{
	/* (a1, a2) < (b1, b2)
	
	lexicographic comparison for 4 elements */
	if (a1 > b1)
		return false;
	if (a1 < b1)
		return true;
	if (a2 > b2)
		return false;
	if (a2 < b2)
		return true;

	return false;
}


/** (a1, a2, a3) > (b1, b2, b3)
 * 
 * lexicographic comparison for 2 tuples 
 */
template <typename myT, typename myT2, typename myT3>
bool lex_greater(myT a1, myT2 a2, myT3 a3, myT b1, myT2 b2, myT3 b3)
{
	if (a1 > b1)
		return true;
	if (a1 < b1)
		return false;
	// If here: a1 == b1
	if (a2 > b2)
		return true;
	if (a2 < b2)
		return false;
	// If here: (a1, a2) == (b1, b2)
	if (a3 >= b3)
		return true;
	return false;
}

/** (a1, a2, a3) >= (b1, b2, b3)
 * 
 * lexicographic comparison for 2 tuples 
 */
template <typename myT, typename myT2, typename myT3>
bool lex_geq(myT a1, myT2 a2, myT3 a3, myT b1, myT2 b2, myT3 b3)
{
	if (lex_greater(a1, a2, a3, b1, b2, b3))
		return true;
	// If here: (a1, a2) == (b1, b2), (a3 >= b3)
	if (a3 == b3)
		return true;
	// If here: a3 > b3
	return false;
}

/** (a1, a2, a3) < (b1, b2, b3)
 * 
 * lexicographic comparison for 2 tuples 
 */
template <typename myT, typename myT2, typename myT3>
bool lex_less(myT a1, myT2 a2, myT3 a3, myT b1, myT2 b2, myT3 b3)
{
	if (a1 < b1)
		return true;
	if (a1 > b1)
		return false;
	// If here: a1 == b1
	if (a2 < b2)
		return true;
	if (a2 > b2)
		return false;
	// If here: a2 == b2
	if (a3 < b3)
		return true;
	return false;
}

/** (a1, a2, a3) <= (b1, b2, b3)
 * 
 * lexicographic comparison for 2 tuples 
 */
template <typename myT, typename myT2, typename myT3>
bool lex_leq(myT a1, myT2 a2, myT3 a3, myT b1, myT2 b2, myT3 b3)
{
	if (lex_less(a1, a2, a3, b1, b2, b3))
		return true;
	// If here: (a1, a2) == (b1, b2), (a3 >= b3)
	if (a3 == b3)
		return true;
	// If here: a3 > b3
	return false;
}

void seconds_to_m_s_ms(double time, int& mins, int& secs, int& milisecs)
{
	mins = int(time/60);
	secs = int((time/60.0 - double(mins))*60.0);
	milisecs = int(((time/60.0 - double(mins))*60.0 - secs)*1000);
}

struct bm_results
{
	string lock_name;
	int num_threads;
	int num_turns;
	int num_tests;
	int num_events;
	int workload = -1;
	int cs_workload = -1;
	double randomness = -1;
	int mutex_fail_count;
	int fcfs_fail_count;
	int lru_fail_count;
	double thp_runtime_wanc;
	double thp_wanc;
	double anc;
	double thp_runtime_ref;
	double thp_ref;
	double bm_runtime;
	// With this, you can control the seperation char for the insertion operator <<
	std::string insertion_sep=";";

	bm_results(std::string name, int th, int turns, int tests, int events, int work, int cs_work, double rand) : lock_name(name), num_threads(th), num_turns(turns), num_tests(tests), num_events(events), workload(work), cs_workload(cs_work), randomness(rand)
	{
		mutex_fail_count = -1;
		fcfs_fail_count = -1;
		lru_fail_count = -1;
		thp_runtime_wanc = -1;
		thp_wanc = -1;
		anc = -1;
		thp_runtime_ref = -1;
		thp_ref = -1;
		bm_runtime = -1;
	};

	std::string getHeader()
	{
		std::string sep = insertion_sep;
		std::string header = "lock_name";
		header += sep+"num_threads";
		header += sep+"num_turns";
		header += sep+"num_tests";
		header += sep+"num_events";
		header += sep+"workload";
		header += sep+"cs_workload";
		header += sep+"randomness"; 
		header += sep+"mutex_fail_count";
		header += sep+"fcfs_fail_count";
		header += sep+"lru_fail_count";
		header += sep+"thp_runtime_wanc";
		header += sep+"thp_wanc";
		header += sep+"anc";
		header += sep+"thp_runtime_ref";
		header += sep+"thp_ref";
		header += sep+"bm_runtime";
		return header;
	};

	friend std::ostream& operator <<(std::ostream& os, bm_results const& a)
    {
		//lock_name;num_threads;num_turns;num_tests;num_events;mutex_fail_count;fcfs_fail_count;
		//lru_fail_count;thp_runtime_wanc;thp_wanc;anc;thp_runtime_ref;thp_ref;bm_runtime

        return os << a.lock_name << a.insertion_sep
	        	  << a.num_threads << a.insertion_sep
	              << a.num_turns << a.insertion_sep
	              << a.num_tests << a.insertion_sep
				  << a.num_events << a.insertion_sep
				  << a.workload << a.insertion_sep
				  << a.cs_workload << a.insertion_sep
				  << a.randomness << a.insertion_sep
	              << a.mutex_fail_count << a.insertion_sep
	              << a.fcfs_fail_count << a.insertion_sep
	              << a.lru_fail_count << a.insertion_sep
	              << a.thp_runtime_wanc << a.insertion_sep
	              << a.thp_wanc << a.insertion_sep
				  << a.anc << a.insertion_sep
	              << a.thp_runtime_ref << a.insertion_sep
	              << a.thp_ref << a.insertion_sep
				  << a.bm_runtime;
    };
};

string log_results(bm_results results, std::string path, std::ios_base::openmode flag=std::fstream::app)
{
	ofstream outfile;
	// Open file for writing and appending
	outfile.open(path, std::fstream::out | flag);
	if (flag==std::fstream::trunc)
	{
		//write header
		// std::string sep = results.insertion_sep;
		// std::string header = "lock_name";
		// header += sep+"num_threads";
		// header += sep+"num_turns";
		// header += sep+"num_tests";
		// header += sep+"num_events"; 
		// header += sep+"mutex_fail_count";
		// header += sep+"fcfs_fail_count";
		// header += sep+"lru_fail_count";
		// header += sep+"thp_runtime_wanc";
		// header += sep+"thp_wanc";
		// header += sep+"anc";
		// header += sep+"thp_runtime_ref";
		// header += sep+"thp_ref";
		// header += sep+"bm_runtime";
		std::string header = results.getHeader();
		outfile << header << endl;
	}
	// outfile has format:
	// string lock_name; num_threads; num_turns; num_tests; num_events; mutex_fail_coun; 
	// fcfs_fail_count; lru_fail_count; anc; thp_runtime; thp; thp_anc; bm_runtime
	outfile << results << endl;
	
	outfile.close();
	return path;
};


//_79_column_check_line:#######################################################
