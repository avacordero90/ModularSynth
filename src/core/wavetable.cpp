#include "wavetable.h"
#include <algorithm>
#include <cmath>

Wavetable::Wavetable(const std::string& name, const std::vector<float>& data) 
    : name(name), data(data), length(data.size()) {}

Wavetable::~Wavetable() = default;

const std::string& Wavetable::getName() const {
    return name;
}

size_t Wavetable::getLength() const {
    return length;
}

float Wavetable::getValue(size_t index) const {
    if (index >= length) {
        return 0.0f;
    }
    return data[index];
}

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