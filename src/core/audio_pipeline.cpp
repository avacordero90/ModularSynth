#include "audio_pipeline.h"
#include <cstring>

AudioPipeline::AudioPipeline(WavetableManager* wm, float sampleRate) 
    : wavetableManager(wm), sampleRate(sampleRate), bufferSize(64) {}

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

void AudioPipeline::setBufferSize(size_t size) {
    bufferSize = size;
}