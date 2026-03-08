#ifndef AUDIO_PIPELINE_H
#define AUDIO_PIPELINE_H

#include "../modules/oscillator.h"
#include "../modules/filter.h"
#include "../modules/envelope.h"
#include "../core/wavetable_manager.h"
#include <vector>
#include <memory>

class AudioPipeline {
private:
    // Audio modules
    std::vector<std::unique_ptr<Oscillator>> oscillators;
    std::vector<std::unique_ptr<Filter>> filters;
    std::vector<std::unique_ptr<Envelope>> envelopes;
    
    WavetableManager* wavetableManager;
    
    // Pipeline state
    float sampleRate;
    size_t bufferSize;
    bool wavetablesInitialized;
    
public:
    AudioPipeline(WavetableManager* wm, float sampleRate = 44100.0f);
    ~AudioPipeline();
    
    // Module management
    void addOscillator(std::unique_ptr<Oscillator> osc);
    void addFilter(std::unique_ptr<Filter> filt);
    void addEnvelope(std::unique_ptr<Envelope> env);
    
    // Routing system
    void connectOscillatorToFilter(size_t oscIndex, size_t filterIndex);
    void connectFilterToEnvelope(size_t filterIndex, size_t envIndex);
    void connectEnvelopeToOutput(size_t envIndex, float* output, size_t outputIndex = 0);
    
    // Audio processing
    void processBlock(float** inputs, float** outputs, size_t numInputs, size_t numOutputs, size_t bufferSize);
    
    // Getters
    WavetableManager* getWavetableManager() const;
    float getSampleRate() const;
    size_t getBufferSize() const; // size of the internal processing block
    
    // Configuration
    void setBufferSize(size_t size);
    
    // Wavetable generation and initialization
    void initializeDefaultWavetables();
    void generateWavetables();
    void configureOscillatorsWithWavetables();
    void setOscillatorWavetable(size_t oscIndex, const std::string& wavetableName);
    void setOscillatorFrequency(size_t oscIndex, float frequency);
    
    // Audio generation helpers
    bool areWavetablesInitialized() const;
};

#endif // AUDIO_PIPELINE_H