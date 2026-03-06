#include "gui.h"
#include <iostream>
#include <string>

// Minimalist GUI Implementation
MinimalistGUI::MinimalistGUI() : m_isVisible(true), m_advancedMode(false) {
    // Initialize basic UI elements
    initializeBasicControls();
}

void MinimalistGUI::initializeBasicControls() {
    // Clear existing controls
    controls.clear();
    
    // Create basic controls that are visible by default
    controls.push_back({"Frequency", ControlType::SLIDER, 20.0f, 20000.0f, 440.0f});
    controls.push_back({"Detune", ControlType::SLIDER, -1200.0f, 1200.0f, 0.0f});
    controls.push_back({"Waveform", ControlType::COMBOBOX, 0.0f, 5.0f, 0.0f});
    controls.push_back({"Filter Freq", ControlType::SLIDER, 20.0f, 20000.0f, 1000.0f});
    controls.push_back({"Filter Res", ControlType::SLIDER, 0.1f, 10.0f, 0.707f});
    controls.push_back({"Envelope Attack", ControlType::SLIDER, 0.001f, 5.0f, 0.01f});
    controls.push_back({"Envelope Decay", ControlType::SLIDER, 0.001f, 5.0f, 0.1f});
}

void MinimalistGUI::showAdvancedMode(bool show) {
    m_advancedMode = show;
    if (m_advancedMode) {
        // Show all controls when entering advanced mode
        setupAdvancedControls();
    } else {
        // Restore basic controls only
        initializeBasicControls();
    }
}

void MinimalistGUI::setupAdvancedControls() {
    // Add more detailed controls for advanced users
    controls.push_back({"Filter Type", ControlType::COMBOBOX, 0.0f, 7.0f, 0.0f});
    controls.push_back({"Envelope Sustain", ControlType::SLIDER, 0.0f, 1.0f, 0.7f});
    controls.push_back({"Envelope Release", ControlType::SLIDER, 0.001f, 5.0f, 0.3f});
    controls.push_back({"Wavetable Select", ControlType::COMBOBOX, 0.0f, 100.0f, 0.0f});
    controls.push_back({"Wavetable Morph", ControlType::SLIDER, 0.0f, 1.0f, 0.5f});
    // Add more advanced parameters as needed
}

void MinimalistGUI::toggleVisibility() {
    m_isVisible = !m_isVisible;
}

bool MinimalistGUI::isAdvancedMode() const {
    return m_advancedMode;
}

const std::vector<Control>& MinimalistGUI::getControls() const {
    return controls;
}

bool MinimalistGUI::isVisible() const {
    return m_isVisible;
}