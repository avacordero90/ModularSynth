#ifndef VST_PLUGIN_H
#define VST_PLUGIN_H

#include "plugin_interface.h"
#include "../modules/oscillator.h"
#include "../modules/filter.h"
#include "../modules/envelope.h"
#include <memory>

// Forward declarations for VST-specific classes (these would be actual VST SDK classes)
class AudioEffect { /* EMPTY */ };
class AudioProcessor { /* EMPTY */ };

class VSTPlugin : public PluginInterface {
private:
    std::unique_ptr<AudioEffect> vstEffect;
    std::unique_ptr<AudioProcessor> processor;
    
    // Plugin state
    bool isInitialized;
    
public:
    VSTPlugin();
    virtual ~VSTPlugin();
    
    // Plugin interface implementation
    void setParameterValue(int paramIndex, float value) override;
    float getParameterValue(int paramIndex) const override;
    
    // Plugin lifecycle
    void initialize() override;
    void processAudio(float** inputs, float** outputs, size_t numInputs, size_t numOutputs, size_t bufferSize) override;
    void cleanup() override;
    
    // VST-specific functions
    void setVstEffect(AudioEffect* effect);
    void setProcessor(AudioProcessor* proc);
    
    // Plugin information
    const char* getName() const;
    const char* getVendor() const;
    int getVersion() const;
    
    // Audio configuration helpers
    void configureAudioIO(size_t numInputs, size_t numOutputs, size_t sampleRate);
};

#endif // VST_PLUGIN_H