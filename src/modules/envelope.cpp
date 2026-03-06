#include "envelope.h"
#include <cmath>

Envelope::Envelope() 
    : attackTime(0.01f), decayTime(0.1f), sustainLevel(0.7f), releaseTime(0.3f),
      value(0.0f), targetValue(0.0f), stepSize(0.0f), currentSegment(EnvelopeSegment::RELEASE),
      isRunning(false), isSustained(false), sampleRate(44100.0f) {}

Envelope::~Envelope() = default;

void Envelope::setAttackTime(float time) {
    attackTime = std::max(0.001f, time);
}

void Envelope::setDecayTime(float time) {
    decayTime = std::max(0.001f, time);
}

void Envelope::setSustainLevel(float level) {
    sustainLevel = std::max(0.0f, std::min(level, 1.0f));
}

void Envelope::setReleaseTime(float time) {
    releaseTime = std::max(0.001f, time);
}

void Envelope::setSampleRate(float rate) {
    sampleRate = rate;
}

void Envelope::triggerAttack() {
    currentSegment = EnvelopeSegment::ATTACK;
    targetValue = 1.0f;
    stepSize = 1.0f / (attackTime * sampleRate);
    value = 0.0f;
    isRunning = true;
    isSustained = false;
}

void Envelope::triggerRelease() {
    if (!isRunning) return;
    
    currentSegment = EnvelopeSegment::RELEASE;
    targetValue = 0.0f;
    stepSize = 1.0f / (releaseTime * sampleRate);
    isRunning = true;
    isSustained = false;
}

void Envelope::reset() {
    value = 0.0f;
    targetValue = 0.0f;
    stepSize = 0.0f;
    currentSegment = EnvelopeSegment::RELEASE;
    isRunning = false;
    isSustained = false;
}

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
            }
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

float Envelope::getValue() const {
    return value;
}

bool Envelope::isFinished() const {
    return !isRunning && value <= 0.0f;
}

float Envelope::getAttackTime() const {
    return attackTime;
}

float Envelope::getDecayTime() const {
    return decayTime;
}

float Envelope::getSustainLevel() const {
    return sustainLevel;
}

float Envelope::getReleaseTime() const {
    return releaseTime;
}

EnvelopeSegment Envelope::getCurrentSegment() const {
    return currentSegment;
}