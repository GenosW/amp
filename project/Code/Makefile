CC := g++ # This is the main compiler
# CC := clang --analyze # and comment out the linker last line for sanity
SRCDIR := source
BUILDDIR := build
PROG = project2
TARGET := bin/$(PROG)
 
SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CFLAGS :=  -Wall -std=c++11 -Xpreprocessor -g -fopenmp
LIB := -fopenmp # for MAC: -lomp
INC := -I./include # for MAC: -I/usr/local/Cellar/libomp/9.0.0/include/omp.h

# Quick summary on flags and command line options:
# -Wall: enable "all" warning (not really all...)
# -std=c++11 : set 
# -Xpreprocessor : enable preprocessor...think only needed for mac, but no problem with linux
# - g : enable debugging output... so you can work with debugger
# -fopenmp : enable OMP library
# MAC/Clang:
# -lomp : instead of -fopenmp
# no -g

$(TARGET): $(OBJECTS)
	@echo " Linking..."
	@echo " $(CC) $^ -o $(TARGET) $(LIB)"; $(CC) $^ -o $(TARGET) $(LIB)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " $(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET)

.PHONY: clean
