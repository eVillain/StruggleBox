#pragma once

#include <map>
#include <string>
#include <vector>

class Options;

struct StatBuffer {
    std::vector<float> values;
    size_t maxValues;
    size_t index;
};

class StatTracker
{
public:
    StatTracker(Options& options);
    ~StatTracker();

    void trackFloatValue(const float value, const std::string& name);
    void trackIntValue(const int32_t value, const std::string& name);
    void setValueBuffered(const std::string& name, const size_t bufferSize);

    const std::map<std::string, float>& getFloatStats() const { return m_floatStats; }
    const std::map<std::string, int32_t>& getIntStats() const { return m_intStats; }
    const std::map<std::string, StatBuffer>& getBufferedStats() const { return m_bufferedFloatStats; }

private:
	Options& _options;

    std::map<std::string, float> m_floatStats;
    std::map<std::string, int32_t> m_intStats;
    std::map<std::string, StatBuffer> m_bufferedFloatStats;
};
