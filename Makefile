# Makefile for Modular Wavetable Synth with GTK+ GUI

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

# GTK Flags (includes pkg-config call for GTK3)
GTK_FLAGS = $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS = $(shell pkg-config --libs gtk+-3.0)

# Combine all flags
CXXFLAGS += $(GTK_FLAGS)
LDFLAGS = $(GTK_LIBS) -pthread

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJECTS)
	@echo "Linking $(TARGET)..."
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

# Build object files
%.o: %.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Dependency generation
%.d: %.cpp
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -MM $< > $@

# Clean build
clean:
	@echo "Cleaning..."
	rm -f $(OBJECTS) $(DEPS)
	rm -f $(TARGET)
	@echo "Clean complete"

# Debug target
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: clean all

# Run the application
run: $(TARGET)
	./$(TARGET)

# Rebuild target
rebuild: clean all

.PHONY: all clean debug run rebuild

# Include dependencies
-include $(DEPS)