# Project 2: Bounded timestamp register locks

Authors:   

- Simon Hinterseer, 09925802
- Peter Holzner, 1426733

## Project structure

Code:

    - /bin... contains the built binaries
    - /build... contains object files
    - /include... contains the implementations of the locks, tests and others
    - /source... contains the main-files
    - /jobs... batch submission scripts used for the cluster
    - Makefile

Data/Plotting:
    - /results... contains the benchmark results in csv format
    - /plots... contains the figures produced by the plots.ipynb
    - plots.ipynb... jupyter notebook for data handling, evaluation and plotting

## How-to

### main (standard target/rule)
Building is handled via a simple Makefile. Simply typing "make" will build the default target, which is main.cpp, and output the binary "bin/project2".
This version is meant mainly for desktop use because it has more verbose output.

A lock needs to be selected in the source file in the lock section (line 86) and then rebuilt. Other parameters can be set via command line.

The following signature is used:
./project2 num_threads num_turns num_tests workload cs_workload randomness

If not set, the following defaults are used:

    - num_threads... 2
	- num_turns... 10000
	- num_tests... 30
	- workload... 400
	- cs_workload... 400
	- randomness... 0.1

Below is an example call and output:

```shell
./bin/project2 4 10000 10 1000 10000 0.3
OMP engaged!
Testing OMP:
Max num threads = 8

Testing lock 2: Taubenfeld_Paper_1_atomic
Performing mutex test: 1
Performing FCFS test: 1
num_threads = 4
num_turns = 10000
num_tests = 10
num_events = 160000
randomness = 0.300000

######################
#     MUTEX TEST     #
######################

Passed: 1

----------------------

######################
#      FCFS TEST     #
######################

Passed: 1

----------------------

######################
#       LRU TEST     #
######################

Passed: 1

######################
#     THROUGHPUT     #
######################

designated throughput test
---------------------------------------------------
runtime (s) = 4.5996
throughput (acq/s) = 8696.3158
average number of "other" contenders (#thr) = 2.0006
average number of contenders (#thr) = 3.0006
> Average time elapsed (s) = 4.601
> Average anc eval time (s) = 0.004

time measurement from record_event_log (no logging)
---------------------------------------------------
> Average time elapsed = 4.594702 s

----------------------

######################
#       RESUMÉ       #
######################

Benchmark parameters:
Lock name (attribute): Taubenfeld_Paper_1_atomic
num_threads = 4
num_turns = 10000
num_tests = 10
num_events = 160000
randomness = 0.300000

Benchmark results:
mutex_fail_count = 0
fcfs_fail_count = 0
lru_fail_count = 0
runtime (s) = 4.5996
throughput (acq/s) = 8696.3158
average number of "other" contenders (#thr) = 2.0006
average number of contenders (#thr) = 3.0006
Total benchmark runtime (min:sec:msecs) = 1:44:998
Total benchmark runtime (sec) = 105.00


Garbage collection...
```

### bm_*
The source files titled bm_*.cpp are the source files used for the benchmarking on the cluster. They are less verbose and will only print results in csv-format (seperated by ;, see the result files) to stdout. Hence the usage for the cluster.

One source file corresponds to one lock implementation:
bm_aravind... Aravind_fix
bm_jayanti... Jayantin_BT
bm_lamport... Lamport_Paper_atomic
bm_reference... Reference_lock_
bm_taubenfeld... Taubenfeld_atomic

They can be built via e.g. "make bm_aravind", and will produce a binary with the respective name in the "/bin" folder. They use the same calling signature as above.

### plot.ipynb

The notebook requires "numpy, pandas, matplotlib and jupyterlab".

The notebook will process all csv-files within the "results" subfolder of "results" ("results/results/*") into one csv-file per benchmark (set of parameters as above) and put them into "results/." (NOT "results/results/."). (Function split_csv).

These seperated csv-files ("results/.") are then further processed via the function "eval_csv" and merged together into one dataframe containing the mean, std, min and max values for each measurement. Each measurement (throughput, runtime, etc.) is saved per test (we used 30 tests per benchmark), so e.g. the mean is calculated across these 30 tests (=1 benchmark).