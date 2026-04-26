#include "audio_pipeline.h"
#include <iostream>
#include <algorithm>

// Build a pipeline with default block size and optional wavetable bootstrap.
AudioPipeline::AudioPipeline(WavetableManager* wm, float sampleRate) 
    : wavetableManager(wm), sampleRate(sampleRate), bufferSize(64), wavetablesInitialized(false), activeMidiNote(-1) {
    // Initialize default wavetables on construction
    if (wavetableManager) {
        initializeDefaultWavetables();
    }
}

// Pipeline resource lifetime is managed by STL containers.
AudioPipeline::~AudioPipeline() = default;

// Add an oscillator and align its sample rate with pipeline settings.
void AudioPipeline::addOscillator(std::unique_ptr<Oscillator> osc) {
    if (osc) {
        osc->setSampleRate(sampleRate);
    }
    oscillators.push_back(std::move(osc));
    oscillatorToFilter.push_back(NO_CONNECTION);
}

// Add a filter and align its sample rate with pipeline settings.
void AudioPipeline::addFilter(std::unique_ptr<Filter> filt) {
    if (filt) {
        filt->setSampleRate(sampleRate);
    }
    filters.push_back(std::move(filt));
    filterToEnvelope.push_back(NO_CONNECTION);
}

// Add an envelope and align its sample rate with pipeline settings.
void AudioPipeline::addEnvelope(std::unique_ptr<Envelope> env) {
    if (env) {
        env->setSampleRate(sampleRate);
    }
    envelopes.push_back(std::move(env));
    envelopeToOutput.push_back(0);
}

// Connect one oscillator to a filter index, or disconnect on invalid target.
void AudioPipeline::connectOscillatorToFilter(size_t oscIndex, size_t filterIndex) {
    if (oscIndex >= oscillators.size()) {
        return;
    }
    oscillatorToFilter[oscIndex] = (filterIndex < filters.size()) ? filterIndex : NO_CONNECTION;
}

// Connect one filter to an envelope index, or disconnect on invalid target.
void AudioPipeline::connectFilterToEnvelope(size_t filterIndex, size_t envIndex) {
    if (filterIndex >= filters.size()) {
        return;
    }
    filterToEnvelope[filterIndex] = (envIndex < envelopes.size()) ? envIndex : NO_CONNECTION;
}

// Route an envelope's output to the requested channel index.
void AudioPipeline::connectEnvelopeToOutput(size_t envIndex, float* output, size_t outputIndex) {
    (void)output;
    if (envIndex >= envelopes.size()) {
        return;
    }
    envelopeToOutput[envIndex] = outputIndex;
}

