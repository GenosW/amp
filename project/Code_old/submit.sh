#! /bin/bash

#SBATCH -p q_student
#SBATCH -N 1
#SBATCH -c 64
#SBATCH --cpu-freq=High
#SBATCH --time=5:00
#SBATCH --output=project2.out

./~/project/Code/bin/project2