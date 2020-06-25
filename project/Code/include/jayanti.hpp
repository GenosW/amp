//_79_column_check_line:#######################################################
#pragma once
#include "locks.hpp"
#include "toolbox.hpp"
#include <atomic>
//#define DEBUG

class Jayanti : public DW_Lock
{
	// Lock by Prasad Jayanti et al. 2001
	// unbounded token version
public:
	string name = "Jayanti";
#ifdef DEBUG
	std::atomic_flag lock_stream = ATOMIC_FLAG_INIT;
#endif // DEBUG

protected: // protected = private but sub-classes can access
	std::atomic <bool> *gettoken;
	std::atomic <int> *token;
	std::atomic <int> X;
	int size;

public:
	Jayanti(int n)
	{
		size = n;
		gettoken = new std::atomic <bool>[n];
		token = new std::atomic <int>[n];

		X.store(0);
		for (int i = 0; i < n; i++)
		{
			gettoken[i] = false;
			token[i] = -1;
			//token[i].store(-1);
		}
	}
	~Jayanti(void)
	{
		printf("Deleting %s...\n", name.c_str());
		delete[] gettoken;
		delete[] token;
	}

// make that one virtual so a subclass can
// override it with its own function
protected:
	virtual int get_token(int* tl_token, int tl_x)
	{
		int new_token = tl_x; // speciality of Jayanti token draw
		for (int i = 0; i < size; i++)
		{
			new_token = std::max(new_token, tl_token[i]);
		}
		return new_token + 1;
	}

protected:
	bool keep_waiting1(int j)
	{
		// "wait till A" translates to "while not(A) {}"
		// not (gettoken[j] == false)
		return (gettoken[j] == true);
	}

	virtual bool keep_waiting2(int id, int j)
	{
		// "wait till A" translates to "while not(A) {}"
		// not(A or B) = not(A) and not(B)
		// not( (token[j] == -1) || lex_lesser_than2(token[i],i,token[j],j) )
		return ((token[j] != -1) 
			    && lex_lesser_than2(token[j].load(), j, 
									token[id].load(), id)
			   );
	}

public:
	void doorway()
	{
		int id = omp_get_thread_num();

		gettoken[id] = true;
		// line 2
		int* tl_token = new int[size]; // thread_local tokens
		for (int i = 0; i < size; i++) {
			tl_token[i] = token[i];
		}
		// line 3
		int tl_x = X.load(); // thread_local X
		// line 4
		token[id] = get_token(tl_token,tl_x);
		// line 5
		gettoken[id] = false;

#ifdef DEBUG
		while (lock_stream.test_and_set()) {}
		printf("thread %i TRIES to acquire the lock\n", id);
		print_status(tl_token, tl_x);
		printf("token[%i] = %i\n", id, token[id].load());
		lock_stream.clear();
#endif
	}

public:
	virtual void wait()
	{
		int id = omp_get_thread_num();
		for (int thread_B = 0; thread_B < size; thread_B++)
		{
			// dont check yourself
			if (thread_B == id) {
				thread_B++;
				if (thread_B == size) break;
			}
			while (keep_waiting1(thread_B)) {} // line 5
			while (keep_waiting2(id, thread_B)) {}	// line 6
		}
		X.store(token[id].load());
#ifdef DEBUG
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

#ifdef DEBUG
		printf("thread %i UNLOCKS\n", id);
#endif

		token[id].store(-1);
	}

	int read_X() {
		// allow external call to read token of last acquirer
		// for checking boundedness of tokens
		return X.load();
	}

#ifdef DEBUG
	protected:
		void print_status(int* mytoken=NULL, int myx=NULL) {
			// "=NULL" was a way to make the arguments optional
			// if (mytoken) translates to if (mytoken != NULL)
			// i.o.w.: if mytoken was passed
			if (mytoken) {
				for (int i = 0; i < size; i++) {
					printf("token[%i]=%i, ", i, mytoken[i]);
				}
			}
			else {
				for (int i = 0; i < size; i++) {
					printf("token[%i]=%i, ", i, token[i].load());
				}
			}
			if (myx) {
				printf("X = %i, ", myx);
				printf("\n");
			}
			else {
				printf("X = %i, ", X.load());
				printf("\n");
			}
		}
#endif // DEBUG

};



// ================================================
//                JAYANTI_BT
// ================================================

