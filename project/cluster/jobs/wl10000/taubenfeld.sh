#!/bin/bash

#SBATCH -p q_student
#SBATCH -N 1
#SBATCH -c 64
# SBATCH --cpu-freq=High
#SBATCH --time=5:00
#SBATCH --output=results/taubenfeld_10000.csv

./bin/bm_taubenfeld 2 1000 30 10000 100 0.4
./bin/bm_taubenfeld 3 1000 30 10000 100 0.4
./bin/bm_taubenfeld 4 1000 30 10000 100 0.4
./bin/bm_taubenfeld 8 1000 30 10000 100 0.4
./bin/bm_taubenfeld 16 1000 30 10000 100 0.4
./bin/bm_taubenfeld 32 1000 30 10000 100 0.4
./bin/bm_taubenfeld 64 1000 30 10000 100 0.4
