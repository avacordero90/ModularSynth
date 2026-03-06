# Makefile for Modular Wavetable Synth

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -fPIC
DEBUG_FLAGS = -g -DDEBUG
TARGET = modular-synth

# Source directories
SRCDIR = src
BUILDDIR = build

# Source files
SOURCES = $(wildcard $(SRCDIR)/**/*.cpp) $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
DEPS = $(OBJECTS:.o=.d)

# Include directories
INC_DIRS = $(SRCDIR) $(SRCDIR)/core $(SRCDIR)/modules $(SRCDIR)/plugin
INCLUDES = $(addprefix -I, $(INC_DIRS))

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

# Build object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean build
clean:
	rm -rf $(BUILDDIR)/*
	rm -f $(TARGET)

# Debug target
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: clean all

.PHONY: all clean debug

# Include dependencies
-include $(DEPS)