# Bounded timestamp register locks

Authors:   

- Simon Hinterseer, 
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

### bm_*
The source files titled bm_*.cpp are the source files used for the benchmarking on the cluster. They are less verbose and will only print results in csv-format (seperated by ;, see the result files) to stdout. Hence the usage for the cluster.

One source file corresponds to one lock implementation:
bm_aravind... Aravind_fix
bm_jayanti... Jayantin_BT
bm_lamport... Lamport_Paper_atomic
bm_reference... Reference_lock_
bm_taubenfeld... Taubenfeld_atomic

They can be built via e.g. "make bm_aravind", and will produce a binary with the respective name in the "/bin" folder. They use the same calling signature as above.