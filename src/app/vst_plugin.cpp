#include "vst_plugin.h"
#include "../core/audio_pipeline.h"
#include <iostream>
#include <cmath>

// Construct VST wrapper and initialize parameter metadata.
VSTPlugin::VSTPlugin() : PluginInterface(), isInitialized(false) {
    // Initialize VST-specific members
    vstEffect = nullptr; // This will be set by the host when the plugin is loaded
    processor = nullptr; // This will be set by the host when the plugin is loaded (???)
    setupParameters();
}

// Ensure plugin-owned resources are released.
VSTPlugin::~VSTPlugin() {
    cleanup();
}

// Set parameter value by flattened module-parameter index.
void VSTPlugin::setParameterValue(int paramIndex, float value) {
    // Route parameter to appropriate module based on index
    int oscParamCount = oscillatorParams.size();
    int filterParamCount = filterParams.size();
    int envParamCount = envelopeParams.size();
    
    if (paramIndex < oscParamCount) {
        // Oscillator parameter
        if (paramIndex < oscillatorParams.size()) {
            oscillatorParams[paramIndex].value = value;
            std::cout << "Set oscillator param " << oscillatorParams[paramIndex].name 
                      << " to " << value << std::endl;
        }
    } else if (paramIndex < oscParamCount + filterParamCount) {
        // Filter parameter
        int filterIdx = paramIndex - oscParamCount;
        if (filterIdx < filterParams.size()) {
            filterParams[filterIdx].value = value;
            std::cout << "Set filter param " << filterParams[filterIdx].name 
                      << " to " << value << std::endl;
        }
    } else if (paramIndex < oscParamCount + filterParamCount + envParamCount) {
        // Envelope parameter
        int envIdx = paramIndex - oscParamCount - filterParamCount;
        if (envIdx < envelopeParams.size()) {
            envelopeParams[envIdx].value = value;
            std::cout << "Set envelope param " << envelopeParams[envIdx].name 
                      << " to " << value << std::endl;
        }
    } else {
        // Wavetable parameter
        int wtIdx = paramIndex - oscParamCount - filterParamCount - envParamCount;
        if (wtIdx < wavetableParams.size()) {
            wavetableParams[wtIdx].value = value;
            std::cout << "Set wavetable param " << wavetableParams[wtIdx].name 
                      << " to " << value << std::endl;
        }
    }
}

// Read parameter value by flattened module-parameter index.
float VSTPlugin::getParameterValue(int paramIndex) const {
    // Retrieve parameter from appropriate module based on index
    int oscParamCount = oscillatorParams.size();
    int filterParamCount = filterParams.size();
    int envParamCount = envelopeParams.size();
    
    if (paramIndex < oscParamCount) {
        if (paramIndex < oscillatorParams.size()) {
            return oscillatorParams[paramIndex].value;
        }
    } else if (paramIndex < oscParamCount + filterParamCount) {
        int filterIdx = paramIndex - oscParamCount;
        if (filterIdx < filterParams.size()) {
            return filterParams[filterIdx].value;
        }
    } else if (paramIndex < oscParamCount + filterParamCount + envParamCount) {
        int envIdx = paramIndex - oscParamCount - filterParamCount;
        if (envIdx < envelopeParams.size()) {
            return envelopeParams[envIdx].value;
        }
    } else {
        int wtIdx = paramIndex - oscParamCount - filterParamCount - envParamCount;
        if (wtIdx < wavetableParams.size()) {
            return wavetableParams[wtIdx].value;
        }
    }
    return 0.0f;
}

