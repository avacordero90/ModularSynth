# Modular Wavetable Synth Plugin Design

## Plugin Interface Overview

This modular wavetable synth is designed as a flexible, extensible audio plugin with multiple interconnected modules that can be controlled through parameter interfaces. The plugin follows a modular architecture where each module (oscillator, filter, envelope) operates independently but can be connected in various configurations.

## Parameter Definitions by Module

### 1. Oscillator Parameters

**Control Type**: Sliders and Combo Boxes

| Parameter | Min Value | Max Value | Default | Modulatable | UI Control |
|-----------|-----------|-----------|---------|-------------|------------|
| Oscillator Frequency | 20 Hz | 20,000 Hz | 440 Hz | Yes | Slider (with Hz display) |
| Oscillator Detune | -1200 cents | 1200 cents | 0 cents | Yes | Slider (with cents display) |
| Oscillator Waveform | 0-5 | 0-5 | 0 | No | Combo Box (Sine, Square, Sawtooth, Triangle, etc.) |

### 2. Filter Parameters

**Control Type**: Sliders and Combo Boxes

| Parameter | Min Value | Max Value | Default | Modulatable | UI Control |
|-----------|-----------|-----------|---------|-------------|------------|
| Filter Frequency | 20 Hz | 20,000 Hz | 1000 Hz | Yes | Slider (with Hz display) |
| Filter Resonance | 0.1 | 10.0 | 0.707 | Yes | Slider (Q factor) |
| Filter Type | 0-7 | 0-7 | 0 | No | Combo Box (Lowpass, Highpass, Bandpass, Notch, Peaking, Lowshelf, Hiself) |

### 3. Envelope Parameters

**Control Type**: Sliders (Time-based)

| Parameter | Min Value | Max Value | Default | Modulatable | UI Control |
|-----------|-----------|-----------|---------|-------------|------------|
| Attack Time | 0.001 s | 5.0 s | 0.01 s | Yes | Slider (with seconds display) |
| Decay Time | 0.001 s | 5.0 s | 0.1 s | Yes | Slider (with seconds display) |
| Sustain Level | 0.0 | 1.0 | 0.7 | Yes | Slider (percentage) |
| Release Time | 0.001 s | 5.0 s | 0.3 s | Yes | Slider (with seconds display) |

### 4. Wavetable Parameters

**Control Type**: Combo Boxes and Sliders

| Parameter | Min Value | Max Value | Default | Modulatable | UI Control |
|-----------|-----------|-----------|---------|-------------|------------|
| Wavetable Selection | 0 | N | 0 | No | Combo Box (wavetable list) |
| Wavetable Morph | 0.0 | 1.0 | 0.5 | Yes | Slider (morph between wavetables) |

## Control Types in UI

### 1. Sliders
- **Purpose**: Continuous parameter control with fine modulation capability
- **Use Cases**: Frequency, resonance, time-based parameters, morph amount
- **Display Format**: 
  - Values shown with appropriate units (Hz, cents, s, Q, %)
  - Fine-grained control with logarithmic scaling for frequency parameters

### 2. Knobs
- **Purpose**: Rotational controls for parameters that benefit from circular interaction
- **Use Cases**: Filter resonance, oscillator detune, envelope shape parameters
- **Visual Design**: Circular UI elements with value indicators

### 3. Switches
- **Purpose**: Binary on/off toggles
- **Use Cases**: Filter bypass, modulation enable/disable, waveform selection (for specific modes)
- **Visual Design**: LED-style toggles with clear on/off states

### 4. Combo Boxes
- **Purpose**: Selection between discrete choices
- **Use Cases**: Filter type, oscillator waveform, wavetable selection
- **Visual Design**: Dropdown menus with visual icons or labels for each option

### 5. Buttons
- **Purpose**: Momentary action triggers
- **Use Cases**: Reset parameters, generate new wavetables, play/stop sequences
- **Visual Design**: Push-button style with click feedback

## UI Layout and Organization

The plugin UI is organized by module types to improve workflow and reduce cognitive load:

### Oscillator Section (Top)
- Frequency control (slider)
- Detune control (slider)
- Waveform selection (combo box)

### Filter Section (Middle)
- Cutoff frequency (slider)  
- Resonance (slider)
- Type selection (combo box)

### Envelope Section (Bottom)
- Attack time (slider)
- Decay time (slider)
- Sustain level (slider)
- Release time (slider)

### Wavetable Section
- Selection controls for current wavetable (combo box)
- Morphing control (slider) between wavetables

## Parameter Mapping to Audio Modules

Parameters are mapped to audio modules through the plugin interface:

```cpp
void PluginInterface::mapParametersToModules() {
    // Example process:
    
    // 1. Oscillator Parameters
    oscillators[0]->setFrequency(getParameterValue("Oscillator Frequency"));
    oscillators[0]->setDetune(getParameterValue("Oscillator Detune"));
    
    // 2. Filter Parameters  
    filters[0]->setFrequency(getParameterValue("Filter Frequency"));
    filters[0]->setResonance(getParameterValue("Filter Resonance"));
    filters[0]->setType(static_cast<FilterType>(getParameterValue("Filter Type")));
    
    // 3. Envelope Parameters
    envelopes[0]->setAttackTime(getParameterValue("Attack Time"));
    envelopes[0]->setDecayTime(getParameterValue("Decay Time"));
    envelopes[0]->setSustainLevel(getParameterValue("Sustain Level"));
    envelopes[0]->setReleaseTime(getParameterValue("Release Time"));
}
```

## Wavetable Management System

### Built-in Waveforms
- Sine wave generation
- Square wave generation  
- Sawtooth wave generation
- Triangle wave generation
- 5th harmonics variations

### User Wavetable Support
- Load wavetables from audio files
- Save modified wavetables
- Generate new customized waveform shapes
- Morph between different wavetables for smooth transitions

## Modulation and Automation

The plugin supports full modulation capabilities:
- All continuous parameters are modulatable
- Parameters can be automated via DAW automation lanes
- MIDI CC support for real-time parameter changes
- Sample-accurate modulation updates during audio processing

## Performance Characteristics

The modular architecture allows for:
- Efficient processor utilization through proper module routing
- Memory optimization with shared wavetable resources
- Real-time parameter updates without audio glitches
- Extensible design for additional modules (LFO, delay, reverb, etc.)