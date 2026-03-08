#include "audio_pipeline.h"
#include <cstring>
#include <iostream>

AudioPipeline::AudioPipeline(WavetableManager* wm, float sampleRate) 
    : wavetableManager(wm), sampleRate(sampleRate), bufferSize(64), wavetablesInitialized(false) {
    // Initialize default wavetables on construction
    if (wavetableManager) {
        initializeDefaultWavetables();
    }
}

AudioPipeline::~AudioPipeline() = default;

void AudioPipeline::addOscillator(std::unique_ptr<Oscillator> osc) {
    oscillators.push_back(std::move(osc));
}

void AudioPipeline::addFilter(std::unique_ptr<Filter> filt) {
    filters.push_back(std::move(filt));
}

void AudioPipeline::addEnvelope(std::unique_ptr<Envelope> env) {
    envelopes.push_back(std::move(env));
}

void AudioPipeline::connectOscillatorToFilter(size_t oscIndex, size_t filterIndex) {
    // Placeholder for connection logic - in real implementation this would set up routing
}

void AudioPipeline::connectFilterToEnvelope(size_t filterIndex, size_t envIndex) {
    // Placeholder for connection logic - in real implementation this would set up routing
}

void AudioPipeline::connectEnvelopeToOutput(size_t envIndex, float* output, size_t outputIndex) {
    // Placeholder for output connection - in real implementation this would route to final output
}

void AudioPipeline::processBlock(float** inputs, float** outputs, size_t numInputs, size_t numOutputs, size_t bufferSize) {
    // Initialize output buffer
    for (size_t i = 0; i < bufferSize; ++i) {
        outputs[0][i] = 0.0f;
    }
    
    // Process all oscillators and accumulate into output
    for (size_t i = 0; i < oscillators.size(); ++i) {
        if (oscillators[i]) {
            float tempBuffer[64]; // Temporary buffer (using 64 to match internal size)
            
            // Process each oscillator with its own buffer
            oscillators[i]->processBlock(tempBuffer, bufferSize);
            
            // Mix oscillator output into main output
            for (size_t j = 0; j < bufferSize && j < 64; ++j) {
                outputs[0][j] += tempBuffer[j];
            }
        }
    }
    
    // Apply filters and envelopes to the mix
    for (size_t i = 0; i < filters.size(); ++i) {
        if (filters[i]) {
            // Apply filter - placeholder for actual filtering logic
        }
    }
    
    for (size_t i = 0; i < envelopes.size(); ++i) {
        if (envelopes[i]) {
            // Apply envelope - placeholder for actual envelope processing
        }
    }
}

WavetableManager* AudioPipeline::getWavetableManager() const {
    return wavetableManager;
}

float AudioPipeline::getSampleRate() const {
    return sampleRate;
}

size_t AudioPipeline::getBufferSize() const {
    return bufferSize;
}

void AudioPipeline::setBufferSize(size_t size) {
    bufferSize = size;
}

void AudioPipeline::initializeDefaultWavetables() {
    if (!wavetableManager) return;
    
    // Generate standard waveforms
    wavetableManager->generateSineWave("sine", 2048);
    wavetableManager->generateSquareWave("square", 2048);
    wavetableManager->generateSawtoothWave("sawtooth", 2048);
    wavetableManager->generateTriangleWave("triangle", 2048);
    
    wavetablesInitialized = true;
    std::cout << "Default wavetables initialized: sine, square, sawtooth, triangle" << std::endl;
}

void AudioPipeline::generateWavetables() {
    if (!wavetableManager) return;
    
    // Generate additional wavetables as needed
    // This can be extended to generate more complex wavetables
}

void AudioPipeline::configureOscillatorsWithWavetables() {
    if (!wavetableManager || !wavetablesInitialized) return;
    
    // Set each oscillator to use the sine waveform by default
    for (size_t i = 0; i < oscillators.size(); ++i) {
        if (oscillators[i]) {
            Wavetable* sineWavetable = wavetableManager->getWavetable("sine");
            if (sineWavetable) {
                oscillators[i]->setWavetable(sineWavetable);
                oscillators[i]->setFrequency(440.0f + (i * 100.0f)); // Slight variation in frequency
                std::cout << "Configured oscillator " << i << " with sine wavetable at " 
                          << (440.0f + (i * 100.0f)) << " Hz" << std::endl;
            }
        }
    }
}

void AudioPipeline::setOscillatorWavetable(size_t oscIndex, const std::string& wavetableName) {
    if (oscIndex >= oscillators.size() || !wavetableManager) return;
    
    if (oscillators[oscIndex]) {
        Wavetable* wavetable = wavetableManager->getWavetable(wavetableName);
        if (wavetable) {
            oscillators[oscIndex]->setWavetable(wavetable);
            oscillators[oscIndex]->setWavetableName(wavetableName);
            std::cout << "Set oscillator " << oscIndex << " to wavetable: " << wavetableName << std::endl;
        }
    }
}

void AudioPipeline::setOscillatorFrequency(size_t oscIndex, float frequency) {
    if (oscIndex >= oscillators.size()) return;
    
    if (oscillators[oscIndex]) {
        oscillators[oscIndex]->setFrequency(frequency);
    }
}

<<<<<<< HEAD
void AudioPipeline::setOscillatorDetune(size_t oscIndex, float cents) {
    if (oscIndex >= oscillators.size()) return;
    if (oscillators[oscIndex]) {
        oscillators[oscIndex]->setDetune(cents);
    }
}

bool AudioPipeline::areWavetablesInitialized() const {
    return wavetablesInitialized;
}

// Monophonic helpers used by the sequencer/MIDI subsystem
void AudioPipeline::noteOn(float frequency) {
    if (!oscillators.empty()) {
        oscillators[0]->setFrequency(frequency);
    }
    if (!envelopes.empty()) {
        envelopes[0]->triggerAttack();
    }
}

void AudioPipeline::noteOff() {
    if (!envelopes.empty()) {
        envelopes[0]->triggerRelease();
    }
=======
bool AudioPipeline::areWavetablesInitialized() const {
    return wavetablesInitialized;
>>>>>>> 5ffc43519522ce0e9298fdc8c58162ba0a4e9e39
}