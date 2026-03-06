#include "wavetable_manager.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <algorithm>

WavetableManager::WavetableManager() = default;

WavetableManager::~WavetableManager() {
    for (auto& pair : wavetables) {
        delete pair.second;
    }
}

void WavetableManager::addWavetable(const std::string& name, const std::vector<float>& data) {
    if (wavetables.find(name) != wavetables.end()) {
        // Remove existing
        delete wavetables[name];
    }
    
    wavetables[name] = new Wavetable(name, data);
    wavetableNames.push_back(name);
}

void WavetableManager::removeWavetable(const std::string& name) {
    auto it = wavetables.find(name);
    if (it != wavetables.end()) {
        delete it->second;
        wavetables.erase(it);
        
        // Remove from names list
        auto nameIt = std::find(wavetableNames.begin(), wavetableNames.end(), name);
        if (nameIt != wavetableNames.end()) {
            wavetableNames.erase(nameIt);
        }
    }
}

Wavetable* WavetableManager::getWavetable(const std::string& name) const {
    auto it = wavetables.find(name);
    return (it != wavetables.end()) ? it->second : nullptr;
}

bool WavetableManager::hasWavetable(const std::string& name) const {
    return wavetables.find(name) != wavetables.end();
}

const std::vector<std::string>& WavetableManager::getWavetableNames() const {
    return wavetableNames;
}

size_t WavetableManager::getNumWavetables() const {
    return wavetables.size();
}

void WavetableManager::generateSineWave(const std::string& name, size_t length) {
    std::vector<float> data(length);
    for (size_t i = 0; i < length; ++i) {
        data[i] = sinf(2.0f * M_PI * static_cast<float>(i) / static_cast<float>(length));
    }
    addWavetable(name, data);
}

void WavetableManager::generateSquareWave(const std::string& name, size_t length) {
    std::vector<float> data(length);
    for (size_t i = 0; i < length; ++i) {
        data[i] = (static_cast<float>(i) / static_cast<float>(length)) < 0.5f ? 1.0f : -1.0f;
    }
    addWavetable(name, data);
}

void WavetableManager::generateSawtoothWave(const std::string& name, size_t length) {
    std::vector<float> data(length);
    for (size_t i = 0; i < length; ++i) {
        data[i] = 2.0f * static_cast<float>(i) / static_cast<float>(length) - 1.0f;
    }
    addWavetable(name, data);
}

void WavetableManager::generateTriangleWave(const std::string& name, size_t length) {
    std::vector<float> data(length);
    for (size_t i = 0; i < length; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(length);
        data[i] = (t < 0.25f) ? 4.0f * t : 
                 (t < 0.75f) ? 2.0f - 4.0f * t : 
                 4.0f * t - 4.0f;
    }
    addWavetable(name, data);
}

void WavetableManager::loadWavetableFile(const std::string& filename) {
    // Implementation for loading wavetables from external files
    std::cout << "Loading wavetable from: " << filename << std::endl;
    // This would typically parse a file format containing wavetable data
}

void WavetableManager::saveWavetableFile(const std::string& name, const std::string& filename) {
    // Implementation for saving a wavetable to external file
    std::cout << "Saving wavetable " << name << " to: " << filename << std::endl;
    // This would typically write the wavetable data to a file in a specific format
}