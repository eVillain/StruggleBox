#pragma once

#include <map>
#include <string>

class Options;
class Renderer;

class StatTracker
{
public:
    StatTracker(Options& options);
    ~StatTracker();

    void trackFloatValue(const float value, const std::string& name);
    void trackIntValue(const int32_t value, const std::string& name);

    void UpdateStats(Renderer* renderer);
    
    const std::map<std::string, float>& getFloatStats() const { return m_floatStats; }
    const std::map<std::string, int32_t>& getIntStats() const { return m_intStats; }
private:
	Options& _options;

    std::map<std::string, float> m_floatStats;
    std::map<std::string, int32_t> m_intStats;
    
    void DisplayStats();
    void HideStats();
};
