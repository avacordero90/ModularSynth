# Makefile for Modular Wavetable Synth with GTK+ GUI

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -fPIC
DEBUG_FLAGS = -g -DDEBUG
TARGET = modular-synth
TEST_TARGET = synth-tests

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
TEST_SOURCES = tests/test_synth_core.cpp \
          $(SRCDIR)/core/audio_pipeline.cpp \
          $(SRCDIR)/core/wavetable.cpp \
          $(SRCDIR)/core/wavetable_manager.cpp \
          $(SRCDIR)/modules/envelope.cpp \
          $(SRCDIR)/modules/filter.cpp \
          $(SRCDIR)/modules/oscillator.cpp
TEST_OBJECTS = $(TEST_SOURCES:.cpp=.o)
TEST_DEPS = $(TEST_OBJECTS:.o=.d)

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
	rm -f $(OBJECTS) $(DEPS) $(TEST_OBJECTS) $(TEST_DEPS)
	rm -f $(TARGET) $(TEST_TARGET)
	@echo "Clean complete"

# Debug target
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: clean all

# Run the application
run: $(TARGET)
	./$(TARGET)

# Run unit/smoke tests
test: $(TEST_TARGET)
	./$(TEST_TARGET)

# Link test executable
$(TEST_TARGET): $(TEST_OBJECTS)
	@echo "Linking $(TEST_TARGET)..."
	$(CXX) $(TEST_OBJECTS) -o $@ -pthread
	@echo "Test build complete: $(TEST_TARGET)"

# Rebuild target
rebuild: clean all

.PHONY: all clean debug run rebuild test

# Include dependencies
-include $(DEPS) $(TEST_DEPS)