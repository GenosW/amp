#pragma once
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
