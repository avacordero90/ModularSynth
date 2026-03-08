#ifndef GUI_H
#define GUI_H

#include <vector>
#include <string>

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
struct Control {
    std::string name;
    ControlType type;
    float minValue;
    float maxValue;
    float defaultValue;
    
    Control(const std::string& n, ControlType ctrlType, float min, float max, float def)
        : name(n), type(ctrlType), minValue(min), maxValue(max), defaultValue(def) {}
};

// Minimalist GUI class for VST plugin
class MinimalistGUI {
private:
    bool m_isVisible;
    bool m_advancedMode;
    std::vector<Control> controls;

public:
    MinimalistGUI();
    void initializeBasicControls();
    void showAdvancedMode(bool show);
    void setupAdvancedControls();
    void toggleVisibility();
    bool isAdvancedMode() const;
    const std::vector<Control>& getControls() const;
    bool isVisible() const;
};

#endif // GUI_H