#!/bin/bash

#SBATCH -p q_student
#SBATCH -N 1
#SBATCH -c 64
# SBATCH --cpu-freq=High
#SBATCH --time=5:00
#SBATCH --output=results/aravind_1000.csv

./bin/bm_aravind 2 1000 30 1000 100 0.4
./bin/bm_aravind 3 1000 30 1000 100 0.4
./bin/bm_aravind 4 1000 30 1000 100 0.4
./bin/bm_aravind 8 1000 30 1000 100 0.4
./bin/bm_aravind 16 1000 30 1000 100 0.4
./bin/bm_aravind 32 1000 30 1000 100 0.4
./bin/bm_aravind 64 1000 30 1000 100 0.4
