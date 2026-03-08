# Modular Wavetable Synth Standalone

A flexible, modular audio synthesizer application with a GTK‑based GUI. It is no longer built as a plugin; instead the program runs standalone on the desktop, providing the same modular audio modules (oscillator, filter, envelope) and wavetable management.

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
│   ├── app/                 # Application GUI and entry point (formerly plugin folder)
│   │   ├── synth_component.h/cpp # GTK GUI wrapper and synth initialization
│   │   ├── midi_handler.h/cpp     # MIDI message processing (legacy)
│   │   └── others (legacy plugin code)
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

## Application Structure

The application exposes the synth engine through a GTK GUI. Parameters are managed directly in the GTK widgets and mapped into the audio modules in `SynthComponent`.

(The previous `PluginInterface` code is retained in the `app/` directory for reference but is no longer compiled or used.)

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

## MIDI Integration

The standalone synthesizer can optionally use the `MidiHandler` component for MIDI input. MIDI code resides in `app/midi_handler.h/cpp` and is currently legacy but may be re‑enabled or extended in the future.

### Supported MIDI Messages:
- Note On/Off events with velocity control
- Control Change messages for real-time parameter adjustment
- Pitch Bend messages for continuous pitch modulation
- MIDI Clock synchronization for tempo matching
- Start/Continue/Stop sequence commands

### Connecting MIDI Hardware:
1. **Connect MIDI Controller**: Use standard USB MIDI cables or Bluetooth MIDI adapters to connect your controller
2. **Configure Your DAW**:
   - Set up your audio host (DAW) to recognize the plugin as a MIDI input device
   - Create routing between your MIDI controller and the synth plugin
3. **Plugin Setup**:
   - Enable MIDI input within the synth's user interface
   - Configure which MIDI channels to listen on (or all channels)
   - Set up channel-specific parameter mappings if desired

### Using MIDI to Play Sounds:
1. Configure your MIDI keyboard or controller to send Note On/Off messages
2. Play notes using the keyboard or pad controller
3. Use CC controllers for real-time modulation of oscillator frequency, filter cutoff, envelope parameters, and other controls
4. Use pitch bend wheel for continuous pitch control
5. Utilize aftertouch for expressive sound shaping

### MIDI Parameter Mapping:
The synth implements a flexible parameter mapping system that allows users to assign any MIDI controller to any synth parameter. By default, the following mappings are configured:

- CC 1: Modulation Wheel (default)
- CC 7: Volume (default)
- CC 11: Aftertouch (default)
- CC 64: Sustain Pedal (default)

Users can modify these mappings through the synthesizer's parameter configuration interface.

#### Available MIDI Message Types:
- **Note On/Off** (0x90/0x80): Play notes with velocity-sensitive triggering
- **Control Change** (0xB0): Real-time modulation of parameters using CC controllers
- **Pitch Bend** (0xE0): Continuous pitch changes from -12 to +12 semitones
- **Clock Messages**: 
  - MIDI Clock (0xF8): For synchronization with DAWs
  - Start (0xFA): Begin playback sequence
  - Continue (0xFB): Resume playback
  - Stop (0xFC): End playback sequence

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
- GTK development headers (for GUI components)

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