#ifndef MIDI_HANDLER_H
#define MIDI_HANDLER_H

#include <vector>
#include <functional>
#include <cstdint>

// MIDI message types
enum class MidiMessageType {
    NOTE_OFF = 0x80,
    NOTE_ON = 0x90,
    CONTROL_CHANGE = 0xB0,
    PROGRAM_CHANGE = 0xC0,
    PITCH_BEND = 0xE0,
    CLOCK = 0xF8,
    START = 0xFA,
    CONTINUE = 0xFB,
    STOP = 0xFC
};

// MIDI note structure
struct MidiNote {
    int noteNumber;
    float velocity;
    bool isOn;
    int channel;
    
    MidiNote(int note, float vel, bool on, int chan) 
        : noteNumber(note), velocity(vel), isOn(on), channel(chan) {}
};

// MIDI controller structure
struct MidiController {
    int controllerNumber;
    float value;
    int channel;
    
    MidiController(int ctrl, float val, int chan)
        : controllerNumber(ctrl), value(val), channel(chan) {}
};

// MIDI clock structure
struct MidiClock {
    int tickCount;
    float tempo; // in BPM
    bool isRunning;
    
    MidiClock() : tickCount(0), tempo(120.0f), isRunning(false) {}
};

class MidiHandler {
private:
    // Callbacks for different MIDI events
    std::function<void(MidiNote)> noteOnCallback;
    std::function<void(MidiNote)> noteOffCallback;
    std::function<void(MidiController)> controllerChangeCallback;
    std::function<void(int, float)> pitchBendCallback;
    std::function<void()> clockCallback;
    std::function<void()> startCallback;
    std::function<void()> continueCallback;
    std::function<void()> stopCallback;
    
    // MIDI state tracking
    std::vector<bool> activeNotes; // Track which notes are currently playing
    
    // Clock state
    MidiClock midiClock;
    
public:
    MidiHandler();
    ~MidiHandler();
    
    // MIDI message processing
    void processMidiMessage(uint8_t status, uint8_t data1, uint8_t data2);
    void processMidiMessage(const std::vector<uint8_t>& message);
    
    // Set callbacks
    void setNoteOnCallback(std::function<void(MidiNote)> callback);
    void setNoteOffCallback(std::function<void(MidiNote)> callback);
    void setControllerChangeCallback(std::function<void(MidiController)> callback);
    void setPitchBendCallback(std::function<void(int, float)> callback);
    void setClockCallback(std::function<void()> callback);
    void setStartCallback(std::function<void()> callback);
    void setContinueCallback(std::function<void()> callback);
    void setStopCallback(std::function<void()> callback);
    
    // MIDI clock management
    void updateClock();
    void startClock();
    void stopClock();
    void setFrameRate(float bpm);
    
    // Utility methods
    int getActiveNoteCount() const;
    bool isNoteActive(int noteNumber) const;
    
    // Getters
    const MidiClock& getMidiClock() const;
};

#endif // MIDI_HANDLER_H