#include "oscillator.h"
#include <cmath>

// Initialize oscillator with neutral defaults.
Oscillator::Oscillator() 
    : wavetable(nullptr), phase(0.0f), frequency(440.0f), detune(0.0f), sampleRate(44100.0f) {}

// Oscillator does not own wavetable memory.
Oscillator::~Oscillator() = default;

// Assign externally managed wavetable storage.
void Oscillator::setWavetable(Wavetable* wt) {
    wavetable = wt;
}

// Keep a human-readable wavetable identifier for UI/state.
void Oscillator::setWavetableName(const std::string& name) {
    wavetableName = name;
}

// Set base oscillator frequency in Hz.
void Oscillator::setFrequency(float freq) {
    frequency = freq;
}

// Set detune offset in cents.
void Oscillator::setDetune(float cents) {
    detune = cents;
}

// Accept realistic audio sample rates only.
void Oscillator::setSampleRate(float rate) {
    if (rate > 1000.0f) {
        sampleRate = rate;
    }
}

// Generate one sample and advance normalized phase.
float Oscillator::process() {
    if (!wavetable) return 0.0f;
    
    float actualFreq = frequency * powf(2.0f, detune / 1200.0f);
    
    // Calculate phase increment
    float phaseInc = actualFreq / sampleRate;
    
    // Get current sample from wavetable
    float output = wavetable->getInterpolatedValue(phase * static_cast<float>(wavetable->getLength()));
    
    // Update phase
    phase += phaseInc;
    if (phase >= 1.0f) {
        phase -= 1.0f;
    }
    
    return output;
}

// Render a contiguous block of oscillator samples.
void Oscillator::processBlock(float* output, size_t bufferSize) {
    if (!wavetable) return;
    
    float actualFreq = frequency * powf(2.0f, detune / 1200.0f);
    float phaseInc = actualFreq / sampleRate;
    
    for (size_t i = 0; i < bufferSize; ++i) {
        output[i] = wavetable->getInterpolatedValue(phase * static_cast<float>(wavetable->getLength()));
        phase += phaseInc;
        if (phase >= 1.0f) {
            phase -= 1.0f;
        }
    }
}

// Force oscillator phase to a specific normalized value.
void Oscillator::setPhase(float p) {
    phase = p;
}

// Increment phase and wrap into [0, 1).
void Oscillator::incrementPhase(float inc) {
    phase += inc;
    if (phase >= 1.0f) {
        phase -= 1.0f;
    }
}

// Apply a static phase shift to the current wavetable.
void Oscillator::modulateWavetablePhase(float phaseOffset) {
    if (wavetable) {
        wavetable->modulatePhase(phaseOffset);
    }
}

// Scale wavetable amplitude in-place.
void Oscillator::modulateWavetableAmplitude(float amplitudeFactor) {
    if (wavetable) {
        wavetable->modulateAmplitude(amplitudeFactor);
    }
}

// Multiply wavetable by a provided envelope curve.
void Oscillator::applyWavetableEnvelopes(const std::vector<float>& envelope) {
    if (wavetable) {
        wavetable->applyEnvelope(envelope);
    }
}

// Inject random noise into wavetable data.
void Oscillator::addWavetableNoise(float intensity) {
    if (wavetable) {
        wavetable->addNoise(intensity);
    }
}

// Getter for base oscillator frequency.
float Oscillator::getFrequency() const {
    return frequency;
}

// Getter for normalized phase.
float Oscillator::getPhase() const {
    return phase;
}

// Getter for active wavetable pointer.
Wavetable* Oscillator::getWavetable() const {
    return wavetable;
}

// Getter for processing sample rate.
float Oscillator::getSampleRate() const {
    return sampleRate;
}

// Reset phase to the start of the cycle.
void Oscillator::reset() {
    phase = 0.0f;
}