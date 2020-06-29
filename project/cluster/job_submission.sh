#!/bin/bash

#SBATCH -p q_student
#SBATCH -N 1
#SBATCH -c 64
# SBATCH --cpu-freq=High
#SBATCH --time=5:00
#SBATCH --output=jayanti_100.csv

./bin/bm_jayanti 2 1000 30 100 1000 0.4
./bin/bm_jayanti 3 1000 30 100 1000 0.4
./bin/bm_jayanti 4 1000 30 100 1000 0.4
./bin/bm_jayanti 8 1000 30 100 1000 0.4
./bin/bm_jayanti 16 1000 30 100 1000 0.4
./bin/bm_jayanti 32 1000 30 100 1000 0.4
./bin/bm_jayanti 64 1000 30 100 1000 0.4
