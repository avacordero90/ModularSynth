#ifndef FILTER_H
#define FILTER_H

#include <vector>

enum class FilterType {
    LOWPASS,
    HIGHPASS,
    BANDPASS,
    NOTCH,
    PEAKING,
    LOWSHELF,
    HISHELF
};

class Filter {
private:
    FilterType type;
    float frequency;
    float resonance;
    float gain; // For shelving filters
    float sampleRate;
    
    // State variables for filter
    std::vector<float> x1, x2, y1, y2;
    
public:
    Filter(FilterType type = FilterType::LOWPASS);
    ~Filter();
    
    // Setup
    void setType(FilterType newType);
    void setFrequency(float freq);
    void setResonance(float res);
    void setGain(float g);
    void setSampleRate(float rate);
    
    // Audio processing
    float process(float input);
    void processBlock(float* input, float* output, size_t bufferSize);
    
    // Getters
    FilterType getType() const;
    float getFrequency() const;
    float getResonance() const;
    float getGain() const;
    float getSampleRate() const;
    
    // Reset
    void reset();
};

#endif // FILTER_H