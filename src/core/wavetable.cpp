#include "wavetable.h"
#include <algorithm>
#include <cmath>
#include <random>

// Construct a wavetable from precomputed sample data.
Wavetable::Wavetable(const std::string& name, const std::vector<float>& data) 
    : name(name), data(data), length(data.size()) {}

// Defaulted destructor for value-type container.
Wavetable::~Wavetable() = default;

// Return wavetable identifier.
const std::string& Wavetable::getName() const {
    return name;
}

// Return number of samples in the table.
size_t Wavetable::getLength() const {
    return length;
}

// Return sample at index, or 0 for out-of-range access.
float Wavetable::getValue(size_t index) const {
    if (index >= length) {
        return 0.0f;
    }
    return data[index];
}

// Read with linear interpolation and wrapped phase indexing.
float Wavetable::getInterpolatedValue(float index) const {
    if (length == 0) return 0.0f;
    
    // Handle wraparound
    index = fmod(index, static_cast<float>(length));
    if (index < 0) index += length;
    
    size_t idx1 = static_cast<size_t>(floorf(index));
    size_t idx2 = (idx1 + 1) % length;
    
    float frac = index - static_cast<float>(idx1);
    
    return data[idx1] * (1.0f - frac) + data[idx2] * frac;
}

// Resize table with simple nearest-neighbor style resampling.
void Wavetable::resize(size_t newLength) {
    if (newLength == 0) return;
    
    std::vector<float> newData(newLength, 0.0f);
    
    // Simple resampling by copying values
    for (size_t i = 0; i < newLength; ++i) {
        size_t oldIdx = (i * length) / newLength;
        if (oldIdx >= length) oldIdx = length - 1;
        newData[i] = data[oldIdx];
    }
    
    data = newData;
    length = newLength;
}

// Placeholder for dynamic wavetable update hooks.
void Wavetable::update() {
    // Virtual method for updating wavetables when needed
    // Override this in subclasses to update wavetable content dynamically
}

// Set one sample if index is valid.
void Wavetable::setSample(size_t index, float value) {
    if (index < length) {
        data[index] = value;
    }
}

// Zero all wavetable samples.
void Wavetable::clear() {
    std::fill(data.begin(), data.end(), 0.0f);
}

// Fill all wavetable samples with constant value.
void Wavetable::fill(float value) {
    std::fill(data.begin(), data.end(), value);
}

// Add white-noise-like perturbation to each sample.
void Wavetable::addNoise(float intensity) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-intensity, intensity);
    
    for (size_t i = 0; i < length; ++i) {
        data[i] += dis(gen);
    }
}

// Multiply wavetable by per-sample envelope of equal length.
void Wavetable::applyEnvelope(const std::vector<float>& envelope) {
    if (envelope.size() != length) return;
    
    for (size_t i = 0; i < length; ++i) {
        data[i] *= envelope[i];
    }
}

// Circularly shift wavetable content by normalized phase offset.
void Wavetable::modulatePhase(float phaseOffset) {
    // Simple phase modulation - shift all samples by a phase offset
    std::vector<float> newData(length);
    
    for (size_t i = 0; i < length; ++i) {
        size_t shiftedIndex = (i + static_cast<size_t>(phaseOffset * length)) % length;
        newData[i] = data[shiftedIndex];
    }
    
    data = newData;
}

// Apply gain scaling to the full table.
void Wavetable::modulateAmplitude(float amplitudeFactor) {
    for (size_t i = 0; i < length; ++i) {
        data[i] *= amplitudeFactor;
    }
}