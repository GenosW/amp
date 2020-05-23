#include <stdio.h>

class BWTicket
{
public:
	bool color;
	int number;

	BWTicket();
	// constructor
	BWTicket(bool c, int num)
	{
		color = c;
		number = num;
	}

	// Member Functions()
	void printTicket()
	{
		printf("Ticket(%u, %u)\n", color, number);
	}
};

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

template <typename myT, typename myT2>
bool lex_geq(myT a1, myT2 a2, myT b1, myT2 b2)
{
	/* (a1, a2) >= (b1, b2)
	
	lexicographic comparison for 4 elements */
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
	return true;
}
