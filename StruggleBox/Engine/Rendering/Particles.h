#ifndef PARTICLES_H
#define PARTICLES_H

#include "ParticleSys.h"
#include <string>
#include <map>
#include <memory>

class Renderer;

class Particles
{
public:
    Particles(std::shared_ptr<Renderer> renderer);
    ~Particles();
    
    std::shared_ptr<ParticleSys> create(const std::string filePath,
                                        const std::string fileName);
    int getSystemID(std::shared_ptr<ParticleSys> system);
    void destroy(std::shared_ptr<ParticleSys> system);
    void destroy(const int sysID);
    void Update(const double deltaTime);
    void Draw();
    void drawUnlit();
    void drawLit();

    bool paused() const { return _paused; };
    void pause() { _paused = true; };
    void resume() { _paused = false; };

private:
	std::shared_ptr<Renderer> _renderer;
    std::map<int, std::shared_ptr<ParticleSys>> _systems;
    bool _paused;
    static int _nextParticleSysID;
};

#endif /* PARTICLES_H */
