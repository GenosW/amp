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
class Jayanti : public DW_Lock
{
public:
	string name = "Jayanti";

protected: // protected = private but sub-classes can access
	volatile bool *gettoken;
	volatile int *token; // unbounded integer token
	int size;
	volatile int X;

public:
	Jayanti(int n)
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
	}

private:
	virtual int draw_ticket()
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
		while (keep_waiting(id))
		{
		}
#ifdef LOCK_MSG
		printf("thread %i ACQUIRES the lock\n", id);
#endif
	}

public:
	void wait(int id)
	{
		
		while (keep_waiting(id))
		{
		}
#ifdef LOCK_MSG
		printf("thread %i ACQUIRES the lock\n", id);
#endif
	}


public:
	void lock()
	{
		int id = omp_get_thread_num();
		doorway(id);					// 1 - 5
		wait(id);						// 6 + 7
		X = token[id];					// 8
	}

public:
	void unlock()
	{
		int id = omp_get_thread_num();
#ifdef LOCK_MSG
		printf("thread %i UNLOCKS\n", id);
#endif
		token[id] = -1;
	}
};
