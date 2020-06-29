#pragma once
#include "locks.hpp"
#include "toolbox.hpp"
#include <vector>
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
	volatile bool *color;
	volatile bool *choosing;
	volatile BWTicket *tickets; // unbounded integer label
	volatile bool inCS;
	int size;
	//int fail;

public:
	string name = "Taubenfeld_Paper_1";

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
#ifdef DESKTOP
		printf("Deleting %s...\n", name.c_str());
#endif
		delete[] choosing;
		delete[] tickets;
	};

private:
	virtual void take_ticket(int id)
	{
		tickets[id].color = *color;
		volatile int new_number = 0;
		for (int i = 0; i < size; i++)
		{
			if (tickets[i].color == tickets[id].color)
			{
				new_number = std::max(new_number, tickets[i].number);
			}
		}
		tickets[id].number = new_number + 1;
	}

	virtual void reset_ticket(int id)
	{
		tickets[id].number = 0; // Set ticket number to zero
	}

	bool keep_waiting(int id)
	{
		for (int j = 0; j < size; j++)
		{
			//while (choosing[j]) {}
			// if not choosing and tickets.number_j < tickets.number_id									|| !gettickets.number[j]
			if (j!=id)
			{
				if (tickets[id].color==tickets[j].color )
				{
					if ((lex_lesser_than2(tickets[j].number, j, tickets[id].number, id) && tickets[j].number!=0) || choosing[j]) //(lex_less(tickets[j].color, tickets[j].number, j, tickets[id].color, tickets[id].number, id) || choosing[j]))
					{
						return true;
					}
				}
				else // color is different --> here it matters who has same color as *color
				{
					if (tickets[j].color!=*color || choosing[j])
					{
						return true;
					}
				}
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
		// while (keep_waiting(id))
		// {
		// }
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
public:
	string name = "Taubenfeld_Paper_1_Fix";

private:
	std::atomic<int> latest_ticket_black; // false=black
	std::atomic<int> latest_ticket_white; // true=white

public:
	Taubenfeld_fix(int n) : Taubenfeld{n, "Taubenfix"}
	{
		latest_ticket_black = 0;
		latest_ticket_white = 0;
	}
	~Taubenfeld_fix()
	{
#ifdef DESKTOP
		printf("Deleting %s...\n", name.c_str());
#endif
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

struct BWTicket_atomic
{
	
	std::atomic <int> number;
	std::atomic <bool> color;

	BWTicket_atomic()
	{
		number = 0;
		color = false;
	};
	BWTicket_atomic(int n, bool c)
	{
		number = n;
		color = c;
	};
};

class Taubenfeld_atomic : public DW_Lock
{
public:
	std::string name = "Taubenfeld_Paper_1_atomic";

protected:
	std::atomic <bool> lock_color;
	std::atomic <bool> *choosing;
	BWTicket_atomic *tickets;
	int size;

public:
	Taubenfeld_atomic(int n)
	{
		size = n;
		tickets = new BWTicket_atomic[n];
		choosing = new std::atomic <bool>[n];
		lock_color = true;

		for (int i = 0; i < n; i++)
		{
			choosing[i] = false;
			tickets[i].number = 0;
			tickets[i].color = false;
		}
	};
	~Taubenfeld_atomic()
	{
#ifdef DESKTOP
		printf("Deleting %s...\n", name.c_str());
#endif
		delete[] choosing;
		delete[] tickets;
	};

private:
	void take_ticket(int id)
	{
		tickets[id].color = lock_color.load();
		volatile int new_number = 0;
		for (int i = 0; i < size; i++)
		{
			if (tickets[i].color == tickets[id].color)
			{
				volatile int dummy = tickets[i].number;
				new_number = std::max(new_number, dummy);
			}
		}
		tickets[id].number = new_number + 1;
	};

	virtual void reset_ticket(int id)
	{
		tickets[id].number = 0; // Set ticket number to zero
	};

	bool keep_waiting(int id)
	{
		for (int j = 0; j < size; j++)
		{
			//while (choosing[j]) {}
			// if not choosing and tickets.number_j < tickets.number_id									|| !gettickets.number[j]
			if (j!=id)
			{
				if (tickets[id].color==tickets[j].color )
				{
					if ((lex_lesser_than2(tickets[j].number.load(), j, tickets[id].number.load(), id) && tickets[j].number!=0) || choosing[j]) //(lex_less(tickets[j].color, tickets[j].number, j, tickets[id].color, tickets[id].number, id) || choosing[j]))
					{
						return true;
					}
				}
				else // color is different --> here it matters who has same color as *color
				{
					if ((tickets[j].color!=lock_color.load() && tickets[j].number!=0) || choosing[j])
					{
						return true;
					}
				}
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

		choosing[id] = true;
		take_ticket(id); // Ticket is colored inside function
		choosing[id] = false;

#ifdef LOCK_MSG
		printf("(mycolor, number)[%i] = (%i, %i)\n", id, tickets[id].color.load(), tickets[id].number.load());
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
					while (!(tickets[j].number == 0 || lex_geq(tickets[j].color.load(), tickets[j].number.load(), j, tickets[id].color.load(), tickets[id].number.load(), id) || tickets[j].color != tickets[id].color))
					{ /*wait*/
					}
				}
				else
				{
					while (!(tickets[j].number == 0 || tickets[id].color != lock_color.load() || tickets[j].color == tickets[id].color))
					{ /*wait*/
					}
				}
			}
		}
		// while (keep_waiting(id))
		// {
		// }
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
		if (tickets[id].color == false)
			lock_color = true;
		else
			lock_color = false;
		reset_ticket(id);
	}
};

class Taubenfeld_adaptive : public DW_Lock
{
protected:
	std::atomic <bool> lock_color;
	std::atomic <bool> *choosing;
	BWTicket_atomic *tickets;
	volatile bool inCS;
	int size;
	std::vector<bool> S;

public:
	string name = "Taubenfeld_adaptive";

public:
	Taubenfeld_adaptive(int n)
	{
		//fail = crash;
		size = n;
		inCS = false;
		tickets = new BWTicket_atomic[n];
		choosing = new std::atomic <bool>[n];
		lock_color = true;
		S.assign(n, false);

		for (int i = 0; i < n; i++)
		{
			choosing[i] = false;
			tickets[i].color = false;
			tickets[i].number = 0;
		}
	};
	~Taubenfeld_adaptive()
	{
#ifdef DESKTOP
		printf("Deleting %s...\n", name.c_str());
#endif
		delete[] choosing;
		delete[] tickets;
	};

private:
	virtual void take_ticket(int id, std::vector<int> localS)
	{
		tickets[id].color = lock_color.load();
		volatile int new_number = 0;
		for (auto i : localS)
		{
			if (tickets[i].color == tickets[id].color)
			{
				volatile int dummy = tickets[i].number;
				new_number = std::max(new_number, dummy);
			}
		}
		tickets[id].number = new_number + 1;
	}

	virtual void reset_ticket(int id)
	{
		tickets[id].number = 0; // Set ticket number to zero
	}

	bool keep_waiting(int id, std::vector<int> localS)
	{
		for (auto j : localS)
		{
			//while (choosing[j]) {}
			// if not choosing and tickets.number_j < tickets.number_id									|| !gettickets.number[j]
			if (tickets[id].color.load()==tickets[j].color.load() )
			{
				if ((lex_lesser_than2(tickets[j].number.load(), j, tickets[id].number.load(), id) && tickets[j].number.load()!=0) || choosing[j].load()) //(lex_less(tickets[j].color, tickets[j].number, j, tickets[id].color, tickets[id].number, id) || choosing[j]))
				{
					return true;
				}
			}
			else // color is different --> here it matters who has same color as lock_color
			{
				if (tickets[j].color.load() !=lock_color || choosing[j].load())
				{
					return true;
				}
			}
		}
		return false;
	}

	std::vector<int> getLocalSet(std::vector<bool>& Sview, int mytid)
	{
		int counter=0, tid=0;
		std::vector<int> Sret(Sview.size(), -1);
		for (auto i : Sview)
		{
			if (i && tid!=mytid) 
			{
				Sret[counter] = tid;
				counter++;
			}
			tid++;
		}
		Sret.resize(counter);
		return Sret;
	}

public:
	void doorway()
	{
		//printf("dw");
		int id = omp_get_thread_num();
		S[id] = true;

#ifdef LOCK_MSG
		printf("thread %i TRIES to acquire the lock\n", id);
#endif
		choosing[id] = true;
		std::vector<int> localS = getLocalSet(S, id);
		take_ticket(id, localS); // Ticket is colored inside function
		choosing[id] = false;

#ifdef LOCK_MSG
		printf("(mycolor, number)[%i] = (%i, %i)\n", id, tickets[id].color.load(), tickets[id].number.load());
#endif
	}

	void wait()
	{
		int id = omp_get_thread_num();
		std::vector<int> localS = getLocalSet(S, id);

		for (auto j : localS)
		{
#ifdef LOCK_MSG
		printf("thread %i checking CHOOSING[%i] the lock\n", id, j);
#endif
			while (choosing[j])
			{
			} // wait until it is done choosing
			if (tickets[j].color == tickets[id].color)
			{
#ifdef LOCK_MSG
		printf("thread %i checking j=%i (same color) the lock\n", id, j);
#endif
				// while (! (tickets[j].number == 0 || lex_geq(tickets[j].color, tickets[j].number, tickets[id].color, tickets[id].number) || tickets[j].color != tickets[id].color) )
				while (!(tickets[j].number == 0 || lex_geq(tickets[j].color.load(), tickets[j].number.load(), j, tickets[id].color.load(), tickets[id].number.load(), id) || tickets[j].color.load() != tickets[id].color.load()))
				{ /*wait*/
				}
			}
			else
			{
#ifdef LOCK_MSG
		printf("thread %i checking j=%i (different color) the lock\n", id, j);
#endif
				while (!(tickets[j].number.load() == 0 || tickets[id].color.load() != lock_color || tickets[j].color.load() == tickets[id].color.load()))
				{ /*wait*/
				}
			}
		}
		// while (keep_waiting(id))
		// {
		// }
		inCS = true;
#ifdef LOCK_MSG
		printf("thread %i ACQUIRES the lock\n", id);
#endif
	}

	void lock()
	{
		doorway();
		wait();
	}

	void unlock()
	{
		int id = omp_get_thread_num();
#ifdef LOCK_MSG
		printf("thread %i ACQUIRES the lock\n", id);
#endif
		if (tickets[id].color == false)
			lock_color = true;
		else
			lock_color = false;
		reset_ticket(id);
		S[id] = false;
		inCS = false;
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