// Render one audio block through oscillator -> filter -> envelope -> output routing.
void AudioPipeline::processBlock(float** inputs, float** outputs, size_t numInputs, size_t numOutputs, size_t bufferSize) {
    (void)inputs;
    (void)numInputs;
    if (!outputs || numOutputs == 0 || !outputs[0]) {
        return;
    }

    this->bufferSize = bufferSize;
    mixBuffer.assign(bufferSize, 0.0f);
    stageBuffer.assign(bufferSize, 0.0f);

    for (size_t ch = 0; ch < numOutputs; ++ch) {
        if (outputs[ch]) {
            std::fill(outputs[ch], outputs[ch] + bufferSize, 0.0f);
        }
    }

    for (size_t oscIndex = 0; oscIndex < oscillators.size(); ++oscIndex) {
        if (!oscillators[oscIndex]) {
            continue;
        }
        std::fill(stageBuffer.begin(), stageBuffer.end(), 0.0f);
        oscillators[oscIndex]->processBlock(stageBuffer.data(), bufferSize);

        size_t filterIndex = (oscIndex < oscillatorToFilter.size()) ? oscillatorToFilter[oscIndex] : NO_CONNECTION;
        if (filterIndex == NO_CONNECTION && !filters.empty()) {
            filterIndex = 0;
        }

        if (filterIndex != NO_CONNECTION && filterIndex < filters.size() && filters[filterIndex]) {
            filters[filterIndex]->processBlock(stageBuffer.data(), stageBuffer.data(), bufferSize);

            size_t envIndex = (filterIndex < filterToEnvelope.size()) ? filterToEnvelope[filterIndex] : NO_CONNECTION;
            if (envIndex == NO_CONNECTION && !envelopes.empty()) {
                envIndex = 0;
            }

            if (envIndex != NO_CONNECTION && envIndex < envelopes.size() && envelopes[envIndex]) {
                for (size_t sample = 0; sample < bufferSize; ++sample) {
                    stageBuffer[sample] *= envelopes[envIndex]->process();
                }
            }
        } else if (!envelopes.empty() && envelopes[0]) {
            for (size_t sample = 0; sample < bufferSize; ++sample) {
                stageBuffer[sample] *= envelopes[0]->process();
            }
        }

        for (size_t sample = 0; sample < bufferSize; ++sample) {
            mixBuffer[sample] += stageBuffer[sample];
        }
    }

    if (!envelopes.empty()) {
        size_t outIndex = (0 < envelopeToOutput.size()) ? envelopeToOutput[0] : 0;
        if (outIndex >= numOutputs || !outputs[outIndex]) {
            outIndex = 0;
        }
        if (outputs[outIndex]) {
            for (size_t sample = 0; sample < bufferSize; ++sample) {
                outputs[outIndex][sample] += mixBuffer[sample];
            }
        }
    } else if (outputs[0]) {
        for (size_t sample = 0; sample < bufferSize; ++sample) {
            outputs[0][sample] += mixBuffer[sample];
        }
    }
}

// Getter for wavetable manager backing the pipeline.
WavetableManager* AudioPipeline::getWavetableManager() const {
    return wavetableManager;
}

// Getter for pipeline processing sample rate.
float AudioPipeline::getSampleRate() const {
    return sampleRate;
}

// Getter for current internal block size.
size_t AudioPipeline::getBufferSize() const {
    return bufferSize;
}

// Resize internal work buffers to match the next processing block size.
void AudioPipeline::setBufferSize(size_t size) {
    bufferSize = size;
    mixBuffer.assign(bufferSize, 0.0f);
    stageBuffer.assign(bufferSize, 0.0f);
}

// Create standard default waveforms for immediate synth playback.
void AudioPipeline::initializeDefaultWavetables() {
    if (!wavetableManager) return;
    
    // Generate standard waveforms
    const size_t tblSize = 8192; // higher resolution table for smoother playback
    wavetableManager->generateSineWave("sine", tblSize);
    wavetableManager->generateSquareWave("square", tblSize);
    wavetableManager->generateSawtoothWave("sawtooth", tblSize);
    wavetableManager->generateTriangleWave("triangle", tblSize);
    
    wavetablesInitialized = true;
    std::cout << "Default wavetables initialized: sine, square, sawtooth, triangle" << std::endl;
}

// Placeholder hook for future custom wavetable generation strategies.
void AudioPipeline::generateWavetables() {
    if (!wavetableManager) return;
    
    // Generate additional wavetables as needed
    // This can be extended to generate more complex wavetables
}

// Assign default waveforms and baseline frequencies to active oscillators.
void AudioPipeline::configureOscillatorsWithWavetables() {
    if (!wavetableManager || !wavetablesInitialized) return;
    
    // Set each oscillator to use the sine waveform by default
    for (size_t i = 0; i < oscillators.size(); ++i) {
        if (oscillators[i]) {
            Wavetable* sineWavetable = wavetableManager->getWavetable("sine");
            if (sineWavetable) {
                oscillators[i]->setWavetable(sineWavetable);
                oscillators[i]->setFrequency(440.0f + (i * 100.0f)); // Slight variation in frequency
                oscillators[i]->setSampleRate(sampleRate);
                std::cout << "Configured oscillator " << i << " with sine wavetable at " 
                          << (440.0f + (i * 100.0f)) << " Hz" << std::endl;
            }
        }
    }
}