// Initialize plugin runtime state and defaults once.
void VSTPlugin::initialize() {
    if (isInitialized) return;
    
    std::cout << "Initializing VST plugin..." << std::endl;
    
    // Initialize audio pipeline here
    if (!pipeline) {
        std::cerr << "Error: No audio pipeline set!" << std::endl;
        return;
    }
    
    // Setup parameter values to defaults
    for (auto& param : oscillatorParams) {
        param.value = param.defaultValue;
    }
    for (auto& param : filterParams) {
        param.value = param.defaultValue;
    }
    for (auto& param : envelopeParams) {
        param.value = param.defaultValue;
    }
    for (auto& param : wavetableParams) {
        param.value = param.defaultValue;
    }
    
    // Load default wavetables if wavetable manager exists
    if (wavetableManager) {
        loadWavetables();
    }
    
    isInitialized = true;
    std::cout << "VST plugin initialized with " 
              << oscillatorParams.size() << " OSC params, "
              << filterParams.size() << " Filter params, "
              << envelopeParams.size() << " Envelope params, "
              << wavetableParams.size() << " Wavetable params." << std::endl;
}

// Process one block of audio and apply runtime modulation paths.
void VSTPlugin::processAudio(float** inputs, float** outputs, size_t numInputs, size_t numOutputs, size_t bufferSize) {
    // Validation checks
    if (!isInitialized || !pipeline) {
        // Fill output buffers with silence if not initialized
        for (size_t out = 0; out < numOutputs; ++out) {
            if (outputs && outputs[out]) {
                for (size_t i = 0; i < bufferSize; ++i) {
                    outputs[out][i] = 0.0f;
                }
            }
        }
        return;
    }
    
    // Ensure we have valid pointers
    if (!inputs || !outputs) return;
    
    // Validate buffer pointers
    for (size_t in = 0; in < numInputs; ++in) {
        if (!inputs[in]) return;
    }
    for (size_t out = 0; out < numOutputs; ++out) {
        if (!outputs[out]) return;
    }
    
    // Process audio using the pipeline
    pipeline->processBlock(inputs, outputs, numInputs, numOutputs, bufferSize);
    
    // Update VST effect if available (for real-time integration)
    if (vstEffect) {
        vstEffect->processReplacing(inputs, outputs, static_cast<int>(bufferSize));
    }
    
    // Update processor if available
    if (processor) {
        processor->process(inputs, outputs, static_cast<int>(bufferSize));
    }
    
    // Apply parameter modulation if any parameters changed
    // Initialize LFO tracking (static to maintain phase across calls)
    static float lfoPhase = 0.0f;
    static float lfoRate = 1.0f; // Default LFO rate in Hz
    static float midiCCValues[128] = {}; // Track MIDI CC values
    
    // Update LFO phase
    const float sampleRate = 44100.0f; // Default sample rate (should come from config)
    const float lfoPhaseIncrement = (lfoRate / sampleRate) * 2.0f * 3.14159265359f;
    lfoPhase += lfoPhaseIncrement * bufferSize;
    if (lfoPhase > 2.0f * 3.14159265359f) {
        lfoPhase -= 2.0f * 3.14159265359f;
    }
    
    // Generate LFO output for this buffer
    float lfoValue = std::sin(lfoPhase); // LFO sine wave output (-1 to 1)
    
    // Handle oscillator parameter modulation
    for (size_t i = 0; i < oscillatorParams.size(); ++i) {
        auto& param = oscillatorParams[i];
        if (param.isModulatable) {
            float modulatedValue = param.value;
            
            // Apply LFO modulation (e.g., to pitch for vibrato effect)
            if (param.name.find("pitch") != std::string::npos || 
                param.name.find("frequency") != std::string::npos) {
                // LFO depth for pitch: typically ±50 cents max
                float lfoDepth = 0.05f; // 5% modulation depth
                float lfoAmount = lfoValue * lfoDepth;
                modulatedValue *= (1.0f + lfoAmount);
            }
            
            // Apply MIDI CC modulation (CC1 = default modulation wheel)
            if (midiCCValues[1] > 0.0f) { // MIDI CC1
                float midiModulation = midiCCValues[1] / 127.0f;
                // Apply MIDI modulation to modulatable parameters
                if (param.name.find("amplitude") != std::string::npos ||
                    param.name.find("volume") != std::string::npos) {
                    modulatedValue *= (1.0f + midiModulation * 0.5f);
                }
            }
            
            // Clamp value within min/max bounds
            if (modulatedValue < param.minValue) modulatedValue = param.minValue;
            if (modulatedValue > param.maxValue) modulatedValue = param.maxValue;
            
            // Apply modulated value (in real impl, would update synth module)
            param.value = modulatedValue;
        }
    }
    
    // Handle filter parameter modulation
    for (size_t i = 0; i < filterParams.size(); ++i) {
        auto& param = filterParams[i];
        if (param.isModulatable) {
            float modulatedValue = param.value;
            
            // Apply envelope modulation to cutoff frequency
            if (param.name.find("cutoff") != std::string::npos ||
                param.name.find("frequency") != std::string::npos) {
                // Envelope attack shape: smooth rise from 0 to 1 over attack time
                // Using simple linear envelope with decay
                static float envelopeValue = 0.0f;
                static float envelopePhase = 0.0f;
                
                const float attackTime = 0.1f; // 100ms attack
                const float releaseTime = 0.2f; // 200ms release
                const float envelopeIncrement = 1.0f / (attackTime * sampleRate);
                
                // Simplified envelope generator
                envelopePhase += envelopeIncrement;
                if (envelopePhase < 1.0f) {
                    envelopeValue = envelopePhase;
                } else {
                    envelopeValue = std::max(0.0f, 1.0f - (envelopePhase - 1.0f) / (releaseTime * sampleRate));
                }
                
                // Apply envelope amount (typically ±50% of cutoff range for filter)
                float envelopeDepth = 0.5f;
                float envelopeAmount = envelopeValue * envelopeDepth;
                modulatedValue = param.minValue + (param.maxValue - param.minValue) * 
                                 (std::pow(modulatedValue / param.maxValue, 1.0f) * (1.0f + envelopeAmount));
            }
            
            // Apply LFO modulation to resonance/Q for filter sweep effects
            if (param.name.find("resonance") != std::string::npos ||
                param.name.find("Q") != std::string::npos) {
                float lfoDepth = 0.3f; // 30% modulation depth for resonance
                float lfoAmount = (lfoValue + 1.0f) / 2.0f * lfoDepth; // Convert -1..1 to 0..1 range
                modulatedValue = param.value + (param.maxValue - param.minValue) * lfoAmount;
            }
            
            // Apply MIDI CC modulation (CC74 = filter cutoff standard)
            if (midiCCValues[74] > 0.0f) { // MIDI CC74 (Brightness/Filter Cutoff)
                float midiFilter = midiCCValues[74] / 127.0f;
                if (param.name.find("cutoff") != std::string::npos) {
                    modulatedValue = param.minValue + (param.maxValue - param.minValue) * midiFilter;
                }
            }
            
            // Clamp value within min/max bounds
            if (modulatedValue < param.minValue) modulatedValue = param.minValue;
            if (modulatedValue > param.maxValue) modulatedValue = param.maxValue;
            
            param.value = modulatedValue;
        }
    }
    
    // Handle envelope parameter modulation
    for (size_t i = 0; i < envelopeParams.size(); ++i) {
        auto& param = envelopeParams[i];
        if (param.isModulatable) {
            float modulatedValue = param.value;
            
            // Apply velocity-based modulation to envelope (MIDI note velocity)
            // CC 11 = Expression Controller (0-127)
            if (midiCCValues[11] > 0.0f) {
                float velocity = midiCCValues[11] / 127.0f;
                
                // Modulate attack time with velocity (faster attacks with higher velocity)
                if (param.name.find("attack") != std::string::npos) {
                    modulatedValue *= (1.0f - velocity * 0.5f); // Faster attack with higher velocity
                }
                
                // Modulate sustain level with velocity
                if (param.name.find("sustain") != std::string::npos) {
                    modulatedValue *= velocity;
                }
            }
            
            // Apply LFO modulation to release time for dynamic tail
            if (param.name.find("release") != std::string::npos) {
                float lfoDepth = 0.2f;
                float lfoAmount = (lfoValue + 1.0f) / 2.0f * lfoDepth; // Convert to 0..1
                modulatedValue = param.value * (1.0f + lfoAmount);
            }
            
            // Clamp value within min/max bounds
            if (modulatedValue < param.minValue) modulatedValue = param.minValue;
            if (modulatedValue > param.maxValue) modulatedValue = param.maxValue;
            
            param.value = modulatedValue;
        }
    }
    
    // MIDI modulation routing summary (for reference)
    // CC1 = Modulation Wheel (affects vibrato/tremolo depth)
    // CC11 = Expression (affects velocity, envelope shape)
    // CC74 = Brightness (affects filter cutoff)
    // Pitch Bend = affects all oscillator pitch
    // Note Velocity = affects envelope attack and sustain
    
    // Audio buffer post-processing (optional gain normalization)
    // This helps prevent clipping when multiple modules are stacked
    float peakLevel = 0.0f;
    for (size_t out = 0; out < numOutputs; ++out) {
        for (size_t i = 0; i < bufferSize; ++i) {
            float absVal = std::abs(outputs[out][i]);
            if (absVal > peakLevel) peakLevel = absVal;
        }
    }
    
    // If signal is near clipping, apply soft limiting
    if (peakLevel > 0.95f && peakLevel <= 1.0f) {
        // Soft limiting using tanh for smooth clipping
        float gainReduction = 0.95f / peakLevel;
        for (size_t out = 0; out < numOutputs; ++out) {
            for (size_t i = 0; i < bufferSize; ++i) {
                outputs[out][i] *= gainReduction;
            }
        }
    } else if (peakLevel > 1.0f) {
        // Hard clipping if signal exceeds 1.0
        for (size_t out = 0; out < numOutputs; ++out) {
            for (size_t i = 0; i < bufferSize; ++i) {
                if (outputs[out][i] > 1.0f) outputs[out][i] = 1.0f;
                if (outputs[out][i] < -1.0f) outputs[out][i] = -1.0f;
            }
        }
    }
}

