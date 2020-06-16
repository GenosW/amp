//_79_column_check_line:#######################################################
#pragma once
#include "locks.hpp"
#include "toolbox.hpp"
#include <atomic>
//#define LOCK_MSG


/**
 * B-Bakery lock as developed by Jayanti et al
 * 
 * An implementation of the algorithm presented
 * in their paper "Bounding Lamport’s Bakery Algorithm", 2001,
 * on page 266 (?)/page 6 of shortened paper in Section4, Fig.2.
 * 
 * 
 */
class Jayanti_B : public DW_Lock
{
public:
	string name = "Jayanti_B";

protected: // protected = private but sub-classes can access
	volatile bool *gettoken;
	volatile int *token; // unbounded integer token
	int size;
	volatile int X;

public:
	Jayanti_B(int n)
	{
		size = n;
		gettoken = new bool[n];
		token = new int[n];
		X = 0;

		for (int i = 0; i < n; i++)
		{
			gettoken[i] = false;
			token[i] = -1;
		}
	};
	~Jayanti_B()
	{
		printf("Deleting %s...\n", name.c_str());
		delete[] gettoken;
		delete[] token;
	};

private:
	int draw_ticket()
	{
		// The main idea of Jayanti is:
		// The smallest possible value of new_ticket is always X
		volatile int new_ticket = 0;
		for (int i = 0; i < size; i++)
		{
			//dummy = token[i];
			if (token[i]!=-1)
			{
				new_ticket = max_mod(token[i], new_ticket, size);
			}
		}
		// Enforce new_ticket >= X
		new_ticket = add_mod(max_mod(X, new_ticket, size), 1, size); // line 4 in Fig.2
		return new_ticket;
	}

	template <typename T1, typename T2>
	int add_mod(T1 a, T2 b, int n)
	{
		return (a+b) % (2*n - 1);
	}

	template <typename T1, typename T2>
	int sub_mod(T1 a, T2 b, int n)
	{
		return (a-b) % (2*n - 1);
	}

	template <typename T1, typename T2>
	int max_mod(T1 a, T2 b, int n)
	{
		return sub_mod(std::max(add_mod(a, n-1-b, n), add_mod(b, n-1-b, n)), n-1-b, n);
	}

	template <typename T1, typename T2>
	int less_mod(T1 a1, T1 b1, T2 a2, T2 b2, int n)
	{
		return lex_lesser_than2(add_mod(a1, n-1-a1, n), b1, add_mod(a2, n-1-a1, n), b2);
	}

	template <typename T1, typename T2>
	int less_mod2(T1 a1, T1 b1, T2 a2, T2 b2, int n)
	{
		return lex_lesser_than2(add_mod(a1, n-1-a2, n), b1, add_mod(a2, n-1-a2, n), b2);
	}

protected:
	bool keep_waiting(int id)
	{
		for (int j = 0; j < size; j++)
		{
			while (gettoken[j]) {}
			// if not gettoken_j and token_j < token_id									|| !gettoken[j]
			if (j!=id && (less_mod(token[j], j, token[id], id, size) && token[j] > -1) && token[j]!=-1)
			{
				return true;
			}
		}
		return false;
	}

public:
	void doorway()
	{
		int id = omp_get_thread_num();
		doorway(id);
	}

public:
	void doorway(int id)
	{
#ifdef LOCK_MSG
		printf("thread %i TRIES to acquire the lock\n", id);
#endif
		//							Line number in Fig.2
		gettoken[id] = true;		// 1
		token[id] = draw_ticket();	// 2-4
		gettoken[id] = false;		// 5

#ifdef LOCK_MSG
		printf("gettoken[%i] = %i, token[%i] = %i\n", id, gettoken[id], id, token[id]);
#endif
	}

public:
	void wait()
	{
		int id = omp_get_thread_num();
		wait(id);
	}

public:
	void wait(int id)
	{
		while (keep_waiting(id))
		{
		}
#ifdef LOCK_MSG
		printf("thread %i ACQUIRES the lock\n", id);
		printf("thread %i SETS new X=%i\n", id, token[id]);
#endif	
		X = token[id];					// 8
	}


public:
	void lock()
	{
		int id = omp_get_thread_num();
		doorway(id);					// 1 - 5
		wait(id);						// 6 + 7
	}

public:
	void unlock()
	{
		int id = omp_get_thread_num();
#ifdef LOCK_MSG
		printf("thread %i UNLOCKS, X = %i\n", id, X);
#endif
		token[id] = -1;
	}
};

/**
 * B-Bakery lock as developed by Jayanti et al
 * 
 * An implementation of the algorithm presented
 * in their paper "Bounding Lamport’s Bakery Algorithm", 2001,
 * on page 266 (?)/page 6 of shortened paper in Section4, Fig.2.
 * 
 * 
 */
class Jayanti_UB : public DW_Lock
{
public:
	string name = "Jayanti_UB";

protected: // protected = private but sub-classes can access
	volatile bool *gettoken;
	volatile int *token; // unbounded integer token
	int size;
	volatile int X;

public:
	Jayanti_UB(int n)
	{
		size = n;
		gettoken = new bool[n];
		token = new int[n];
		X = 0;

		for (int i = 0; i < n; i++)
		{
			gettoken[i] = false;
			token[i] = -1;
		}
	};
	~Jayanti_UB()
	{
		printf("Deleting %s...\n", name.c_str());
		delete[] gettoken;
		delete[] token;
	};

private:
	int draw_ticket()
	{
		// The main idea of Jayanti is:
		// The smallest possible value of new_ticket is always X
		volatile int new_ticket = 0;
		for (int i = 0; i < size; i++)
		{
			//dummy = token[i];
			new_ticket = std::max(new_ticket, token[i]);
		}
		// Enforce new_ticket >= X
		new_ticket = std::max(new_ticket, X);
		return new_ticket + 1;
	}

protected:
	bool keep_waiting(int id)
	{
		for (int j = 0; j < size; j++)
		{
			while (gettoken[j]) {}
			// if not gettoken_j and token_j < token_id									|| !gettoken[j]
			if (j!=id && (lex_lesser_than2(token[j], j, token[id], id) && token[j] > -1)           )
			{
				return true;
			}
		}
		return false;
	}

public:
	void doorway()
	{
		int id = omp_get_thread_num();
		doorway(id);
	}

public:
	void doorway(int id)
	{
#ifdef LOCK_MSG
		printf("thread %i TRIES to acquire the lock\n", id);
#endif
		//							Line number in Fig.2
		gettoken[id] = true;		// 1
		token[id] = draw_ticket();	// 2-4
		gettoken[id] = false;		// 5

#ifdef LOCK_MSG
		printf("gettoken[%i] = %i, token[%i] = %i\n", id, gettoken[id], id, token[id]);
#endif
	}

public:
	void wait()
	{
		int id = omp_get_thread_num();
		wait(id);			// 8
	}

public:
	void wait(int id)
	{
		
		while (keep_waiting(id))
		{
		}
#ifdef LOCK_MSG
		printf("thread %i ACQUIRES the lock\n", id);
		printf("thread %i SETS new X=%i\n", id, token[id]);
#endif	
		X = token[id];					// 8
	}

public:
	void lock()
	{
		int id = omp_get_thread_num();
		doorway(id);					// 1 - 5
		wait(id);						// 6 + 7

	}

public:
	void unlock()
	{
		int id = omp_get_thread_num();
#ifdef LOCK_MSG
		printf("thread %i UNLOCKS, X = %i\n", id, X);
#endif
		token[id] = -1;
	}
};
