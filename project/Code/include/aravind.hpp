//_79_column_check_line:#######################################################
#pragma once
#include "locks.hpp"
#include "toolbox.hpp"
#include <atomic>
//#define LOCK_MSG


class Aravind : public DW_Lock
{
	// Lock according to Aravind 2011
public:
	string name = "Aravind";

protected: // protected = private but sub-classes can access
	std::atomic <bool> *c; //trying to acquire lock
	std::atomic <bool> *stage; // 2 stages of acquisition
	std::atomic <int> *ts; // keeps track of when the lock was last acquired
	int size;

public:
	Aravind(int n)
	{
		size = n;
		c = new std::atomic<bool>[n];
		stage = new std::atomic<bool>[n];
		ts = new std::atomic<int>[n];

		for (int i = 0; i < n; i++)
		{
			c[i] = false;
			stage[i] = false;
			ts[i] = i;
		}
	}
	~Aravind()
	{
		printf("Deleting %s...\n", name.c_str());
		delete[] c;
		delete[] stage;
		delete[] ts;
	}

private:
	virtual int new_ts()
	{
		// returns a new time stamp 
		// aravind states in the 2011 paper that instead of calculating the
		// max it is also possible to use an additional incrementing integer
		// register

		int new_ts = 0;
		for (int i = 0; i < size; i++)
		{
			new_ts = std::max(new_ts, ts[i].load());
		}
		return new_ts + 1;
	}

protected:
	bool keep_waiting_1(int id)
	{
		// check if someone else has higher priority
		for (int i = 0; i < size; i++)
		{
			if (i == id) { // dont check yourself
				i += 1;
				if (i == size) {
					return false; // you checked everybody already
				}
			}
			// i has lower prioritiy than me
			bool has_lower_p = (not(c[i]) || (ts[i] > ts[id]));
			if (not(has_lower_p)) {
				return true;
			}
		}
		// no one had higher priority? no need to keep waiting!
		return false;
	}

	bool keep_waiting_2(int id)
	{
		// check if someone else is at stage 1 of acquisition
		for (int i = 0; i < size; i++)
		{
			if (i == id) { // dont check yourself
				i += 1;
				if (i == size) {
					return false; // you checked everybody already
				}
			}
			if (stage[i]) {
				return true;
			}
		}
		// no one else was at stage 1? no need to keep waiting!
		return false;
	}



public:
	void doorway()
	{
		int id = omp_get_thread_num();

#ifdef LOCK_MSG
		printf("thread %i TRIES to acquire the lock\n", id);
#endif
		c[id] = true;
#ifdef LOCK_MSG
		printf("ts[%i] = %i\n", id, ts[id].load());
#endif
	}

public:
	void wait()
	{
		int id = omp_get_thread_num();
		do {
			stage[id] = false;
			while (keep_waiting_1(id)) {}
			stage[id] = true;
		} while (keep_waiting_2(id));

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
		ts[id] = new_ts();
		stage[id] = false;
		c[id] = false;
	}
};

class Aravind_fix : public Aravind
{
	// Lock according to Aravind 2011 including the check_c array to 
	// reduce the number of lru violations
public:
	string name = "Aravind_fix";

public:
	Aravind_fix(int n) : Aravind{ n }
	{
	}

	~Aravind_fix()
	{
		printf("Deleting %s...\n", name.c_str());
		delete[] c;
		delete[] stage;
		delete[] ts;
	}

protected:
	virtual bool alt_keep_waiting_1(int id, bool* check_c)
	{
		for (int i = 0; i < size; i++) check_c[i] = c[i];
			
		// check if someone else has higher priority
		for (int i = 0; i < size; i++)
		{
			if (i == id) { // dont check yourself
				i += 1;
				if (i == size) {
					return false; // you checked everybody already
				}
			}
			// i has lower prioritiy than me
			bool has_lower_p = (not(c[i]) || (ts[i] > ts[id]));
			if (not(has_lower_p)) {
				return true;
			}
		}
		// no one had higher priority? no need to keep waiting!
		return false;
	}

private: bool check_contenders(bool* check_c) {
		for (int i = 0; i < size; i++) {
			if (c[i] != check_c[i]) return false;
		}
		return true;
	}

public:
	void wait() override
	{
		// array of contenders
		bool* check_c = new bool[size];
		for (int i = 0; i < size; i++) {
			check_c[i] = false;
		}

		int id = omp_get_thread_num();
		do {
			stage[id] = false;
			while (alt_keep_waiting_1(id,check_c)) {}
			stage[id] = true;
			//check again, if no one entered after you checked
		} while (keep_waiting_2(id) || not(check_contenders(check_c)));

#ifdef LOCK_MSG
		printf("thread %i ACQUIRES the lock\n", id);
#endif
	}

};



//_79_column_check_line:#######################################################
