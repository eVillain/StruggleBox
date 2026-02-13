#include "StatTracker.h"

#include "GFXHelpers.h"
#include "Options.h"
#include "Log.h"

#include <stdlib.h>
#include <string.h>

StatTracker::StatTracker(
	Options& options) :
	_options(options)
{
	//Log::Info("[StatTracker] constructor, instance at %p", this);
}
StatTracker::~StatTracker()
{
	//Log::Info("[StatTracker] destructor, instance at %p", this);
}

void StatTracker::trackFloatValue(const float value, const std::string& name)
{
	if (m_bufferedFloatStats.find(name) != m_bufferedFloatStats.end())
	{
		StatBuffer& buffer = m_bufferedFloatStats.at(name);
		buffer.index++;
		if (buffer.index >= buffer.maxValues)
		{
			buffer.index = 0;
		}
		buffer.values[buffer.index] = value;
	}
	else
	{
		m_floatStats[name] = value;
	}
}

void StatTracker::trackIntValue(const int32_t value, const std::string& name)
{
    m_intStats[name] = value;
}

void StatTracker::setValueBuffered(const std::string& name, const size_t bufferSize)
{
	if (m_bufferedFloatStats.find(name) == m_bufferedFloatStats.end())
	{
		StatBuffer buffer;
		buffer.values.resize(bufferSize);
		buffer.maxValues = bufferSize;
		buffer.index = 0;
		m_bufferedFloatStats[name] = buffer;
	}
}
