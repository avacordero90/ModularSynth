#include "oscillator.h"
#include <cmath>

Oscillator::Oscillator() 
    : wavetable(nullptr), phase(0.0f), frequency(440.0f), detune(0.0f) {}

Oscillator::~Oscillator() = default;

void Oscillator::setWavetable(Wavetable* wt) {
    wavetable = wt;
}

void Oscillator::setWavetableName(const std::string& name) {
    wavetableName = name;
}

void Oscillator::setFrequency(float freq) {
    frequency = freq;
}

void Oscillator::setDetune(float cents) {
    detune = cents;
}

float Oscillator::process() {
    if (!wavetable) return 0.0f;
    
    float actualFreq = frequency * powf(2.0f, detune / 1200.0f);
    
    // Calculate phase increment
    float phaseInc = actualFreq / 44100.0f; // Assuming 44.1kHz sample rate
    
    // Get current sample from wavetable
    float output = wavetable->getInterpolatedValue(phase * static_cast<float>(wavetable->getLength()));
    
    // Update phase
    phase += phaseInc;
    if (phase >= 1.0f) {
        phase -= 1.0f;
    }
    
    return output;
}

void Oscillator::processBlock(float* output, size_t bufferSize) {
    if (!wavetable) return;
    
    float actualFreq = frequency * powf(2.0f, detune / 1200.0f);
    float phaseInc = actualFreq / 44100.0f;
    
    for (size_t i = 0; i < bufferSize; ++i) {
        output[i] = wavetable->getInterpolatedValue(phase * static_cast<float>(wavetable->getLength()));
        phase += phaseInc;
        if (phase >= 1.0f) {
            phase -= 1.0f;
        }
    }
}

void Oscillator::setPhase(float p) {
    phase = p;
}

void Oscillator::incrementPhase(float inc) {
    phase += inc;
    if (phase >= 1.0f) {
        phase -= 1.0f;
    }
}

float Oscillator::getFrequency() const {
    return frequency;
}

float Oscillator::getPhase() const {
    return phase;
}

Wavetable* Oscillator::getWavetable() const {
    return wavetable;
}

void Oscillator::reset() {
    phase = 0.0f;
}