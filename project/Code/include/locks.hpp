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
	virtual void lock() = 0;
	virtual void unlock() = 0;
};

class DW_Lock : public Lock
{
	// basic lock interface for locks, that have a doorway
public:
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
	Reference_Lock();
	void doorway();
	void wait();
	void lock();
	void unlock();
};
