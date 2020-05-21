PROG = colorBakergy
OBJECTS = colorbakery.cpp
CC = g++

FLAGS = -Wall -fopenmp -std=c++11

all: $(PROG)

$(PROG): $(OBJECTS)
	$(CC) $(OBJECTS) $(FLAGS) -o $@

clean:
	rm $(PROG)

