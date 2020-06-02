#pragma once
#include <algorithm> // for the std::max() function
#include <omp.h>
#include <atomic> // for the reference lock
#include <string>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <tuple>

using namespace std;

// compiler switch that turns some debugging messages on/off
//#define LOCK_MSG

class Lock
{
	// basic lock interface
public:
	string name = "Lock";

	virtual void lock() = 0;
	virtual void unlock() = 0;
};

class DW_Lock : public Lock
{
	// basic lock interface for locks, that have a doorway
public:
	string name = "DW-Lock";

	virtual void doorway() = 0;
	virtual void wait() = 0;
	virtual void lock() = 0;
	virtual void unlock() = 0;
};

class Reference_Lock : public DW_Lock
{
	// the "native" c++ lock wrapped in our class
private:
	std::atomic_flag lock_stream = ATOMIC_FLAG_INIT;

public:
	string name = "Reference-Lock";

	Reference_Lock()
	{
		std::atomic_flag lock_stream = ATOMIC_FLAG_INIT;
	}

public:
	void doorway()
	{
#ifdef LOCK_MSG
		int id = omp_get_thread_num();
		printf("thread %i TRIES to acquire the lock\n", id);
#endif
	}

public:
	void wait()
	{
		while (lock_stream.test_and_set())
		{
		}
#ifdef LOCK_MSG
		int id = omp_get_thread_num();
		printf("thread %i ACQUIRES the lock\n", id);
#endif
	}

public:
	void lock()
	{
		doorway();
		wait();
	}

public:
	void unlock()
	{
#ifdef LOCK_MSG
		int id = omp_get_thread_num();
		printf("thread %i UNLOCKS\n", id);
#endif
		lock_stream.clear();
	}
};
