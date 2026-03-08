#include "plugin_interface.h"
#include "../modules/oscillator.h"
#include "../modules/filter.h"
#include "../modules/envelope.h"

PluginInterface::PluginInterface() : pipeline(nullptr), wavetableManager(nullptr), currentWavetableIndex(0) {
    setupParameters();
}

PluginInterface::~PluginInterface() = default;

void PluginInterface::setPipeline(AudioPipeline* pipe) {
    pipeline = pipe;
    if (pipe) {
        wavetableManager = pipe->getWavetableManager();
    }
}

void PluginInterface::setupParameters() {
    // Oscillator parameters
    oscillatorParams.push_back(Parameter("Oscillator Frequency", 20.0f, 20000.0f, 440.0f, true, ControlType::SLIDER, "Hz"));
    oscillatorParams.push_back(Parameter("Oscillator Detune", -1200.0f, 1200.0f, 0.0f, true, ControlType::SLIDER, "Cents"));
    oscillatorParams.push_back(Parameter("Oscillator Waveform", 0.0f, 5.0f, 0.0f, false, ControlType::COMBOBOX)); // Different wavetable types
    
    // Filter parameters
    filterParams.push_back(Parameter("Filter Frequency", 20.0f, 20000.0f, 1000.0f, true, ControlType::SLIDER, "Hz"));
    filterParams.push_back(Parameter("Filter Resonance", 0.1f, 10.0f, 0.707f, true, ControlType::SLIDER, "Q"));
    filterParams.push_back(Parameter("Filter Type", 0.0f, 7.0f, 0.0f, false, ControlType::COMBOBOX)); // Enum for filter types
    
    // Envelope parameters
    envelopeParams.push_back(Parameter("Attack Time", 0.001f, 5.0f, 0.01f, true, ControlType::SLIDER, "s"));
    envelopeParams.push_back(Parameter("Decay Time", 0.001f, 5.0f, 0.1f, true, ControlType::SLIDER, "s"));
    envelopeParams.push_back(Parameter("Sustain Level", 0.0f, 1.0f, 0.7f, true, ControlType::SLIDER, "%"));
    envelopeParams.push_back(Parameter("Release Time", 0.001f, 5.0f, 0.3f, true, ControlType::SLIDER, "s"));
    
    // Wavetable parameters
    wavetableParams.push_back(Parameter("Wavetable Selection", 0.0f, 100.0f, 0.0f, false, ControlType::COMBOBOX));
    wavetableParams.push_back(Parameter("Wavetable Morph", 0.0f, 1.0f, 0.5f, true, ControlType::SLIDER));
    
    // Create control layouts for UI
    controlLayouts.push_back(ControlLayout(10, 10, 150, 20, ParameterCategory::OSCILLATOR));      // Oscillator Frequency
    controlLayouts.push_back(ControlLayout(10, 40, 150, 20, ParameterCategory::OSCILLATOR));      // Oscillator Detune
    controlLayouts.push_back(ControlLayout(10, 70, 150, 20, ParameterCategory::OSCILLATOR));      // Oscillator Waveform
    
    controlLayouts.push_back(ControlLayout(10, 120, 150, 20, ParameterCategory::FILTER));         // Filter Frequency
    controlLayouts.push_back(ControlLayout(10, 150, 150, 20, ParameterCategory::FILTER));         // Filter Resonance
    controlLayouts.push_back(ControlLayout(10, 180, 150, 20, ParameterCategory::FILTER));         // Filter Type
    
    controlLayouts.push_back(ControlLayout(10, 230, 150, 20, ParameterCategory::ENVELOPE));       // Attack Time
    controlLayouts.push_back(ControlLayout(10, 260, 150, 20, ParameterCategory::ENVELOPE));       // Decay Time
    controlLayouts.push_back(ControlLayout(10, 290, 150, 20, ParameterCategory::ENVELOPE));       // Sustain Level
    controlLayouts.push_back(ControlLayout(10, 320, 150, 20, ParameterCategory::ENVELOPE));       // Release Time
    
    controlLayouts.push_back(ControlLayout(10, 370, 150, 20, ParameterCategory::WAVETABLE));      // Wavetable Selection
    controlLayouts.push_back(ControlLayout(10, 400, 150, 20, ParameterCategory::WAVETABLE));      // Wavetable Morph
}

const std::vector<Parameter>& PluginInterface::getOscillatorParams() const {
    return oscillatorParams;
}

const std::vector<Parameter>& PluginInterface::getFilterParams() const {
    return filterParams;
}

const std::vector<Parameter>& PluginInterface::getEnvelopeParams() const {
    return envelopeParams;
}

const std::vector<Parameter>& PluginInterface::getWavetableParams() const {
    return wavetableParams;
}

const std::vector<ControlLayout>& PluginInterface::getControlLayouts() const {
    return controlLayouts;
}

void PluginInterface::loadWavetables() {
    // Load wavetables into the manager
    if (wavetableManager) {
        // Generate some default wavetables
        wavetableManager->generateSineWave("sine", 1024);
        wavetableManager->generateSquareWave("square", 1024);
        wavetableManager->generateSawtoothWave("saw", 1024);
        wavetableManager->generateTriangleWave("triangle", 1024);
        
        // Add more complex wavetables
        wavetableManager->addWavetable("sine_5th", {0.0f, 0.5f, 0.0f, -0.5f, 0.0f});
        wavetableManager->addWavetable("square_5th", {1.0f, 1.0f, -1.0f, -1.0f, 1.0f});
    }
}

void PluginInterface::saveWavetables() {
    // Save any modified wavetables
}

void PluginInterface::addNewWavetable(const std::string& name, const std::vector<float>& data) {
    if (wavetableManager) {
        wavetableManager->addWavetable(name, data);
    }
}

void PluginInterface::removeWavetable(const std::string& name) {
    if (wavetableManager) {
        wavetableManager->removeWavetable(name);
    }
}

void PluginInterface::generateWavetable(const std::string& name, int wavetableType, size_t length) {
    if (wavetableManager) {
        switch(wavetableType) {
            case 0:
                wavetableManager->generateSineWave(name, length);
                break;
            case 1:
                wavetableManager->generateSquareWave(name, length);
                break;
            case 2:
                wavetableManager->generateSawtoothWave(name, length);
                break;
            case 3:
                wavetableManager->generateTriangleWave(name, length);
                break;
        }
    }
}

void PluginInterface::mapParametersToModules() {
    // This would be called when parameters are changed to update the audio modules
    // Implementation will depend on how audio pipelines work
    
    // For now, this is a placeholder
    // In practice, we'd iterate through parameters and set module values accordingly
}

std::string PluginInterface::getCategoryName(ParameterCategory category) const {
    switch(category) {
        case ParameterCategory::OSCILLATOR: return "Oscillator";
        case ParameterCategory::FILTER: return "Filter";
        case ParameterCategory::ENVELOPE: return "Envelope";
        case ParameterCategory::WAVETABLE: return "Wavetable";
        default: return "Unknown";
    }
}