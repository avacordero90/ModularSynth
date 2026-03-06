#ifndef ENVELOPE_H
#define ENVELOPE_H

#include <vector>

enum class EnvelopeSegment {
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE
};

class Envelope {
private:
    // ADSR parameters
    float attackTime;
    float decayTime;
    float sustainLevel;
    float releaseTime;
    
    // State
    float value;
    float targetValue;
    float stepSize;
    EnvelopeSegment currentSegment;
    bool isRunning;
    bool isSustained;
    
    float sampleRate;
    
public:
    Envelope();
    ~Envelope();
    
    // Setup
    void setAttackTime(float time);
    void setDecayTime(float time);
    void setSustainLevel(float level);
    void setReleaseTime(float time);
    void setSampleRate(float rate);
    
    // Control
    void triggerAttack();
    void triggerRelease();
    void reset();
    
    // Audio processing
    float process();
    float getValue() const;
    bool isFinished() const;
    
    // Getters
    float getAttackTime() const;
    float getDecayTime() const;
    float getSustainLevel() const;
    float getReleaseTime() const;
    EnvelopeSegment getCurrentSegment() const;
};

#endif // ENVELOPE_H