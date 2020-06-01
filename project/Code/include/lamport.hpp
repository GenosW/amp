#pragma once
#include "locks.hpp"

//#define LOCK_MSG

class Lamport_Lecture : public DW_Lock
{
	// bakery class according to lecture notes

protected: // protected = private but sub-classes can access
	bool *flag;
	int *label; // unbounded integer label
	int size;

public:
	Lamport_Lecture(int n);

	// make that one virtual so a subclass can
	// override it with its own function
private:
	virtual int draw_ticket();

protected:
	bool keep_waiting(int id);

public:
	void doorway();

public:
	void wait();

public:
	void lock();

public:
	void unlock();
};

class Lamport_Lecture_fix : public Lamport_Lecture
{
	// bakery class according to lecture notes (fixed)
	// this uses an atomic integer register for keeping track of
	// the latest drawn ticket. like this mutual exclusion holds.
private:
	std::atomic<int> latest_ticket;
	// normal int would not be good enough
	//int latest_ticket;

public:
	Lamport_Lecture_fix(int n);

private:
	virtual int draw_ticket() override;
};

class Lamport_Original : public DW_Lock
{
	// bakery class according to Lamport 1974
private:
	bool *gettoken;
	int *token; // unbounded integer label
	int size;

public:
	Lamport_Original(int n);

private:
	int take_ticket();

public:
	void wait();

public:
	void doorway();

public:
	void lock();

public:
	void unlock();
};
