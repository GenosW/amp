#!/bin/bash

#SBATCH -p q_student
#SBATCH -N 1
#SBATCH -c 64
# SBATCH --cpu-freq=High
#SBATCH --time=5:00
#SBATCH --output=results/reference_0_3.csv

# ./bin/bm_reference 2 5000 30 0 0 0.4
# ./bin/bm_reference 3 5000 30 0 0 0.4
# ./bin/bm_reference 4 5000 30 0 0 0.4
# ./bin/bm_reference 8 5000 30 0 0 0.4

# ./bin/bm_reference 16 5000 30 0 0 0.4
# ./bin/bm_reference 32 5000 30 0 0 0.4

./bin/bm_reference 64 5000 30 0 0 0.4
