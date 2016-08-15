#ifndef SCENE_H
#define SCENE_H

#include <string>

class Scene
{
public:
    Scene(const std::string sceneID);
    virtual ~Scene() {};
    const std::string GetID() const;

    virtual void Initialize();
    virtual void ReInitialize() = 0;
    void Release();

    virtual void Pause();
    virtual void Resume();
    
    virtual void Update(const double delta) = 0;
    virtual void Draw() = 0;
    
    bool IsInitialized() { return _init; };
    bool IsPaused() { return _paused; };
    
private:
    const std::string _sceneID;
    bool _init;
    bool _paused;
    
    Scene(const Scene&);            // Intentionally undefined constructor
    Scene& operator=(const Scene&); // Intentionally undefined constructor
};

#endif /* SCENE_H */
