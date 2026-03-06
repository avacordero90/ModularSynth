#include "core/wavetable_manager.h"
#include "core/audio_pipeline.h"
#include "plugin/vst_plugin.h"
#include "modules/oscillator.h"
#include "modules/filter.h"
#include "modules/envelope.h"
#include <iostream>

int main() {
    std::cout << "Creating modular wavetable synthesizer..." << std::endl;
    
    // Create wavetable manager
    WavetableManager* wavetableManager = new WavetableManager();
    
    // Generate some default wavetables
    wavetableManager->generateSineWave("sine", 1024);
    wavetableManager->generateSquareWave("square", 1024);
    wavetableManager->generateSawtoothWave("saw", 1024);
    wavetableManager->generateTriangleWave("triangle", 1024);
    
    // Create audio pipeline
    AudioPipeline* pipeline = new AudioPipeline(wavetableManager, 44100.0f);
    
    // Create modules
    std::unique_ptr<Oscillator> oscillator = std::make_unique<Oscillator>();
    std::unique_ptr<Filter> filter = std::make_unique<Filter>(FilterType::LOWPASS);
    std::unique_ptr<Envelope> envelope = std::make_unique<Envelope>();
    
    // Configure oscillator with a wavetable
    Wavetable* sineWave = wavetableManager->getWavetable("sine");
    if (sineWave) {
        oscillator->setWavetable(sineWave);
        oscillator->setFrequency(440.0f);
    }
    
    // Add modules to pipeline
    pipeline->addOscillator(std::move(oscillator));
    pipeline->addFilter(std::move(filter));
    pipeline->addEnvelope(std::move(envelope));
    
    // Create plugin interface
    VSTPlugin* plugin = new VSTPlugin();
    plugin->setPipeline(pipeline);
    plugin->initialize();
    
    // Use the plugin for audio processing
    float input[64] = {0};
    float output[64] = {0};
    float* inputs[] = {input};
    float* outputs[] = {output};
    
    // Process some audio block
    plugin->processAudio(inputs, outputs, 1, 1, 64);
    
    std::cout << "Modular wavetable synth initialized and tested successfully!" << std::endl;
    
    // Cleanup
    delete plugin;
    delete pipeline;
    delete wavetableManager;
    
    return 0;
}