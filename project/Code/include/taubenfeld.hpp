#pragma once
#include "locks.hpp"
//#define LOCK_MSG

struct BWTicket
{
	bool color;
	int number;

	BWTicket(){};
};

/**
 * Black-White Bakery lock as developed by Taubenfeld, 2004.
 * 
 * A modified version of the original Bakery by Lamport.
 * The ticket (numbers) are bounded to *n* by using colors,
 * where *n* is the number of processes/threads.
 * A ticket has structure: (color, number)
 * color... black or white (=*bool*: false or true)
 * number... int in range [0, *n*]
 * 
 * Entry to CS is granted to lowest ticket, where
 * lowest is defined via the lexicographic ordering
 * (color_i, number_i) < (color_j, number_j).
 */
class Taubenfeld : public DW_Lock
{
protected:
	string name;
	volatile bool *color;
	volatile bool *choosing;
	volatile BWTicket *tickets; // unbounded integer label
	volatile bool inCS;
	int size;
	//int fail;

public:
	//Taubenfeld();
	Taubenfeld(int n);
	Taubenfeld(int n, string lock_name);
	~Taubenfeld();
	void doorway();
	void wait();
	void lock();
	void unlock();

private:
	virtual void take_ticket(int id);
	virtual void reset_ticket(int id);
};

class Taubenfeld_fix : public Taubenfeld
{
private:
	std::atomic<int> latest_ticket_black; // false=black
	std::atomic<int> latest_ticket_white; // true=white

public:
	Taubenfeld_fix(int n);

private:
	void take_ticket(int id);
	void reset_ticket(int id);
};