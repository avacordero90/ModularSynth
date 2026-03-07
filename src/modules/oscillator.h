#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include "../core/wavetable.h"
#include <string>

class Oscillator {
private:
    Wavetable* wavetable;
    float phase;
    float frequency;
    float detune;
    std::string wavetableName;
    
public:
    Oscillator();
    ~Oscillator();
    
    // Setup
    void setWavetable(Wavetable* wt);
    void setWavetableName(const std::string& name);
    void setFrequency(float freq);
    void setDetune(float cents);
    
    // Audio processing
    float process();
    void processBlock(float* output, size_t bufferSize);
    
    // Modulation
    void setPhase(float p);
    void incrementPhase(float inc);
    
    // Wavetable modulation (new)
    void modulateWavetablePhase(float phaseOffset);
    void modulateWavetableAmplitude(float amplitudeFactor);
    void applyWavetableEnvelopes(const std::vector<float>& envelope);
    void addWavetableNoise(float intensity);
    
    // Getters
    float getFrequency() const;
    float getPhase() const;
    Wavetable* getWavetable() const;
    
    // Reset
    void reset();
};

#endif // OSCILLATOR_H