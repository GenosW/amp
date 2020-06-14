PROG = project2
OBJECTS = main.cpp
PREREQ = main.cpp locks.cpp toolbox.cpp Makefile
CC = g++

FLAGS = -Wall -fopenmp -std=c++11
# optimization flags are dangerous here
# -O1 and -O3 will break Lamport_Original
#FLAGS = -Wall -fopenmp -O3

all: $(PROG)

$(PROG): $(PREREQ)
	$(CC) $(OBJECTS) $(FLAGS) -o $@

clean:
	rm $(PROG)

