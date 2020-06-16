#! /bin/bash

#SBATCH -p q_student
#SBATCH -N 1                 
#SBATCH -c 64   # use all 64 cores 
#SBATCH --cpu-freq=High
#SBATCH --time=5:00
#SBATCH --output=ticketlock_job.out

export OMP_NUM_THREADS=16

./project2