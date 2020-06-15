
#include <algorithm> // for the std::max() function
#include <omp.h>
#include <atomic> 	// for the reference lock

#include "toolbox.cpp"

// compiler switch that turns some debugging messages on/off
//#define LOCK_MSG

class Lock {
	// basic lock interface
	public:
		virtual void lock() = 0;
		virtual void unlock() = 0;
};

class DW_Lock : public Lock {
	// basic lock interface for locks, that have a doorway
	public:
		virtual void doorway() = 0;
		virtual void wait() = 0;	
		virtual void lock() = 0;
		virtual void unlock() = 0;
};

class Reference_Lock : public DW_Lock {
	// the "native" c++ lock wrapped in our class structure
	private: std::atomic_flag lock_stream = ATOMIC_FLAG_INIT;
	// unfortunately, declaring like this fails:
	//private: std::atomic_flag lock_stream;

	public: Reference_Lock(){
		std::atomic_flag lock_stream = ATOMIC_FLAG_INIT;	
	}

	public: void doorway(){
		int id = omp_get_thread_num();
		#ifdef LOCK_MSG
			printf("thread %i TRIES to acquire the lock\n",id);
		#endif
	}
	
	public: void wait(){
		int id = omp_get_thread_num();
		while ( lock_stream.test_and_set() ) {}
		#ifdef LOCK_MSG
			printf("thread %i ACQUIRES the lock\n",id);
		#endif	
	}
	
	public: void lock(){
		doorway();
		wait();
	}
	
	public: void unlock(){
		int id = omp_get_thread_num();
		#ifdef LOCK_MSG
			printf("thread %i UNLOCKS\n",id);			
		#endif
		lock_stream.clear();	
	}
};

class Lamport_Lecture : public DW_Lock {
	// bakery class according to lecture notes

	protected: // protected = private but sub-classes can access
		bool *flag;
		int *label; // unbounded integer label
		int size;

	public: Lamport_Lecture(int n) {
		size = n;
		flag = new bool[n];
		label = new int[n];

		for	(int i =0; i < n; i ++){
			flag[i] = false;
			label[i] = 0;
		}
	}
	
	// make that one virtual so a subclass can
	// override it with its own function
	private: virtual int draw_ticket() {
		int new_ticket = 0;
		for (int i = 0; i < size; i++) {
			new_ticket = std::max(new_ticket, label[i]);
		}
		return new_ticket + 1;
	}

	protected: bool keep_waiting(int id){
		for (int i = 0; i < size; i++){
			if ((flag[i]) && (lex_lesser_than2(label[i],i,label[id],id))){
				return true;
			}
		}
		return false;
	}
	
	public: void doorway(){
		int id = omp_get_thread_num();

		#ifdef LOCK_MSG
			printf("thread %i TRIES to acquire the lock\n",id);
		#endif

		flag[id] = true;
		label[id] = draw_ticket();

		#ifdef LOCK_MSG
			printf("flag[%i] = %i, label[%i] = %i\n",id,flag[id],id,label[id]);
		#endif	
	}
	
	public: void wait(){
		int id = omp_get_thread_num();
		while ( keep_waiting(id) ) {}
		#ifdef LOCK_MSG
			printf("thread %i ACQUIRES the lock\n",id);
		#endif	
	}
	
	public: void lock(){
		doorway();
		wait();
	}
	
	public: void unlock(){
		int id = omp_get_thread_num();

		#ifdef LOCK_MSG
			printf("thread %i UNLOCKS\n",id);
		#endif

		flag[id] = false;	
	}	
};

class Lamport_Lecture_fix : public Lamport_Lecture {
	// bakery class according to lecture notes (fixed)
	// this uses an atomic integer register for keeping track of
	// the latest drawn ticket. like this mutual exclusion holds.

	private: 
		std::atomic <int> latest_ticket;
		// normal int would not be good enough
		//int latest_ticket;

	public: Lamport_Lecture_fix(int n) : Lamport_Lecture{ n }
	{
		latest_ticket.store(0);
		//latest_ticket = 0;
	}

	private: virtual int draw_ticket() override {
		return latest_ticket.fetch_add(1);
		// this is the atomic version of
		//return ++latest_ticket;
	}
};


class Lamport_Original : public DW_Lock {
	// bakery class according to Lamport 1974

private:
	bool *gettoken;
	int *token; // unbounded integer label
	int size;

	public: Lamport_Original(int n) {
		size = n;
		gettoken = new bool[n];
		token = new int[n];

		for (int i = 0; i < n; i++) {
			gettoken[i] = false;
			token[i] = -1;
		}
	}

	private: int take_ticket() {
		int new_ticket = 0;
		for (int i = 0; i < size; i++) {
			new_ticket = std::max(new_ticket, token[i]);
		}
		return new_ticket + 1;
	}

	public: void wait() {
		int id = omp_get_thread_num();
		for (int j = 0; j < size; j++) {
			if (j == id) j++;
			if (j == size) break;
			while (gettoken[j]) {}
			while (not((token[j] == -1)
				|| lex_lesser_than2(token[id], id, token[j], j)
				)) {}
		}
		#ifdef LOCK_MSG
			printf("thread %i ACQUIRES the lock\n", id);
		#endif	
	}

	public: void doorway() {
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

	public: void lock() {
		doorway();
		wait();
	}

	public: void unlock() {
		int id = omp_get_thread_num();
		#ifdef LOCK_MSG
			printf("thread %i UNLOCKS\n", id);
		#endif
		token[id] = -1;
	}
};

////////////////////////////////////////////////////////////////////////////79