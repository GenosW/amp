#!/bin/bash

#SBATCH -p q_student
#SBATCH -N 1
#SBATCH -c 64
# SBATCH --cpu-freq=High
#SBATCH --time=5:00
#SBATCH --output=results/reference_10000_3.csv

# ./bin/bm_reference 2 1000 30 10000 100 0.4
# ./bin/bm_reference 3 1000 30 10000 100 0.4
# ./bin/bm_reference 4 1000 30 10000 100 0.4
# ./bin/bm_reference 8 1000 30 10000 100 0.4
# ./bin/bm_reference 16 1000 30 10000 100 0.4
# ./bin/bm_reference 32 1000 30 10000 100 0.4
./bin/bm_reference 64 100 30 10000 100 0.4
