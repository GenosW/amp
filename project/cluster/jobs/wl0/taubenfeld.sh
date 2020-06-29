#!/bin/bash

#SBATCH -p q_student
#SBATCH -N 1
#SBATCH -c 64
# SBATCH --cpu-freq=High
#SBATCH --time=5:00
#SBATCH --output=results/taubenfeld_0.csv

./bin/bm_taubenfeld 2 5000 30 0 0 0.4
./bin/bm_taubenfeld 3 5000 30 0 0 0.4
./bin/bm_taubenfeld 4 5000 30 0 0 0.4
./bin/bm_taubenfeld 8 5000 30 0 0 0.4
./bin/bm_taubenfeld 16 5000 30 0 0 0.4
./bin/bm_taubenfeld 32 5000 30 0 0 0.4
./bin/bm_taubenfeld 64 5000 30 0 0 0.4
