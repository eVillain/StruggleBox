#ifndef PARTICLES_H
#define PARTICLES_H

#include "ParticleSys.h"
#include <string>
#include <map>
#include <memory>

class Locator;
class Renderer;

class Particles
{
public:
    Particles(Locator& locator);
    ~Particles();
    
    std::shared_ptr<ParticleSys> create(const std::string filePath,
                                        const std::string fileName);
    int getSystemID(std::shared_ptr<ParticleSys> system);
    void destroy(std::shared_ptr<ParticleSys> system);
    void destroy(const int sysID);
    void Update(const double deltaTime);
    void Draw(Renderer* renderer);
    void drawUnlit(Renderer* renderer);
    void drawLit(Renderer* renderer);

    bool paused() const { return _paused; };
    void pause() { _paused = true; };
    void resume() { _paused = false; };
private:
    Locator& _locator;
    std::map<int, std::shared_ptr<ParticleSys>> _systems;
    bool _paused;
    static int _nextParticleSysID;
};

#endif /* PARTICLES_H */
