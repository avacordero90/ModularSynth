#include "midi_handler.h"
#include <iostream>

// Initialize MIDI note state and transport clock defaults.
MidiHandler::MidiHandler() : activeNotes(128, false) {
    // Initialize MIDI clock
    midiClock.tempo = 120.0f;
    midiClock.isRunning = false;
    midiClock.tickCount = 0;
}

// Defaulted destructor; callbacks and containers self-manage.
MidiHandler::~MidiHandler() = default;

// Decode a short MIDI message and dispatch to registered callbacks.
void MidiHandler::processMidiMessage(uint8_t status, uint8_t data1, uint8_t data2) {
    // Extract channel and message type
    int messageType = status & 0xF0;
    int channel = status & 0x0F;
    
    switch (messageType) {
        case static_cast<int>(MidiMessageType::NOTE_ON):
            if (data2 > 0) {
                // Note on event
                MidiNote note(data1, data2 / 127.0f, true, channel);
                if (noteOnCallback) {
                    noteOnCallback(note);
                }
                activeNotes[data1] = true;
            } else {
                // Note off (velocity 0)
                MidiNote note(data1, 0.0f, false, channel);
                if (noteOffCallback) {
                    noteOffCallback(note);
                }
                activeNotes[data1] = false;
            }
            break;
            
        case static_cast<int>(MidiMessageType::NOTE_OFF):
            {
                // Note off event
                MidiNote note(data1, data2 / 127.0f, false, channel);
                if (noteOffCallback) {
                    noteOffCallback(note);
                }
                activeNotes[data1] = false;
            }
            break;
            
        case static_cast<int>(MidiMessageType::CONTROL_CHANGE):
            {
                // Controller change event
                MidiController controller(data1, data2 / 127.0f, channel);
                if (controllerChangeCallback) {
                    controllerChangeCallback(controller);
                }
            }
            break;
            
        case static_cast<int>(MidiMessageType::PITCH_BEND):
            {
                // Pitch bend event (14-bit value)
                int pitchValue = (data2 << 7) | data1;
                float bendValue = (pitchValue - 8192.0f) / 8192.0f;  // Normalize to -1 to +1
                if (pitchBendCallback) {
                    pitchBendCallback(channel, bendValue);
                }
            }
            break;
            
        case static_cast<int>(MidiMessageType::CLOCK):
            {
                // MIDI clock event
                if (clockCallback) {
                    clockCallback();
                }
                midiClock.tickCount++;
            }
            break;
            
        case static_cast<int>(MidiMessageType::START):
            {
                // Start sequence
                if (startCallback) {
                    startCallback();
                }
                startClock();
            }
            break;
            
        case static_cast<int>(MidiMessageType::CONTINUE):
            {
                // Continue sequence
                if (continueCallback) {
                    continueCallback();
                }
                startClock();
            }
            break;
            
        case static_cast<int>(MidiMessageType::STOP):
            {
                // Stop sequence
                if (stopCallback) {
                    stopCallback();
                }
                stopClock();
            }
            break;
    }
}

// Decode vector-form MIDI bytes into status/data tuple.
void MidiHandler::processMidiMessage(const std::vector<uint8_t>& message) {
    if (message.empty()) return;
    
    uint8_t status = message[0];
    uint8_t data1 = (message.size() > 1) ? message[1] : 0;
    uint8_t data2 = (message.size() > 2) ? message[2] : 0;
    
    processMidiMessage(status, data1, data2);
}

// Register note-on event callback.
void MidiHandler::setNoteOnCallback(std::function<void(MidiNote)> callback) {
    noteOnCallback = callback;
}

// Register note-off event callback.
void MidiHandler::setNoteOffCallback(std::function<void(MidiNote)> callback) {
    noteOffCallback = callback;
}

// Register control-change event callback.
void MidiHandler::setControllerChangeCallback(std::function<void(MidiController)> callback) {
    controllerChangeCallback = callback;
}

// Register pitch-bend event callback.
void MidiHandler::setPitchBendCallback(std::function<void(int, float)> callback) {
    pitchBendCallback = callback;
}

// Register MIDI clock tick callback.
void MidiHandler::setClockCallback(std::function<void()> callback) {
    clockCallback = callback;
}

// Register MIDI start callback.
void MidiHandler::setStartCallback(std::function<void()> callback) {
    startCallback = callback;
}

// Register MIDI continue callback.
void MidiHandler::setContinueCallback(std::function<void()> callback) {
    continueCallback = callback;
}

// Register MIDI stop callback.
void MidiHandler::setStopCallback(std::function<void()> callback) {
    stopCallback = callback;
}

// Increment internal clock tick counter.
void MidiHandler::updateClock() {
    midiClock.tickCount++;
}

// Mark transport clock as running.
void MidiHandler::startClock() {
    midiClock.isRunning = true;
}

// Mark transport clock as stopped.
void MidiHandler::stopClock() {
    midiClock.isRunning = false;
}

// Set transport tempo (BPM) for downstream timing consumers.
void MidiHandler::setFrameRate(float bpm) {
    midiClock.tempo = bpm;
}

// Count active note flags currently held in note-state table.
int MidiHandler::getActiveNoteCount() const {
    int count = 0;
    for (bool isActive : activeNotes) {
        if (isActive) count++;
    }
    return count;
}

// Query whether a specific MIDI note is currently active.
bool MidiHandler::isNoteActive(int noteNumber) const {
    if (noteNumber >= 0 && noteNumber < 128) {
        return activeNotes[noteNumber];
    }
    return false;
}

// Return immutable reference to current MIDI clock state.
const MidiClock& MidiHandler::getMidiClock() const {
    return midiClock;
}