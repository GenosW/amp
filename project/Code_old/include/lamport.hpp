//_79_column_check_line:#######################################################
#pragma once
#include "locks.hpp"
#include "toolbox.hpp"
#include <atomic>
//#define LOCK_MSG


class Lamport_Lecture : public DW_Lock
{
	// bakery class according to lecture notes
public:
	string name = "Lamport_Lecture";

protected: // protected = private but sub-classes can access
	bool *flag;
	int *label; // unbounded integer label
	int size;

public:
	Lamport_Lecture(int n)
	{
		size = n;
		flag = new bool[n];
		label = new int[n];

		for (int i = 0; i < n; i++)
		{
			flag[i] = false;
			label[i] = 0;
		}
	}

	// make that one virtual so a subclass can
	// override it with its own function
private:
	virtual int draw_ticket()
	{
		int new_ticket = 0;
		for (int i = 0; i < size; i++)
		{
			new_ticket = std::max(new_ticket, label[i]);
		}
		return new_ticket + 1;
	}

protected:
	bool keep_waiting(int id)
	{
		for (int i = 0; i < size; i++)
		{
			if ((flag[i]) && (lex_lesser_than2(label[i], i, label[id], id)))
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

		flag[id] = true;
		label[id] = draw_ticket();

#ifdef LOCK_MSG
		printf("flag[%i] = %i, label[%i] = %i\n", id, flag[id], id, label[id]);
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
	void lock()
	{
		doorway();
		wait();
	}

public:
	void unlock()
	{
		int id = omp_get_thread_num();

#ifdef LOCK_MSG
		printf("thread %i UNLOCKS\n", id);
#endif

		flag[id] = false;
	}
};


class Lamport_Lecture_fix : public Lamport_Lecture
{
	// bakery class according to lecture notes (fixed)
	// this uses an atomic integer register for keeping track of
	// the latest drawn ticket. like this mutual exclusion holds.
public:
	string name = "Lamport_Lecture_fix";

private:
	std::atomic<int> latest_ticket;
	// normal int would not be good enough
	//int latest_ticket;

public:
	Lamport_Lecture_fix(int n) : Lamport_Lecture{n}
	{
		latest_ticket = 0;
	}

private:
	virtual int draw_ticket() override
	{
		return latest_ticket.fetch_add(1);
		//return ++latest_ticket;
	}
};

class Lamport_Original : public DW_Lock
{
	// bakery class according to Lamport 1974
public:
	string name = "Lamport_Original";

private:
	bool *gettoken;
	int *token; // unbounded integer label
	int size;

public:
	Lamport_Original(int n)
	{
		size = n;
		gettoken = new bool[n];
		token = new int[n];

		for (int i = 0; i < n; i++)
		{
			gettoken[i] = false;
			token[i] = -1;
		}
	}

private:
	int take_ticket()
	{
		int new_ticket = 0;
		for (int i = 0; i < size; i++)
		{
			new_ticket = std::max(new_ticket, token[i]);
		}
		return new_ticket + 1;
	}

public:
	void wait()
	{
		int id = omp_get_thread_num();
		for (int j = 0; j < size; j++)
		{
			if (j == id)
				j++;
			if (j == size)
				break;
			while (gettoken[j])
			{
			}
			while (not((token[j] == -1) || lex_lesser_than2(token[id], id, token[j], j)))
			{
			}
		}
#ifdef LOCK_MSG
		printf("thread %i ACQUIRES the lock\n", id);
#endif
	}

public:
	void doorway()
	{
		int id = omp_get_thread_num();

#ifdef LOCK_MSG
		printf("thread %i TRIES to acquire the lock\n", id);
#endif

		gettoken[id] = true;
		token[id] = take_ticket();
		gettoken[id] = false;

#ifdef LOCK_MSG
		printf("token[%i] = %i\n", id, token[id]);
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
		int id = omp_get_thread_num();
#ifdef LOCK_MSG
		printf("thread %i UNLOCKS\n", id);
#endif
		token[id] = -1;
	}
};


class Lamport_Lecture_atomic : public DW_Lock
{
	// bakery class according to lecture notes
public:
	string name = "Lamport_Lecture_atomic";

protected: // protected = private but sub-classes can access
	std::atomic <bool> *flag;
	std::atomic <int> *label; // unbounded integer label
	int size;

public:
	Lamport_Lecture_atomic(int n)
	{
		size = n;
		flag = new std::atomic <bool> [n];
		label = new std::atomic <int> [n];

		for (int i = 0; i < n; i++)
		{
			flag[i] = false;
			label[i] = 0;
		}
	}

	// make that one virtual so a subclass can
	// override it with its own function
private:
	virtual int draw_ticket()
	{
		int new_ticket = 0;
		for (int i = 0; i < size; i++)
		{
			new_ticket = std::max(new_ticket, label[i].load());
		}
		return new_ticket + 1;
	}

protected:
	bool keep_waiting(int id)
	{
		for (int i = 0; i < size; i++)
		{
			if ((flag[i]) && (lex_lesser_than2(label[i].load(), i, label[id].load(), id)))
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

		flag[id] = true;
		label[id] = draw_ticket();

#ifdef LOCK_MSG
		printf("flag[%i] = %i, label[%i] = %i\n", id, flag[id], id, label[id]);
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
	void lock()
	{
		doorway();
		wait();
	}

public:
	void unlock()
	{
		int id = omp_get_thread_num();

#ifdef LOCK_MSG
		printf("thread %i UNLOCKS\n", id);
#endif

		flag[id] = false;
	}
};


// class Lamport_Lecture : public DW_Lock
// {
// 	// bakery class according to lecture notes

// protected: // protected = private but sub-classes can access
// 	bool *flag;
// 	int *label; // unbounded integer label
// 	int size;

// public:
// 	Lamport_Lecture(int n);

// 	// make that one virtual so a subclass can
// 	// override it with its own function
// private:
// 	virtual int draw_ticket();

// protected:
// 	bool keep_waiting(int id);

// public:
// 	void doorway();

// public:
// 	void wait();

// public:
// 	void lock();

// public:
// 	void unlock();
// };

// class Lamport_Lecture_fix : public Lamport_Lecture
// {
// 	// bakery class according to lecture notes (fixed)
// 	// this uses an atomic integer register for keeping track of
// 	// the latest drawn ticket. like this mutual exclusion holds.
// private:
// 	std::atomic<int> latest_ticket;
// 	// normal int would not be good enough
// 	//int latest_ticket;

// public:
// 	Lamport_Lecture_fix(int n);

// private:
// 	virtual int draw_ticket() override;
// };

// class Lamport_Original : public DW_Lock
// {
// 	// bakery class according to Lamport 1974
// private:
// 	bool *gettoken;
// 	int *token; // unbounded integer label
// 	int size;

// public:
// 	Lamport_Original(int n);

// private:
// 	int take_ticket();

// public:
// 	void wait();

// public:
// 	void doorway();

// public:
// 	void lock();

// public:
// 	void unlock();
// };

//_79_column_check_line:#######################################################
