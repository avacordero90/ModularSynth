#include "vst_plugin.h"
#include "../core/audio_pipeline.h"
#include <iostream>

VSTPlugin::VSTPlugin() : PluginInterface(), isInitialized(false) {
    // Initialize VST-specific members
    vstEffect = nullptr;
    processor = nullptr;
    setupParameters();
}

VSTPlugin::~VSTPlugin() {
    cleanup();
}

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

void VSTPlugin::processAudio(float** inputs, float** outputs, size_t numInputs, size_t numOutputs, size_t bufferSize) {
    if (!isInitialized || !pipeline) return;
    
    // Process audio using the pipeline
    pipeline->processBlock(inputs, outputs, numInputs, numOutputs, bufferSize);
    
    // In a full implementation, this would also handle:
    // - MIDI events
    // - Parameter modulation
    // - Real-time control updates
}

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

void VSTPlugin::setVstEffect(AudioEffect* effect) {
    vstEffect = std::unique_ptr<AudioEffect>(effect);
}

void VSTPlugin::setProcessor(AudioProcessor* proc) {
    processor = std::unique_ptr<AudioProcessor>(proc);
}

const char* VSTPlugin::getName() const {
    return "Modular Wavetable Synth";
}

const char* VSTPlugin::getVendor() const {
    return "ModularAudio Inc.";
}

int VSTPlugin::getVersion() const {
    return 1000; // Version 1.0.0
}

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