//_79_column_check_line:#######################################################
#pragma once
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <assert.h>
#include "locks.hpp"

void print_log(int* array, int cols, int rows);

template<typename T>
T convertTo(const int position, const T init, int argc, char *argv[]);

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
T convertTo(const int position, const T init, int argc, char *argv[]) {
  if (argc <= position) {
    std::cout
        << "Conversion of argument " << position
        << " failed, not enough parameters, using default parameter: "
        << init << std::endl;
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

void print_array(int *array, int cols, int rows)
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

struct bm_results
{
	string lock_name;
	int num_threads;
	int num_turns;
	int num_tests;
	int mutex_fail_count;
	int fcfs_fail_count;
	int lru_fail_count;
	double mutex_time;
	double fcfs_time;
	double lru_time;
	// With this, you can control the seperation char for the insertion operator <<
	string insertion_sep=";";

	friend std::ostream& operator <<(std::ostream& os, bm_results const& a)
    {
        return os << a.lock_name << a.insertion_sep
	        	  << a.num_threads << a.insertion_sep
	              << a.num_turns << a.insertion_sep
	              << a.num_tests << a.insertion_sep
	              << a.mutex_fail_count << a.insertion_sep
	              << a.fcfs_fail_count << a.insertion_sep
	              << a.lru_fail_count << a.insertion_sep
	              << a.mutex_time << a.insertion_sep
	              << a.fcfs_time << a.insertion_sep
	              << a.lru_time << a.insertion_sep;
    }
};

string log_results(bm_results results, string path)
{
	ofstream outfile;
	// Open file for writing and appending
	outfile.open(path, std::fstream::out | std::fstream::app);
	// outfile has format:
	// string lock_name; num_threads; num_turns; num_tests; mutex_fail_coun; 
	// fcfs_fail_count; lru_fail_count; mutex_time; fcfs_time; lru_time;
	outfile << results << endl;
	
	outfile.close();
	return path;
};

//_79_column_check_line:#######################################################
