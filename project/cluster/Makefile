CC := g++ # This is the main compiler
# CC := clang #--analyze
SRCDIR := source
BUILDDIR := build
PROG = project2
BINDIR := bin
TARGET := $(BINDIR)/$(PROG)
 
SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CFLAGS :=  -Wall -std=c++11 -Xpreprocessor -fopenmp -g
LIB := -fopenmp # for MAC: -lomp
INC := -I./include

main: build/main.o
	@echo " Linking..."
	@echo " $(CC) $? -o $(TARGET) $(LIB)"; $(CC) $?  -o $(TARGET) $(LIB)

bm_aravind: build/bm_aravind.o
	@echo " Linking..."
	@echo " $(CC) $? -o $(BINDIR)/$@ $(LIB)"; $(CC) $? -o $(BINDIR)/$@ $(LIB)

bm_jayanti: build/bm_jayanti.o
	@echo " Linking..."
	@echo " $(CC) $? -o $(BINDIR)/$@ $(LIB)"; $(CC) $? -o $(BINDIR)/$@ $(LIB)

bm_lamport: build/bm_lamport.o
	@echo " Linking..."
	@echo " $(CC) $? -o $(BINDIR)/$@ $(LIB)"; $(CC) $? -o $(BINDIR)/$@ $(LIB)

bm_taubenfeld: build/bm_taubenfeld.o
	@echo " Linking..."
	@echo " $(CC) $? -o $(BINDIR)/$@ $(LIB)"; $(CC) $? -o $(BINDIR)/$@ $(LIB)

bm_reference: build/bm_reference.o
	@echo " Linking..."
	@echo " $(CC) $? -o $(BINDIR)/$@ $(LIB)"; $(CC) $? -o $(BINDIR)/$@ $(LIB)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " $(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR)/* $(BINDIR)/*"; $(RM) -r $(BUILDDIR)/* $(BINDIR)/*

.PHONY: clean


# To make all Cluster benchmarks:
# make bm_aravind && make bm_jayanti && make bm_lamport && make bm_taubenfeld && bm_reference

# To make default target:
# make

# Project structure + Makefile adapted from:
# Hilton Lipschitz 2013, https://hiltmon.com/blog/2013/07/03/a-simple-c-plus-plus-project-structure/
# "Don't reinvent the wheel!"