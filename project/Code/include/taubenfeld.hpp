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
	string name = "Taubenfeld";
	volatile bool *color;
	volatile bool *choosing;
	volatile BWTicket *tickets; // unbounded integer label
	volatile bool inCS;
	int size;
	//int fail;

public:
	//Taubenfeld();
	Taubenfeld(int n)
	{
		//fail = crash;
		size = n;
		inCS = false;
		color = new bool; // Starting value is arbitrary
		choosing = new bool[n];
		tickets = new BWTicket[n];

		*color = false;
		for (int i = 0; i < n; i++)
		{
			choosing[i] = false;
			tickets[i].color = false;
			tickets[i].number = 0;
		}
	};
	Taubenfeld(int n, string lock_name)
	{
		name = lock_name;
		//fail = crash;
		size = n;
		inCS = false;
		color = new bool; // Starting value is arbitrary
		choosing = new bool[n];
		tickets = new BWTicket[n];

		*color = false;
		for (int i = 0; i < n; i++)
		{
			choosing[i] = false;
			tickets[i].color = false;
			tickets[i].number = 0;
		}
	};
	~Taubenfeld()
	{
		printf("Deleting %s...\n", name.c_str());
		delete[] choosing;
		delete[] tickets;
	};

private:
	virtual void take_ticket(int id)
	{
		tickets[id].color = *color;
		int new_number = 0;
		for (int i = 0; i < size; i++)
		{
			if (tickets[i].color == tickets[id].color)
			{
				int ticket_number = tickets[i].number;
				new_number = std::max(new_number, ticket_number);
			}
		}
		tickets[id].number = new_number + 1;
	}

	virtual void reset_ticket(int id)
	{
		tickets[id].number = 0; // Set ticket number to zero
	}

public:
	void doorway()
	{
		int id = omp_get_thread_num();

#ifdef LOCK_MSG
		printf("thread %i TRIES to acquire the lock\n", id);
#endif

		choosing[id] = true;
		take_ticket(id); // Ticket is colored inside function
		choosing[id] = false;

#ifdef LOCK_MSG
		printf("(mycolor, number)[%i] = (%i, %i)\n", id, tickets[id].color, tickets[id].number);
#endif
	}

	void wait()
	{
		int id = omp_get_thread_num();

		for (int j = 0; j < size; j++)
		{
			if (j != id)
			{
				while (choosing[j])
				{
				} // wait until it is done choosing
				if (tickets[j].color == tickets[id].color)
				{
					// while (! (tickets[j].number == 0 || lex_geq(tickets[j].color, tickets[j].number, tickets[id].color, tickets[id].number) || tickets[j].color != tickets[id].color) )
					while (!(tickets[j].number == 0 || lex_geq(tickets[j].color, tickets[j].number, j, tickets[id].color, tickets[id].number, id) || tickets[j].color != tickets[id].color))
					{ /*wait*/
					}
				}
				else
				{
					while (!(tickets[j].number == 0 || tickets[id].color != *color || tickets[j].color == tickets[id].color))
					{ /*wait*/
					}
				}
			}
		}
		inCS = true;
		//#ifdef LOCK_MSG
		//printf("thread %i ACQUIRES the lock\n", id);
		//#endif
	}

	void lock()
	{
		doorway();
		wait();
	}

	void unlock()
	{
		int id = omp_get_thread_num();
		// #ifdef LOCK_MSG
		//printf("thread %i UNLOCKS\n", id);
		// #endif
		//*color = !*color;		// Flip color
		if (tickets[id].color == false)
			*color = true;
		else
			*color = false;
		reset_ticket(id);
		inCS = false;
	}
};

class Taubenfeld_fix : public Taubenfeld
{
private:
	std::atomic<int> latest_ticket_black; // false=black
	std::atomic<int> latest_ticket_white; // true=white

public:
	Taubenfeld_fix(int n) : Taubenfeld{n, "Taubenfix"}
	{
		latest_ticket_black = 0;
		latest_ticket_white = 0;
	}

private:
	void take_ticket(int id) override
	{
		tickets[id].color = *color;
		if (tickets[id].color)
		{
			tickets[id].number = ++latest_ticket_white;
		}
		else
		{
			tickets[id].number = ++latest_ticket_black;
		}
	}

	void reset_ticket(int id) override
	{
		if (tickets[id].number==size)
		{
			if (tickets[id].color)
			{
				latest_ticket_white = 0;
			}
			else
			{
				latest_ticket_black = 0;
			}
		}
		tickets[id].number = 0;
	}
};

// struct BWTicket
// {
// 	bool color;
// 	int number;

// 	BWTicket(){};
// };

// /**
//  * Black-White Bakery lock as developed by Taubenfeld, 2004.
//  * 
//  * A modified version of the original Bakery by Lamport.
//  * The ticket (numbers) are bounded to *n* by using colors,
//  * where *n* is the number of processes/threads.
//  * A ticket has structure: (color, number)
//  * color... black or white (=*bool*: false or true)
//  * number... int in range [0, *n*]
//  * 
//  * Entry to CS is granted to lowest ticket, where
//  * lowest is defined via the lexicographic ordering
//  * (color_i, number_i) < (color_j, number_j).
//  */
// class Taubenfeld : public DW_Lock
// {
// protected:
// 	string name;
// 	volatile bool *color;
// 	volatile bool *choosing;
// 	volatile BWTicket *tickets; // unbounded integer label
// 	volatile bool inCS;
// 	int size;
// 	//int fail;

// public:
// 	//Taubenfeld();
// 	Taubenfeld(int n);
// 	Taubenfeld(int n, string lock_name);
// 	~Taubenfeld();
// 	void doorway();
// 	void wait();
// 	void lock();
// 	void unlock();

// private:
// 	virtual void take_ticket(int id);
// 	virtual void reset_ticket(int id);
// };

// class Taubenfeld_fix : public Taubenfeld
// {
// private:
// 	std::atomic<int> latest_ticket_black; // false=black
// 	std::atomic<int> latest_ticket_white; // true=white

// public:
// 	Taubenfeld_fix(int n);

// private:
// 	void take_ticket(int id);
// 	void reset_ticket(int id);
// };