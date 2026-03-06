#include "vst_plugin.h"
#include "../core/audio_pipeline.h"
#include <iostream>

VSTPlugin::VSTPlugin() : PluginInterface(), isInitialized(false) {
    // Initialize VST-specific members
}

VSTPlugin::~VSTPlugin() {
    cleanup();
}

void VSTPlugin::setParameterValue(int paramIndex, float value) {
    // This method would handle parameter changes from the host
    // In this example, we'll just print to show it's working
    std::cout << "Setting parameter " << paramIndex << " to " << value << std::endl;
    
    // In a real implementation, this would update either:
    // - Module parameters directly 
    // - Or set some internal control state
}

float VSTPlugin::getParameterValue(int paramIndex) const {
    // Return current value of parameter
    return 0.0f; // Placeholder
}

void VSTPlugin::initialize() {
    if (isInitialized) return;
    
    std::cout << "Initializing VST plugin..." << std::endl;
    
    // Initialize audio pipeline here
    if (!pipeline) {
        std::cerr << "Error: No audio pipeline set!" << std::endl;
        return;
    }
    
    isInitialized = true;
    std::cout << "VST plugin initialized." << std::endl;
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
    isInitialized = false;
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
    if (pipeline) {
        std::cout << "Configuring audio I/O: " << numInputs << " inputs, " 
                  << numOutputs << " outputs at " << sampleRate << " Hz" << std::endl;
    }
}