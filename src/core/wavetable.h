#ifndef WAVETABLE_H
#define WAVETABLE_H

#include <vector>
#include <string>

class Wavetable {
private:
    std::string name;
    std::vector<float> data;
    size_t length;
    
public:
    Wavetable(const std::string& name, const std::vector<float>& data);
    ~Wavetable();
    
    // Getters
    const std::string& getName() const;
    size_t getLength() const;
    float getValue(size_t index) const;
    
    // Interpolation methods
    float getInterpolatedValue(float index) const;
    
    // Wavetable manipulation
    void resize(size_t newLength);
    void update();
};

#endif // WAVETABLE_H