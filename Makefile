# Directories
SRCDIR=src
TESTDIR=tests
DEPDIR=deps
OBJDIR=objs

# Find all sources
SRC = $(shell find $(SRCDIR) -name "*.cpp")
OTMP = $(patsubst %.cpp,%.o,$(SRC:$(SRCDIR)/%=%))
OBJS = $(patsubst %,$(OBJDIR)/%,$(OTMP))
DEPS = $(patsubst %.o,$(DEPDIR)/%.d,$(OTMP))

# Variables
CC = g++
CPPFLAGS =-O0 -Iheaders -std=c++11 -Wall -Wextra 
LIBS = -lpthread $(shell pkg-config --libs  ibex)

all: init $(OBJS)
	@echo Building the executable...
	$(CC) -o main $(OBJS) $(LIBS)

init:
	@echo Initilizing...
	mkdir -p $(DEPDIR)
	mkdir -p $(OBJDIR)

exe: all
	./main

clean:
	@echo Clearing...
	rm -fr $(OBJDIR)/ $(DEPDIR)/

.PHONY: all clean test

-include $(DEPS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	mkdir -p $(dir $@)
	mkdir -p $(DEPDIR)/$(dir $(@:$(OBJDIR)/%=%))
	$(eval CPP = $(shell find $(SRCDIR)/$(dir $*) -name "$(notdir $*).cpp"))
	$(CC) -c $(CPPFLAGS) $(CPP) -o $@ $(LIBS)
	$(CC) -MM $(CPPFLAGS) $(CPP) $(LIBS) > $(DEPDIR)/$*.d
