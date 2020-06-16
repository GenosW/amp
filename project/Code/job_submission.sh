#! /bin/bash

#SBATCH -p q_student
#SBATCH -N 1
#SBATCH -c 64
#SBATCH --cpu-freq=High
#SBATCH --time=5:00
#SBATCH --output=ticketlock_job.out

./~/project/Code/bin/project2 4 10 10

# srun -p q_student --time=1:00 -N 1 ./project2 4 10 10