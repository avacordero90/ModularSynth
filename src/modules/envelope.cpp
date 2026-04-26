#include "envelope.h"
#include <cmath>

// Initialize ADSR defaults and idle state.
Envelope::Envelope() 
    : attackTime(0.01f), decayTime(0.1f), sustainLevel(0.7f), releaseTime(0.3f),
      value(0.0f), targetValue(0.0f), stepSize(0.0f), currentSegment(EnvelopeSegment::RELEASE),
      isRunning(false), isSustained(false), sampleRate(44100.0f) {}

// Envelope owns no external resources.
Envelope::~Envelope() = default;

// Clamp attack to a small positive value to avoid divide-by-zero.
void Envelope::setAttackTime(float time) {
    attackTime = std::max(0.001f, time);
}

// Clamp decay to a small positive value to avoid divide-by-zero.
void Envelope::setDecayTime(float time) {
    decayTime = std::max(0.001f, time);
}

// Sustain is normalized to the [0, 1] gain range.
void Envelope::setSustainLevel(float level) {
    sustainLevel = std::max(0.0f, std::min(level, 1.0f));
}

// Clamp release to a small positive value to avoid divide-by-zero.
void Envelope::setReleaseTime(float time) {
    releaseTime = std::max(0.001f, time);
}

// Update runtime sample rate used by ADSR step calculations.
void Envelope::setSampleRate(float rate) {
    sampleRate = rate;
}

// Start a new note by entering ATTACK from zero.
void Envelope::triggerAttack() {
    currentSegment = EnvelopeSegment::ATTACK;
    targetValue = 1.0f;
    stepSize = 1.0f / (attackTime * sampleRate);
    value = 0.0f;
    isRunning = true;
    isSustained = false;
}

// Move the running envelope into RELEASE from its current level.
void Envelope::triggerRelease() {
    if (!isRunning) return;
    
    currentSegment = EnvelopeSegment::RELEASE;
    targetValue = 0.0f;
    stepSize = 1.0f / (releaseTime * sampleRate);
    isRunning = true;
    isSustained = false;
}

// Hard reset to a silent, non-running state.
void Envelope::reset() {
    value = 0.0f;
    targetValue = 0.0f;
    stepSize = 0.0f;
    currentSegment = EnvelopeSegment::RELEASE;
    isRunning = false;
    isSustained = false;
}

// Advance one sample of the ADSR state machine and return current gain.
float Envelope::process() {
    if (!isRunning) return 0.0f;
    
    // Update value based on current segment
    switch (currentSegment) {
        case EnvelopeSegment::ATTACK:
            value += stepSize;
            if (value >= targetValue) {
                value = targetValue;
                currentSegment = EnvelopeSegment::DECAY;
                targetValue = sustainLevel;
                stepSize = (sustainLevel - 1.0f) / (decayTime * sampleRate);
            }
            break;
            
        case EnvelopeSegment::DECAY:
            value += stepSize;
            if (value <= targetValue) {
                value = targetValue;
                isSustained = true;
                currentSegment = EnvelopeSegment::SUSTAIN;
            }
            break;

        case EnvelopeSegment::SUSTAIN:
            value = sustainLevel;
            break;
            
        case EnvelopeSegment::RELEASE:
            value -= stepSize;
            if (value <= targetValue) {
                value = targetValue;
                isRunning = false;
                isSustained = false;
            }
            break;
    }
    
    return value;
}

// Read current envelope gain value.
float Envelope::getValue() const {
    return value;
}

// Envelope is finished once release reaches (or passes) zero.
bool Envelope::isFinished() const {
    return !isRunning && value <= 0.0f;
}

// Getter for attack time in seconds.
float Envelope::getAttackTime() const {
    return attackTime;
}

// Getter for decay time in seconds.
float Envelope::getDecayTime() const {
    return decayTime;
}

// Getter for sustain level.
float Envelope::getSustainLevel() const {
    return sustainLevel;
}

// Getter for release time in seconds.
float Envelope::getReleaseTime() const {
    return releaseTime;
}

// Getter for current ADSR segment.
EnvelopeSegment Envelope::getCurrentSegment() const {
    return currentSegment;
}