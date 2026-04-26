#include "filter.h"
#include <cmath>

// Initialize filter state with default type and coefficients context.
Filter::Filter(FilterType type) 
    : type(type), frequency(1000.0f), resonance(0.707f), gain(0.0f), sampleRate(44100.0f) {
    // Initialize filter state
    x1.resize(2, 0.0f);
    x2.resize(2, 0.0f);
    y1.resize(2, 0.0f);
    y2.resize(2, 0.0f);
}

// Defaulted destructor; vectors self-manage.
Filter::~Filter() = default;

// Select filter topology.
void Filter::setType(FilterType newType) {
    type = newType;
}

// Clamp and set cutoff frequency.
void Filter::setFrequency(float freq) {
    frequency = std::max(20.0f, std::min(freq, sampleRate * 0.49f));
}

// Clamp and set resonance/Q value.
void Filter::setResonance(float res) {
    resonance = std::max(0.01f, std::min(res, 10.0f));
}

// Set gain used by shelving/peaking filter variants.
void Filter::setGain(float g) {
    gain = g;
}

// Set sample rate used for coefficient calculations.
void Filter::setSampleRate(float rate) {
    sampleRate = rate;
}

// Process one sample through selected biquad formulation.
float Filter::process(float input) {
    float output = 0.0f;
    
    // Calculate filter coefficients
    float omega = 2.0f * M_PI * frequency / sampleRate;
    float sn = sinf(omega);
    float cs = cosf(omega);
    float alpha = sn / (2.0f * resonance);
    
    float a0, a1, a2, b0, b1, b2;
    
    switch (type) {
        case FilterType::LOWPASS:
            b0 = (1.0f - cs) / 2.0f;
            b1 = 1.0f - cs;
            b2 = (1.0f - cs) / 2.0f;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cs;
            a2 = 1.0f - alpha;
            break;
            
        case FilterType::HIGHPASS:
            b0 = (1.0f + cs) / 2.0f;
            b1 = -(1.0f + cs);
            b2 = (1.0f + cs) / 2.0f;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cs;
            a2 = 1.0f - alpha;
            break;
            
        case FilterType::BANDPASS:
            b0 = sn / 2.0f;
            b1 = 0.0f;
            b2 = -sn / 2.0f;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cs;
            a2 = 1.0f - alpha;
            break;
            
        case FilterType::NOTCH:
            b0 = 1.0f;
            b1 = -2.0f * cs;
            b2 = 1.0f;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cs;
            a2 = 1.0f - alpha;
            break;
            
        case FilterType::PEAKING:
            b0 = 1.0f + alpha * gain;
            b1 = -2.0f * cs;
            b2 = 1.0f - alpha * gain;
            a0 = 1.0f + alpha / gain;
            a1 = -2.0f * cs;
            a2 = 1.0f - alpha / gain;
            break;
            
        case FilterType::LOWSHELF:
            {
                float A = powf(10.0f, gain / 40.0f);
                b0 = A * ((A + 1.0f) - (A - 1.0f) * cs + 2.0f * sqrtf(A) * alpha);
                b1 = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * cs);
                b2 = A * ((A + 1.0f) - (A - 1.0f) * cs - 2.0f * sqrtf(A) * alpha);
                a0 = (A + 1.0f) + (A - 1.0f) * cs + 2.0f * sqrtf(A) * alpha;
                a1 = -2.0f * ((A - 1.0f) + (A + 1.0f) * cs);
                a2 = (A + 1.0f) + (A - 1.0f) * cs - 2.0f * sqrtf(A) * alpha;
            }
            break;
            
        case FilterType::HISHELF:
            {
                float A = powf(10.0f, gain / 40.0f);
                b0 = A * ((A + 1.0f) + (A - 1.0f) * cs + 2.0f * sqrtf(A) * alpha);
                b1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cs);
                b2 = A * ((A + 1.0f) + (A - 1.0f) * cs - 2.0f * sqrtf(A) * alpha);
                a0 = (A + 1.0f) - (A - 1.0f) * cs + 2.0f * sqrtf(A) * alpha;
                a1 = 2.0f * ((A - 1.0f) - (A + 1.0f) * cs);
                a2 = (A + 1.0f) - (A - 1.0f) * cs - 2.0f * sqrtf(A) * alpha;
            }
            break;
    }
    
    // Apply filter
    output = (b0 * input + b1 * x1[0] + b2 * x2[0] - a1 * y1[0] - a2 * y2[0]) / a0;
    
    // Update delay lines
    x2[0] = x1[0];
    x1[0] = input;
    y2[0] = y1[0];
    y1[0] = output;
    
    return output;
}

// Process a full sample block in-place or out-of-place.
void Filter::processBlock(float* input, float* output, size_t bufferSize) {
    for (size_t i = 0; i < bufferSize; ++i) {
        output[i] = process(input[i]);
    }
}

// Getter for current filter type.
FilterType Filter::getType() const {
    return type;
}

// Getter for cutoff frequency.
float Filter::getFrequency() const {
    return frequency;
}

// Getter for resonance/Q.
float Filter::getResonance() const {
    return resonance;
}

// Getter for filter gain parameter.
float Filter::getGain() const {
    return gain;
}

// Getter for configured sample rate.
float Filter::getSampleRate() const {
    return sampleRate;
}

// Clear delay-line state to silence/history reset.
void Filter::reset() {
    for (auto& val : x1) val = 0.0f;
    for (auto& val : x2) val = 0.0f;
    for (auto& val : y1) val = 0.0f;
    for (auto& val : y2) val = 0.0f;
}