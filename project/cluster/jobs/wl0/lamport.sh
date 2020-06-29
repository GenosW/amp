#!/bin/bash

#SBATCH -p q_student
#SBATCH -N 1
#SBATCH -c 64
# SBATCH --cpu-freq=High
#SBATCH --time=5:00
#SBATCH --output=results/lamport_0.csv

./bin/bm_lamport 2 1000 30 0 0 0.4
./bin/bm_lamport 3 1000 30 0 0 0.4
./bin/bm_lamport 4 1000 30 0 0 0.4
./bin/bm_lamport 8 1000 30 0 0 0.4
./bin/bm_lamport 16 1000 30 0 0 0.4
./bin/bm_lamport 32 1000 30 0 0 0.4
./bin/bm_lamport 64 1000 30 0 0 0.4
