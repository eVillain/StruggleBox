//
//  SkyDome.h
//  Ingenium
//
//  Created by The Drudgerist on 31/12/13.
//  Copyright (c) 2013 The Drudgerist. All rights reserved.
//

#ifndef NGN_SKY_DOME_H
#define NGN_SKY_DOME_H

#include "GFXDefines.h"
#include "VertexBuffer.h"
class Renderer;
class Shader;
class Camera;
class Texture;
class Light3D;

class SkyDome {
    GLuint m_fbo;
    GLuint SunTextures[4];
//    GLuint m_vao, m_vbo, m_ibo;                                // Sky dome
    int numDomeVerts;
    GLuint m_sphere_vao, m_sphere_vbo, m_sphere_ibo;    // Sun sphere
    
    Texture* skyTex;
    Shader* sunShader;
    
    Light3D* sunLight;
    
    Shader* sunDepthShader;
    Shader* blurHShader;
    Shader* blurVShader;
    Shader* sunRayLensFlareHaloShader;
    
    VertexBuffer* cloudBuffer;
    int numClouds, numCloudBuffer;
    int lastCloudX, lastCloudY, lastCloudZ;
    float cloudPhase;

    VertexBuffer* vBuffer;
//    VertexBuffer* groundVBuffer;
public:

    SkyDome();
    ~SkyDome();
    
//    void SetupShaders();
    void SetupVBO();
    void SetupVBOSphere();
    void SetupSphere();
    
    void Draw( Renderer* renderer, const Camera& camera );
    void DrawClouds( Renderer* renderer );
    void Update( double delta );
    
    bool renderSky;
    bool renderClouds;
    float cloudScale;
    float cloudTimeScale;
    
    static float timeOfDay;             // Used to calculate sun position in shaders
    static float timeFactor;            // Used to calculate sun position in shaders
    static int timeScale;               // Used to calculate sun position in shaders
    static float fogDensity;
    static float sunTilt;

    static inline glm::vec3 GetSunPos( void ) {
		float time = float((timeOfDay*timeFactor) - M_PI_2);
        return glm::vec3(-cosf(time), sinf(time), sunTilt);
    }
    static inline Color GetSunColor() {
        float refractionFactor = (1.0f - sqrt(fmax(0.0f, GetSunPos().y)));
        glm::vec3 sc = 1.0f - glm::vec3(0.0f, 0.5f, 1.0f) * refractionFactor;
        return RGBAColor(sc.x, sc.y, sc.z, 1.0f);
    }
    static inline Color GetLightColor() {
        float refractionFactor = (1.0f - sqrt(fmax(0.0f, GetSunPos().y)));
        glm::vec3 lc = 1.0f - glm::vec3(0.0f, 0.25f, 0.5f) * refractionFactor;
        float diffuseIntensity = 0.75f * fmin(1.0f, fmax(0.0f, (0.03125f + GetSunPos().y) / 0.0625f));
        return RGBAColor(lc.x, lc.y, lc.z, diffuseIntensity);
    }
    static inline Color GetLightAmbient() {
        float refractionFactor = (1.0f - sqrt(fmax(0.0f, GetSunPos().y)));
        glm::vec3 lc = 1.0f - glm::vec3(0.0f, 0.25f, 0.5f) * refractionFactor;
        float ambientIntensity = 0.0625f + 0.1875f * fmin(1.0f, fmax(0.0f, (0.375f + GetSunPos().y) / 0.25f));
        return RGBAColor(lc.x, lc.y, lc.z, ambientIntensity);
    }
    static inline Color GetFogColor() {
        float refractionFactor = (sqrt(fmax(0.0f, GetSunPos().y)));
        glm::vec3 sc = glm::vec3(0.5f, 0.6f, 0.8f) * refractionFactor;
        return RGBAColor(sc.x, sc.y, sc.z, 1.0f);
    }
};

#endif /* defined(NGN_SKY_DOME_H) */