class Jayanti_BT : public Jayanti
{
	// Lock by Prasad Jayanti et al. 2001
	// bounded token version

public:
	string name = "Jayanti_BT";

public:
	Jayanti_BT(int n) :Jayanti{ n }
	{}
	~Jayanti_BT(void)
	{
		printf("Deleting %s...\n", name.c_str());
		delete[] gettoken;
		delete[] token;
	}

private:
	int jaya_plus(int a, int b) {
		// in c++: (-12 mod 5 = -2) but we need (-12 mod 5 = 3)
		// so this loop is necessary
		int arg = a + b;
		while (arg < 0) {
			arg += 2 * size - 1;
		}
		return arg % (2 * size - 1);
		//return (a + b) % (2 * size - 1);
	}
private:
	int jaya_minus(int a, int b) {
		// in c++: (-12 mod 5 = -2) but we need (-12 mod 5 = 3)
		// so this loop is necessary
		int arg = a - b;
		while (arg < 0) {
			arg += 2 * size - 1;
		}
		return arg % (2 * size - 1);
	}

private:
	bool jaya_comp(int token_a,int a,int token_b,int b) {

		int shift_a = jaya_plus(token_a, size - 1 - token_a);
		int shift_b = jaya_plus(token_b, size - 1 - token_a);

		return lex_lesser_than2(shift_a,a,shift_b,b);
	}

private:
	int jaya_max(int* tl_token, int tl_x) {

		// T = S u{ x } \ {-1}
		// jaya_max(S u{ x }) = jaya_minus(max(jaya_plus(v, n - 1 - x:v in T)),
		//								   n - 1 - x)
		int max_val = 0;

		// max over T\{X}
		for (int i = 0; i < size; i++) {
			// skil all { -1 } values
			while (tl_token[i] == -1) { 
				i = i + 1; 
				if (i == size) { break; }
			}
			if (i == size) { break; }

			max_val = std::max(max_val,
				jaya_plus(tl_token[i], size - 1 - tl_x)
			);
		}

		// max over T
		max_val = std::max(max_val,
			jaya_plus(tl_x, size - 1 - tl_x)
		);

		return jaya_minus(max_val, size - 1 - tl_x);
	}


protected:
	virtual int get_token(int* tl_token, int tl_x) override
	{
		int max_val = jaya_max(tl_token,tl_x);
		return jaya_plus(max_val, 1);
	}

	virtual bool keep_waiting2(int id, int j) override
	{
		// "wait till A" translates to "while not(A) {}"
		// not(A or B) = not(A) and not(B)
		// not( (token[j] == -1) || lex_lesser_than2(token[i],i,token[j],j) )
		return ((token[j].load() != -1)
			&& jaya_comp(token[j].load(), j, token[id].load(), id)
			);
	}

	public:
		virtual void wait() override
		{
			int id = omp_get_thread_num();
#ifdef DEBUG
			while (lock_stream.test_and_set()) {}
			printf("thread %i ENTERS wait()\n", id);
			for (int i = 0; i < size; i++) {
				bool cv = jaya_comp(token[i].load(), i, token[id].load(), id);
				printf("jaya_comp(%i,%i,%i,%i) = %i\n",
					token[i].load(), i, token[id].load(), id, cv);
			}
			lock_stream.clear();
#endif
			for (int thread_B = 0; thread_B < size; thread_B++)
			{
				// dont check yourself
				if (thread_B == id) {
					thread_B++;
					if (thread_B == size) break;
				}
				while (keep_waiting1(thread_B)) {}
				while (keep_waiting2(id, thread_B)) {}
			}
			X.store(token[id].load());
#ifdef DEBUG
			while (lock_stream.test_and_set()) {}
			printf("thread %i ACQUIRES the lock\n", id);
			print_status();
			for (int i = 0; i < size; i++) {
				bool kw = keep_waiting2(id, i);
				printf("keep_waiting2(%i,%i) = %i\n",id,i,kw);
			}
			lock_stream.clear();
#endif
		}

#ifdef DEBUG
		public: void my_test() {
			printf("######### my_test ############\n");
			printf("jaya_comp(5,0,0,2) = ...\n");
			printf("jaya_comp(5,0,0,2) = %i \n", jaya_comp(5, 0, 0, 2));
			int token_a = 5;
			int token_b = 0;
			int shift_a = jaya_plus(5, size - 1 - 5);
			printf("shift_a = %i \n", shift_a);
			int shift_b = jaya_plus(token_b, size - 1 - token_a); // sic
			printf("shift_b = %i \n", shift_b);
			bool ll2 = lex_lesser_than2(shift_a, 0, shift_b, 2);
			printf("ll2(%i,%i,%i,%i) = %i\n", shift_a, 0, shift_b, 2, ll2);
		}
#endif // DEBUG


};

//_79_column_check_line:#######################################################
