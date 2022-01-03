#pragma once

#include <string>

struct ParticleSystemConfig;

class ParticleSystemLoader
{
public:
    static ParticleSystemConfig load(const std::string& fileName);
    static void save(const ParticleSystemConfig& config, const std::string& fileName);
};
