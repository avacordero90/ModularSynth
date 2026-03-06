#ifndef PLUGIN_INTERFACE_H
#define PLUGIN_INTERFACE_H

#include "../core/audio_pipeline.h"
#include "../core/wavetable_manager.h"
#include <string>
#include <vector>

// Control types for UI elements
enum class ControlType {
    SLIDER,
    KNOB,
    SWITCH,
    BUTTON,
    COMBOBOX,
    TEXT_INPUT
};

// Plugin control structure for each module parameter
struct Parameter {
    std::string name;
    float minValue;
    float maxValue;
    float defaultValue;
    float value;
    bool isModulatable;
    ControlType controlType;
    std::string unit; // For display purposes (e.g., "Hz", "%", "dB")
    
    Parameter(const std::string& n, float min, float max, float def, bool mod = false, ControlType ctrlType = ControlType::SLIDER, const std::string& unitStr = "")
        : name(n), minValue(min), maxValue(max), defaultValue(def), value(def), isModulatable(mod), controlType(ctrlType), unit(unitStr) {}
};

// Plugin parameter categories
enum class ParameterCategory {
    OSCILLATOR,
    FILTER,
    ENVELOPE,
    WAVETABLE
};

// UI Layout structure 
struct ControlLayout {
    int x, y; // Position
    int width, height; // Size
    ParameterCategory category; // Which module this parameter belongs to
    
    ControlLayout(int x_pos, int y_pos, int w, int h, ParameterCategory cat)
        : x(x_pos), y(y_pos), width(w), height(h), category(cat) {}
};

// Base plugin interface class
class PluginInterface {
protected:
    AudioPipeline* pipeline;
    WavetableManager* wavetableManager;
    
    // Parameters for all modules
    std::vector<Parameter> oscillatorParams;
    std::vector<Parameter> filterParams;
    std::vector<Parameter> envelopeParams;
    std::vector<Parameter> wavetableParams;
    
    // UI layouts for parameters
    std::vector<ControlLayout> controlLayouts;
    
    // Internal state
    int currentWavetableIndex;
    
public:
    PluginInterface();
    virtual ~PluginInterface();
    
    // Setup
    void setPipeline(AudioPipeline* pipe);
    void setupParameters();
    
    // Control methods
    virtual void setParameterValue(int paramIndex, float value) = 0;
    virtual float getParameterValue(int paramIndex) const = 0;
    
    // Plugin lifecycle
    virtual void initialize() = 0;
    virtual void processAudio(float** inputs, float** outputs, size_t numInputs, size_t numOutputs, size_t bufferSize) = 0;
    virtual void cleanup() = 0;
    
    // Getters for parameters
    const std::vector<Parameter>& getOscillatorParams() const;
    const std::vector<Parameter>& getFilterParams() const;
    const std::vector<Parameter>& getEnvelopeParams() const;
    const std::vector<Parameter>& getWavetableParams() const;
    
    // Control layout
    const std::vector<ControlLayout>& getControlLayouts() const;
    
    // Wavetable management methods (these would be called from UI)
    void loadWavetables();
    void saveWavetables();
    void addNewWavetable(const std::string& name, const std::vector<float>& data);
    void removeWavetable(const std::string& name);
    void generateWavetable(const std::string& name, int wavetableType, size_t length);
    
    // Parameter mapping methods for audio modules
    void mapParametersToModules();
    
    // Utility methods for UI
    std::string getCategoryName(ParameterCategory category) const;
};

#endif // PLUGIN_INTERFACE_H