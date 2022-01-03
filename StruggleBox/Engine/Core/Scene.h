#pragma once

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
    
    bool IsInitialized() { return m_init; };
    bool IsPaused() { return m_paused; };
    
private:
    const std::string m_sceneID;
    bool m_init;
    bool m_paused;
    
    Scene(const Scene&);            // Intentionally undefined constructor
    Scene& operator=(const Scene&); // Intentionally undefined constructor
};

