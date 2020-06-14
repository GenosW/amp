#include  <stdio.h>

template <typename myT>
bool isequal(myT* array1, myT* array2, int length){
	// checks if two arrays are the same (pointwise)
	for(int i = 0; i<length; i++){
		if (array1[i] != array2[i]) return false;
	}
	return true;
}


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



void print_array(int* array, int cols, int rows){
	// prints an [array] to the console
	// cols/rows ... number of columns/rows
	for (int j = 0; j != rows; ++j)
  {
    for (int i = 0; i != cols; ++i)
    {
			printf("%d,",array[i + cols * j]);
    }
    printf("\n");
  }
}

template <typename myT>
bool lex_lesser_than(myT* a, myT* b, int size){
	// lexicographic comparison for 2 arrays
	for(int i = 0; i < size; i++){
		if (a[i] > b[i]) return false;
		if (a[i] < b[i]) return true;
	}
	return false;
}

template <typename myT>
bool lex_lesser_than2(myT a1, myT a2, myT b1, myT b2){
	// lexicographic comparison for 4 elements
	if (a1 > b1) return false;
	if (a1 < b1) return true;
	if (a2 > b2) return false;
	if (a2 < b2) return true;

	return false;
}

////////////////////////////////////////////////////////////////////////////79
