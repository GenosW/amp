//_79_column_check_line:#######################################################
#pragma once
#include <stdio.h>
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


/** (a1, a2, a3) >= (b1, b2, b3)
 * 
 * lexicographic comparison for 2 tuples 
 */
template <typename myT, typename myT2, typename myT3>
bool lex_geq(myT a1, myT2 a2, myT3 a3, myT b1, myT2 b2, myT3 b3)
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

//_79_column_check_line:#######################################################
