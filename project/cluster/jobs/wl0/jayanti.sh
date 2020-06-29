#!/bin/bash

#SBATCH -p q_student
#SBATCH -N 1
#SBATCH -c 64
# SBATCH --cpu-freq=High
#SBATCH --time=5:00
#SBATCH --output=results/jayanti_0.csv

./bin/bm_jayanti 2 5000 30 0 0 0.4
./bin/bm_jayanti 3 5000 30 0 0 0.4
./bin/bm_jayanti 4 5000 30 0 0 0.4
./bin/bm_jayanti 8 5000 30 0 0 0.4
./bin/bm_jayanti 16 5000 30 0 0 0.4
./bin/bm_jayanti 32 5000 30 0 0 0.4
./bin/bm_jayanti 64 5000 30 0 0 0.4
