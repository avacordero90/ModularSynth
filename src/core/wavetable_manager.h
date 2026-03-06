#ifndef WAVETABLE_MANAGER_H
#define WAVETABLE_MANAGER_H

#include "wavetable.h"
#include <unordered_map>
#include <vector>
#include <string>

class WavetableManager {
private:
    std::unordered_map<std::string, Wavetable*> wavetables;
    std::vector<std::string> wavetableNames;
    
public:
    WavetableManager();
    ~WavetableManager();
    
    // Wavetable management
    void addWavetable(const std::string& name, const std::vector<float>& data);
    void removeWavetable(const std::string& name);
    Wavetable* getWavetable(const std::string& name) const;
    bool hasWavetable(const std::string& name) const;
    
    // Wavetable list operations
    const std::vector<std::string>& getWavetableNames() const;
    size_t getNumWavetables() const;
    
    // Wavetable generation utilities
    void generateSineWave(const std::string& name, size_t length);
    void generateSquareWave(const std::string& name, size_t length);
    void generateSawtoothWave(const std::string& name, size_t length);
    void generateTriangleWave(const std::string& name, size_t length);
    
    // Load wavetables from external sources
    void loadWavetableFile(const std::string& filename);
    void saveWavetableFile(const std::string& name, const std::string& filename);
};

#endif // WAVETABLE_MANAGER_H