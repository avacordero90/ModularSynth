# Makefile for Modular Wavetable Synth with GTK+ GUI

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -fPIC
DEBUG_FLAGS = -g -DDEBUG
TARGET = modular-synth

# Source directories
SRCDIR = src
BUILDDIR = build

# Source files
# Only compile core modules and the application entry point (synth component).
SOURCES = $(wildcard $(SRCDIR)/core/*.cpp) \
          $(wildcard $(SRCDIR)/modules/*.cpp) \
          $(SRCDIR)/app/synth_component.cpp \
          $(SRCDIR)/app/midi_handler.cpp \
          $(SRCDIR)/main.cpp
OBJECTS = $(SOURCES:.cpp=.o)
DEPS = $(OBJECTS:.o=.d)

# Include directories
INC_DIRS = $(SRCDIR) $(SRCDIR)/core $(SRCDIR)/modules $(SRCDIR)/app
INCLUDES = $(addprefix -I, $(INC_DIRS))

# GTK Flags (includes pkg-config call for GTK3)
GTK_FLAGS = $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS = $(shell pkg-config --libs gtk+-3.0)


# Combine all flags
CXXFLAGS += $(GTK_FLAGS) -DUSE_PORTAUDIO
LDFLAGS = $(GTK_LIBS) -lportaudio -pthread

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