# Modular Wavetable Synth Plugin

A flexible, modular audio plugin designed for virtual studio environments. The synth provides multiple interconnected audio modules (oscillator, filter, envelope) with extensive parameter control and wavetable management.

## Project Structure

```
VstProject/
├── src/
│   ├── core/                # Core audio processing components
│   │   ├── audio_pipeline.h/cpp   # Audio module routing and processing pipeline
│   │   ├── wavetable.h/cpp        # Wavetable generation and sampling
│   │   └── wavetable_manager.h/cpp # Wavetable storage and management
│   ├── modules/             # Individual audio modules
│   │   ├── oscillator.h/cpp       # Oscillator with multiple waveform types
│   │   ├── filter.h/cpp           # Audio filtering with multiple filter types
│   │   └── envelope.h/cpp         # Envelope generation (ADSR)
│   ├── plugin/              # Plugin interface and VST implementation
│   │   ├── plugin_interface.h/cpp # Base plugin functionality and parameters
│   │   └── vst_plugin.h/cpp       # VST-specific implementation
├── docs/                    # Documentation and design files
│   └── PLUGIN_DESIGN.md     # Detailed plugin design and control specifications
└── README.md                # This file
```

## Core Architecture

### Modularity
The synth is built on a modular architecture where:
- Each audio module (oscillator, filter, envelope) operates independently
- Modules can be connected in various configurations through the audio pipeline
- Wavetable resources are shared to minimize memory usage

### Audio Pipeline
The `AudioPipeline` class manages connections between modules and handles:
- Module allocation and initialization
- Routing of signals between components
- Batch processing of audio blocks
- Configuration management for sample rate and buffer size

### Wavetable System
The wavetable manager provides:
- Built-in waveform generation (sine, square, sawtooth, triangle)
- Custom wavetable loading and saving
- Wavetable morphing capabilities
- Shared resource management for efficient memory use

## Plugin Interface

The plugin interface (`PluginInterface`) defines:
- All available parameters with min/max/default values
- Parameter categories (oscillator, filter, envelope, wavetable)
- Control types (sliders, combo boxes, etc.)
- UI layout specifications
- Wavetable management methods
- Parameter mapping to actual audio modules

### Parameters by Module

#### Oscillator
- Frequency: 20 Hz to 20,000 Hz (modulatable)
- Detune: -1200 cents to 1200 cents (modulatable)  
- Waveform: 0-5 selection (combo box)

#### Filter
- Cutoff Frequency: 20 Hz to 20,000 Hz (modulatable)
- Resonance: 0.1 to 10.0 Q factor (modulatable)
- Filter Type: 8 different filter types (combo box)

#### Envelope
- Attack Time: 0.001 s to 5.0 s (modulatable)
- Decay Time: 0.001 s to 5.0 s (modulatable)
- Sustain Level: 0.0 to 1.0 (modulatable)
- Release Time: 0.001 s to 5.0 s (modulatable)

#### Wavetable
- Selection: Combo box with wavetable list
- Morph: 0.0 to 1.0 (modulatable)

## Controls and UI Design

The plugin features a well-organized UI that groups controls by functionality:
- **Oscillator Section**: Frequency, detune, waveform selection
- **Filter Section**: Cutoff, resonance, type selection  
- **Envelope Section**: Attack, decay, sustain, release times
- **Wavetable Section**: Wavetable selection and morph control

### Control Types
- **Sliders**: Fine-grained continuous controls with proper scaling
- **Combo Boxes**: Discrete choice selections for filter types, waveforms
- **Knobs**: Rotational controls optimized for resonance and detune
- **Buttons**: Momentary actions for reset operations

## Compilation Requirements

To build this project:
- C++14 or later compiler  
- Standard C++ libraries
- Audio development tools (for VST integration)
- CMake or compatible build system

## Usage Notes

1. Initialize the plugin before use to load wavetables and set up modules
2. Use parameter automation for dynamic control during playback
3. Combine different modules in various routing patterns for unique sounds
4. Modify wavetables using the wavetable manager for custom waveform shapes
5. Use modulation sources (LFOs, envelopes) for evolving parameter changes

## Extensibility

The modular architecture supports easy extension:
- Add new audio modules by implementing base class interfaces
- Extend parameter definitions to include new control types
- Integrate additional UI controls through the layout management system
- Implement custom wavetable generation algorithms

## Future Enhancements

Planned improvements include:
- LFO module for modulation sources
- Delay and reverb effects modules
- Step sequencer capabilities  
- MIDI controller integration
- Multi-timbral support
- Advanced filter types (state-variable, notch, etc.)

## License

This project is intended for educational and development purposes.

---
*Created with C++ and modular audio design principles*