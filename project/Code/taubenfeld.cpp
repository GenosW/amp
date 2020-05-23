#include "locks.hpp"

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
private:
	volatile bool *color;
	volatile bool *choosing;
	BWTicket *tickets; // unbounded integer label
	int size;
	int fail;

public:
	Taubenfeld();
	Taubenfeld(int n)
	{
		//fail = crash;
		size = n;
		*color = false; // Starting value is arbitrary
		choosing = new bool[n];
		tickets = new BWTicket[n];

		for (int i = 0; i < n; i++)
		{
			choosing[i] = false;
			tickets[i].color = false;
			tickets[i].number = 0;
		}
	};

private:
	int take_ticket(int j)
	{
		tickets[j].color = *color;
		bool my_color = tickets[j].color;
		int new_number = 0;
		for (int i = 0; i < size; i++)
		{
			if (tickets[i].color == my_color)
			{
				new_number = std::max(new_number, tickets[i].number);
			}
		}
		return new_number + 1;
	}

public:
	void doorway()
	{
		int id = omp_get_thread_num();

#ifdef LOCK_MSG
		printf("thread %i TRIES to acquire the lock\n", id);
#endif

		choosing[id] = true;
		tickets[id].number = take_ticket(id); // Ticket is colored inside function
		choosing[id] = true;

#ifdef LOCK_MSG
		printf("(mycolor, number)[%i] = %i\n", id, tickets[id].color, tickets[id].number);
#endif
	}

	void wait()
	{
		int id = omp_get_thread_num();

		for (int i = 0; i < size; i++)
		{
			while (!choosing[i])
			{
			} // wait until it is done choosing
			if (tickets[i].color == tickets[id].color)
			{
				while (!(tickets[i].number == 0 || lex_geq(tickets[i].color, tickets[i].number, tickets[id].color, tickets[id].number) || tickets[i].color != tickets[id].color))
				{ /*wait*/
				}
			}
			else
			{
				while (!(tickets[i].number == 0 || tickets[id].color != *color || tickets[i].color == tickets[id].color))
				{ /*wait*/
				}
			}
		}
#ifdef LOCK_MSG
		printf("thread %i ACQUIRES the lock\n", id);
#endif
	}

	void lock()
	{
		int id = omp_get_thread_num();
		doorway();
		wait();
	}

	void unlock()
	{
		int id = omp_get_thread_num();
#ifdef LOCK_MSG
		printf("thread %i UNLOCKS\n", id);
#endif
		*color = !*color;		// Flip color
		tickets[id].number = 0; // Set ticket number to zero
	}
};