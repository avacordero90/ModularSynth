# GTK+ Setup Guide for Modular Wavetable Synth

## Overview
This project now uses GTK+ 3 to provide a professional, cross-platform GUI for the modular synthesizer. GTK+ is a lightweight, widely-supported GUI framework that works seamlessly across Linux, macOS, and Windows.

## Prerequisites

### On Linux (Ubuntu/Debian)
```bash
# Install required GTK+ development packages
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    pkg-config \
    libgtk-3-dev \
    libgtk-3-0

# These should already be installed, but verify:
sudo apt-get install -y \
    libglib2.0-dev \
    libcairo2-dev \
    libpango1.0-dev
```

## Building

### Standard Build
```bash
make clean
make
```

### Debug Build
```bash
make debug
```

### Run the Application
```bash
make run
# or directly:
./modular-synth
```

## Project Structure with GTK+

The GTK+ integration includes:
- **main.cpp**: GTK+ application entry point with main loop initialization
- **synth_component.h/cpp**: Main GUI component with oscillator, filter, and envelope controls
- **Makefile**: Updated to compile with GTK+ libraries and pkg-config

## Features

The GTK+ GUI provides:
- **Tabbed Interface**: Three main tabs for different sections
  - Oscillator: Frequency, detune, and waveform selection
  - Filter: Cutoff frequency, resonance, and filter type
  - Envelope: Attack, decay, sustain, and release settings
- **Interactive Sliders**: All parameters have visual sliders for easy adjustment
- **Dropdown Menus**: Waveform and filter type selection via combo boxes
- **Status Display**: Real-time status updates and initialization messages
- **Responsive Layout**: Auto-sizing widgets based on window dimensions

## Controls

### Oscillator Tab
- **Frequency**: 20 Hz to 20 kHz (default 440 Hz)
- **Detune**: -1200 to +1200 cents
- **Waveform**: Sine, Square, Sawtooth, Triangle

### Filter Tab
- **Cutoff**: 20 Hz to 20 kHz (default 1000 Hz)
- **Resonance**: 0.1 to 10.0 (default 0.707)
- **Filter Type**: Lowpass, Highpass, Bandpass

### Envelope Tab
- **Attack**: 0.001 to 5.0 seconds (default 0.01s)
- **Decay**: 0.001 to 5.0 seconds (default 0.1s)
- **Sustain**: 0.0 to 1.0 (default 0.7)
- **Release**: 0.001 to 5.0 seconds (default 0.3s)

## Troubleshooting

### GTK+ headers not found
```bash
sudo apt-get install libgtk-3-dev
```

### pkg-config not finding GTK+
Ensure pkg-config is installed and GTK+ development files are properly installed:
```bash
pkg-config --modversion gtk+-3.0
```

### Build errors with g++
Ensure g++ is installed and supports C++17:
```bash
g++ --version  # Should be 7.0+
```

## File Descriptions

- **src/main.cpp**: Entry point that initializes GTK+ and creates the synthesizer component
- **src/plugin/synth_component.h**: Header defining the GUI component class and GTK widgets
- **src/plugin/synth_component.cpp**: Implementation of GUI layout, event handling, and synth initialization
- **Makefile**: Build configuration with GTK+ flags and library linking

## Dependencies

- GTK+ 3.0 or later
- GLib 2.0+
- pkg-config
- GCC 7.0+ (with C++17 support)
- Standard POSIX build tools (make, g++)

## Next Steps

1. Ensure GTK+ development files are installed:
   ```bash
   sudo apt-get install libgtk-3-dev
   ```
2. Run `make clean && make` to build the project
3. Execute `./modular-synth` to launch the GUI
4. Interact with the sliders and dropdowns to adjust synthesizer parameters
5. Check the status display for initialization messages

## Customization

You can easily customize the GUI by:
- Modifying slider ranges in `synth_component.cpp`
- Changing default values in `setupOscillatorSection()`, `setupFilterSection()`, etc.
- Adding more controls by following the existing pattern
- Changing window size and layout in the `SynthComponent` constructor

## Additional Resources

- GTK+ Documentation: https://docs.gtk.org/gtk3/
- GObject Introspection: https://gi.readthedocs.io/
- GTK+ Examples: https://github.com/GNOME/gtk/tree/main/examples
