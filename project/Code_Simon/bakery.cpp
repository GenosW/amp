
#include  <stdio.h>
#include <omp.h>
#include <algorithm> // for the std::max() function
#include <math.h>

#include <assert.h>

#define LOCK_MSG

template <typename myT>
bool isequal(myT* array1, myT* array2, int length){
	// checks if two arrays are the same (pointwise)
	for(int i = 0; i<length; i++){
		if (array1[i] != array2[i]) return false;
	}
	return true;
}

void print_array(int* array, int cols, int rows){
	// prints an [array] to the console
	// cols/rows ... number of columns/rows
	for (int j = 0; j != rows; ++j)
  {
    for (int i = 0; i != cols; ++i)
    {
			printf("%d,",array[i + cols * j]);
    }
    printf("\n");
  }
}

template <typename myT>
bool lex_lesser_than(myT* a, myT* b, int size){
	// lexicographic comparison for 2 arrays
	for(int i = 0; i < size; i++){
		if (a[i] > b[i]) return false;
		if (a[i] < b[i]) return true;
	}
	return false;
}

template <typename myT>
bool lex_lesser_than2(myT a1, myT a2, myT b1, myT b2){
	// lexicographic comparison for 4 elements
	if (a1 > b1) return false;
	if (a1 < b1) return true;
	if (a2 > b2) return false;
	if (a2 < b2) return true;

	return false;
}


class Bakery {
	// bakery class according to lecture notes (adapted)
	private:
		bool *flag;
		int *label; // unbounded integer label
		int size;

	public:
		// array of thread ids, that complete the doorway
		int dw_completed;
		int *dw_completers;
		// array of thread ids, that acquire the lock
		int acquired;
		int *acquirers;

	// constructor
	public: Bakery(int n, int test_size) {
		size = n;
		flag = new bool[n];
		label = new int[n];
		dw_completed = 0;
		dw_completers = new int[n*test_size];
		acquired = 0;
		acquirers = new int[n*test_size];

		for	(int i =0; i < n; i ++){
			flag[i] = false;
			label[i] = 0;
		}
	}
	
	private: int take_ticket(){
		int new_ticket = 0;
		for (int i = 0; i < size; i++){
			new_ticket = std::max(new_ticket,label[i]);
		}	
		return new_ticket + 1;
	}
	
	private: bool keep_waiting(int id){
		bool kw = false;
		for (int i = 0; i < size; i++){
			if (flag[i] && (lex_lesser_than2(label[i],i,label[id],id))){
				kw = true;
			}
		}
		return kw;
	}
	
	public: void lock(int id){
		#ifdef LOCK_MSG
			printf("thread %i TRIES to acquire the lock\n",id);
		#endif

		flag[id] = true;
		label[id] = take_ticket();

		#ifdef LOCK_MSG
			printf("flag[%i] = %i, label[%i] = %i\n",id,flag[id],id,label[id]);
		#endif

		dw_completers[dw_completed] = id;
		dw_completed += 1;

		while ( keep_waiting(id) ) {}

		acquirers[acquired] = id;
		acquired += 1;
		#ifdef LOCK_MSG
			printf("thread %i ACQUIRES the lock\n",id);
		#endif
	}
	
	public: void unlock(int id){
		#ifdef LOCK_MSG
			printf("thread %i UNLOCKS\n",id);
		#endif

		flag[id] = false;	
	}	
};

void do_some_work(int workload){
	double dummy = 2.;

	for (int i = 0; i < workload; i++){
		dummy = exp(dummy);
		dummy += 1;
		dummy = log(dummy);
	}
}


int main(){

	int num_threads = 2;
	
	// how many times does every thread need to pass through critical section
	int num_turns = 10; 
	
	int workload = int(1e6); // amount of work in the critical section
	omp_set_num_threads(num_threads);

	//Bakery mybakery = Bakery(4);
	auto mybakery = new Bakery(num_threads, num_turns);


	#pragma omp parallel
	{
		int thread_id = omp_get_thread_num();
		
		for (int i = 0; i < num_turns; i++){
			mybakery->lock(thread_id);
			do_some_work(workload);
			mybakery->unlock(thread_id);
		}
		
		#pragma omp barrier
		// output of results
		
		if (thread_id == 0){
			printf("dw_completed = %d\n",mybakery->dw_completed);
			printf("acquired = %d\n",mybakery->acquired);
			bool fcfs = isequal(mybakery->dw_completers, mybakery->acquirers, num_threads*num_turns);
			printf("acquirers = dw_completers (i.e. fcfs) = %d\n",fcfs);

			printf("dw_completers = \n");
			print_array(mybakery->dw_completers,num_threads,num_turns);
			printf("acquirers = \n");
			print_array(mybakery->acquirers,num_threads,num_turns);
		}
		
	}

/*
	int t[] = {3,3,4,3};
	printf("llt(%d,%d,%d,%d) = %d\n",t[0],t[1],t[2],t[3],
																	lex_lesser_than2(t[0],t[1],t[2],t[3]));
*/

	return 0;	        
}




