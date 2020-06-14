PROG = project2
OBJECTS = main.cpp
PREREQ = main.cpp locks.cpp toolbox.cpp Makefile
CXX = g++
#CXX = clang

FLAGS = -Wall -std=c++11 -Xpreprocessor -fopenmp -lomp
# optimization flags are dangerous here
# -O1 and -O3 will break Lamport_Original
#FLAGS = -Wall -fopenmp -O3

all: $(PROG)

$(PROG): $(PREREQ)
	$(CXX) $(OBJECTS) $(FLAGS) -g -o $@

.PHONY: clean
clean:
	rm $(PROG)