// Set one oscillator's waveform by wavetable name lookup.
void AudioPipeline::setOscillatorWavetable(size_t oscIndex, const std::string& wavetableName) {
    if (oscIndex >= oscillators.size() || !wavetableManager) return;
    
    if (oscillators[oscIndex]) {
        Wavetable* wavetable = wavetableManager->getWavetable(wavetableName);
        if (wavetable) {
            oscillators[oscIndex]->setWavetable(wavetable);
            oscillators[oscIndex]->setWavetableName(wavetableName);
            std::cout << "Set oscillator " << oscIndex << " to wavetable: " << wavetableName << std::endl;
        }
    }
}

// Set one oscillator's base frequency.
void AudioPipeline::setOscillatorFrequency(size_t oscIndex, float frequency) {
    if (oscIndex >= oscillators.size()) return;
    
    if (oscillators[oscIndex]) {
        oscillators[oscIndex]->setFrequency(frequency);
    }
}

// Set one oscillator's detune amount in cents.
void AudioPipeline::setOscillatorDetune(size_t oscIndex, float cents) {
    if (oscIndex >= oscillators.size()) return;
    if (oscillators[oscIndex]) {
        oscillators[oscIndex]->setDetune(cents);
    }
}

// Set filter cutoff for the selected filter.
void AudioPipeline::setFilterCutoff(size_t filterIndex, float cutoff) {
    if (filterIndex >= filters.size()) return;
    if (filters[filterIndex]) {
        filters[filterIndex]->setFrequency(cutoff);
    }
}

// Set filter resonance for the selected filter.
void AudioPipeline::setFilterResonance(size_t filterIndex, float resonance) {
    if (filterIndex >= filters.size()) return;
    if (filters[filterIndex]) {
        filters[filterIndex]->setResonance(resonance);
    }
}

// Set filter topology for the selected filter.
void AudioPipeline::setFilterType(size_t filterIndex, FilterType type) {
    if (filterIndex >= filters.size()) return;
    if (filters[filterIndex]) {
        filters[filterIndex]->setType(type);
    }
}

// Set envelope attack for the selected envelope.
void AudioPipeline::setEnvelopeAttack(size_t envIndex, float attack) {
    if (envIndex >= envelopes.size()) return;
    if (envelopes[envIndex]) {
        envelopes[envIndex]->setAttackTime(attack);
    }
}

// Set envelope decay for the selected envelope.
void AudioPipeline::setEnvelopeDecay(size_t envIndex, float decay) {
    if (envIndex >= envelopes.size()) return;
    if (envelopes[envIndex]) {
        envelopes[envIndex]->setDecayTime(decay);
    }
}

// Set envelope sustain for the selected envelope.
void AudioPipeline::setEnvelopeSustain(size_t envIndex, float sustain) {
    if (envIndex >= envelopes.size()) return;
    if (envelopes[envIndex]) {
        envelopes[envIndex]->setSustainLevel(sustain);
    }
}

// Set envelope release for the selected envelope.
void AudioPipeline::setEnvelopeRelease(size_t envIndex, float release) {
    if (envIndex >= envelopes.size()) return;
    if (envelopes[envIndex]) {
        envelopes[envIndex]->setReleaseTime(release);
    }
}

// Getter for wavetable initialization state.
bool AudioPipeline::areWavetablesInitialized() const {
    return wavetablesInitialized;
}

// Trigger monophonic note-on and store active note identity.
void AudioPipeline::noteOn(float frequency, int noteNumber) {
    if (!oscillators.empty()) {
        oscillators[0]->setFrequency(frequency);
    }
    if (!envelopes.empty()) {
        envelopes[0]->triggerAttack();
    }
    activeMidiNote = noteNumber;
}

// Trigger monophonic note-off, but ignore mismatched note numbers.
void AudioPipeline::noteOff(int noteNumber) {
    if (noteNumber >= 0 && activeMidiNote >= 0 && noteNumber != activeMidiNote) {
        return;
    }
    if (!envelopes.empty()) {
        envelopes[0]->triggerRelease();
    }
    activeMidiNote = -1;
}