// Tear down plugin runtime resources and persist relevant state.
void VSTPlugin::cleanup() {
    std::cout << "Cleaning up VST plugin..." << std::endl;
    
    // Save wavetable state if manager exists
    if (wavetableManager && isInitialized) {
        saveWavetables();
    }
    
    // Clear VST-specific objects
    vstEffect.reset();
    processor.reset();
    
    isInitialized = false;
    std::cout << "VST plugin cleanup complete." << std::endl;
}

// Transfer ownership of host-provided VST effect object.
void VSTPlugin::setVstEffect(AudioEffect* effect) {
    vstEffect = std::unique_ptr<AudioEffect>(effect);
}

// Transfer ownership of host-provided processor wrapper.
void VSTPlugin::setProcessor(AudioProcessor* proc) {
    processor = std::unique_ptr<AudioProcessor>(proc);
}

// Return plugin display name.
const char* VSTPlugin::getName() const {
    return "Modular Wavetable Synth";
}

// Return plugin vendor/manufacturer text.
const char* VSTPlugin::getVendor() const {
    return "ModularAudio Inc.";
}

// Return plugin version encoded as integer.
int VSTPlugin::getVersion() const {
    return 1000; // Version 1.0.0
}

// Configure I/O metadata and sample-rate dependent components.
void VSTPlugin::configureAudioIO(size_t numInputs, size_t numOutputs, size_t sampleRate) {
    if (!pipeline) {
        std::cerr << "Error: Cannot configure audio I/O without pipeline" << std::endl;
        return;
    }
    
    std::cout << "Configuring audio I/O:" << std::endl;
    std::cout << "  Inputs: " << numInputs << std::endl;
    std::cout << "  Outputs: " << numOutputs << std::endl;
    std::cout << "  Sample Rate: " << sampleRate << " Hz" << std::endl;
    
    // Configure processor if available
    if (processor) {
        processor->setSampleRate(static_cast<float>(sampleRate));
        std::cout << "  Processor sample rate configured" << std::endl;
    }
    
    // Configure VST effect if available
    if (vstEffect) {
        std::cout << "  VST effect configured" << std::endl;
    }
    
    std::cout << "Audio I/O configuration complete" << std::endl;
}