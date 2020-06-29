#!/bin/bash

#SBATCH -p q_student
#SBATCH -N 1
#SBATCH -c 64
# SBATCH --cpu-freq=High
#SBATCH --time=5:00
#SBATCH --output=results/aravind_0.csv

./bin/bm_aravind 2 5000 30 0 0 0.4
./bin/bm_aravind 3 5000 30 0 0 0.4
./bin/bm_aravind 4 5000 30 0 0 0.4
./bin/bm_aravind 8 5000 30 0 0 0.4
./bin/bm_aravind 16 5000 30 0 0 0.4
./bin/bm_aravind 32 5000 30 0 0 0.4
./bin/bm_aravind 64 5000 30 0 0 0.4
