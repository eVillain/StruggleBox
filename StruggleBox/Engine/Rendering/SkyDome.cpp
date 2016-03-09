//
//  SkyDome.cpp
//  Ingenium
//
//  Created by The Drudgerist on 31/12/13.
//  Copyright (c) 2013 The Drudgerist. All rights reserved.
//

#include "SkyDome.h"
#include "HyperVisor.h"
#include "SysCore.h"
#include "Console.h"
#include "Options.h"
#include "Shader.h"
#include "ShaderManager.h"
#include "Renderer.h"
#include "RendererGLProg.h"
#include "TextureManager.h"
#include "Texture.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "glm/gtc/noise.hpp"

#include "Camera.h"

#define CLOUD_SIZE 16
#define CLOUD_LEVEL 32

#define CLOUD_CHUNK_WIDTH 32
#define CLOUD_CHUNK_HEIGHT 8

#define CLOUD_PARTICLES CLOUD_CHUNK_WIDTH*CLOUD_CHUNK_HEIGHT*CLOUD_CHUNK_WIDTH
#define MAX_CLOUDS_UPDATES 8192

float SkyDome::timeOfDay = 8.0f;                // Start off in morning
float SkyDome::timeFactor = float(M_PI/12.0f);  // 24 hour full cycle equals 2*pi radians
int SkyDome::timeScale = 0;                     // 1 = 1 hour per second
float SkyDome::sunTilt = 0.05f;                 // Alignment along North-South axis
float SkyDome::fogDensity = 0.02f;              // Thickness of fog

//const bool debugSky = false;

SkyDome::SkyDome() {

    vBuffer = NULL;
    cloudBuffer = NULL;
    renderClouds = false;
    numClouds = 0;
    numCloudBuffer = 0;
    lastCloudX = 0;
    lastCloudY = 0;
    lastCloudZ = 0;
    cloudScale = 1.0f;
    cloudTimeScale = 0.01f;

//    skyTex = TextureManager::Inst()->LoadTexture(FileUtil::GetPath()+"Data/GFX/", "skyMap1024.png");
//    glGenTextures(4, SunTextures);

//    SetupSphere();
    
    if ( vBuffer == NULL ) {
        vBuffer = new VertexBuffer(-1, RType_SkyVerts);

        SetupVBO();
//        SetupVBOSphere();
    }
    Console::AddVar(SkyDome::timeOfDay, "timeOfDay");
    Console::AddVar(SkyDome::timeScale, "timeScale");
    Console::AddVar(renderClouds, "renderClouds");
    Console::AddVar(cloudScale, "cloudScale");
    Console::AddVar(cloudTimeScale, "cloudTimeScale");
}

SkyDome::~SkyDome() {
    Console::RemoveVar( "timeOfDay" );
    Console::RemoveVar( "timeScale" );
    Console::RemoveVar( "renderClouds" );
    delete cloudBuffer;
    cloudBuffer = NULL;
//    TextureManager::Inst()->UnloadTexture(skyTex);
//    glDeleteTextures(4, SunTextures);
//    skyTex = NULL;
}
void SkyDome::SetupVBO() {
    if ( vBuffer == NULL ) return;
    float InnerRadius = 10.0f;
	float OuterRadius = 10.25f;
    unsigned int rings = 16;
    unsigned int sectors = 16;
    
    numDomeVerts = 6*rings*(sectors-1)+3*rings;
    if ( vBuffer->p_verts != NULL ) {
        delete [] vBuffer->p_verts;
    }
    vBuffer->p_verts = new glm::vec4[numDomeVerts];
    glm::vec4 va, vb, vc, vd;
	const float stepa = (float)M_PI * 2.0f / rings;
    const float startb = asin(InnerRadius / OuterRadius);
    const float stepb = ((float)(M_PI_2) - startb) / sectors;
	int pos = 0;
    // Side ring
	for(unsigned int y = 0; y < sectors-1; y++)
	{
		float b = startb + stepb * y;
		for(unsigned int x = 0; x < rings; x++)
		{
			float a = stepa * x;
			va = glm::vec4(sin(a) * cos(b), sin(b), -cos(a) * cos(b), 1.0f) * OuterRadius;
			vb = glm::vec4(sin(a + stepa) * cos(b), sin(b), -cos(a + stepa) * cos(b), 1.0f) * OuterRadius;
			vc = glm::vec4(sin(a + stepa) * cos(b + stepb), sin(b + stepb), -cos(a + stepa) * cos(b + stepb), 1.0f) * OuterRadius;
			vd = glm::vec4(sin(a) * cos(b + stepb), sin(b + stepb), -cos(a) * cos(b + stepb), 1.0f) * OuterRadius;
			vBuffer->p_verts[pos + 0] = va;
			vBuffer->p_verts[pos + 1] = vb;
			vBuffer->p_verts[pos + 2] = vc;
			pos += 3;
			vBuffer->p_verts[pos + 0] = vc;
			vBuffer->p_verts[pos + 1] = vd;
			vBuffer->p_verts[pos + 2] = va;
			pos += 3;
		}
	}
    // Top
	float b = startb + stepb * (sectors-1);
	for(unsigned int x = 0; x < rings; x++)
	{
		float a = stepa * x;
		va = glm::vec4(sin(a) * cos(b), sin(b), -cos(a) * cos(b), 1.0f) * OuterRadius;
		vb = glm::vec4(sin(a + stepa) * cos(b), sin(b), -cos(a + stepa) * cos(b), 1.0f) * OuterRadius;
		vc = glm::vec4(0.0f, OuterRadius, 0.0f, 1.0f);
		vBuffer->p_verts[pos + 0] = va;
		vBuffer->p_verts[pos + 1] = vb;
		vBuffer->p_verts[pos + 2] = vc;
		pos += 3;
	}
    vBuffer->numVerts = numDomeVerts;
    vBuffer->updated = true;
//    glGenVertexArrays(1, &m_vao);
//    glBindVertexArray(m_vao);
//	glGenBuffers(1, &m_vbo);
//	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
//	glBufferData(GL_ARRAY_BUFFER, numDomeVerts * 3 * sizeof(GLfloat), SkyDomeVertices, GL_STATIC_DRAW);
//    glEnableVertexAttribArray(0);
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//    glBindVertexArray(0);
    
}
void SkyDome::SetupVBOSphere() {
    float OuterRadius = 10.25f;
    unsigned int rings = 16;
    unsigned int sectors = 16;

    float const R = 1.0f/(float)(rings-1);
    float const S = 1.0f/(float)(sectors-1);
    unsigned int r, s;
    GLfloat* vertices = new GLfloat[rings * sectors * 3];
    GLfloat* texcoords = new GLfloat[rings * sectors * 2];
    GLfloat* v = vertices;
    GLfloat* t = texcoords;
    numDomeVerts = rings*sectors;
    
    for(r = 0; r < rings; r++) for(s = 0; s < sectors; s++) {
        double const y = sin( -M_PI_2 + M_PI * r * R );
        double const x = cos(2*M_PI * s * S) * sin( M_PI * r * R );
        double const z = sin(2*M_PI * s * S) * sin( M_PI * r * R );
        *v++ = (GLfloat)x * OuterRadius;
		*v++ = (GLfloat)y * OuterRadius;
		*v++ = (GLfloat)z * OuterRadius;
        *t++ = s*S;
        *t++ = r*R;
    }
    GLushort* indices = new GLushort[rings * sectors * 6];
    GLushort* i = indices;
    for(r = 0; r < rings-1; r++) for(s = 0; s < sectors-1; s++) {
        *i++ = r * sectors + s;
        *i++ = r * sectors + (s+1);
        *i++ = (r+1) * sectors + (s+1);
        *i++ = r * sectors + s;
        *i++ = (r+1) * sectors + (s+1);
        *i++ = (r+1) * sectors + s;
    }
//    glGenVertexArrays(1, &m_vao);
//    glBindVertexArray(m_vao);
//	glGenBuffers(1, &m_vbo);
//	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
//    glGenBuffers(1, &m_ibo);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*numDomeVerts*5, vertices, GL_STATIC_DRAW);
//    glBufferSubData(GL_ARRAY_BUFFER, sizeof(GLfloat)*numDomeVerts*3, sizeof(GLfloat)*numDomeVerts*2, texcoords);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*numDomeVerts*6, indices, GL_STATIC_DRAW);
//    glEnableVertexAttribArray(0);
//    glEnableVertexAttribArray(1);
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
//    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(sizeof(GLfloat)*numDomeVerts*3));
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//    glBindVertexArray(0);
    delete [] vertices;
    delete [] texcoords;
    delete [] indices;
}

void SkyDome::SetupSphere() {
    float radius = 15.0f;
    unsigned int rings = 32;
    unsigned int sectors = 16;
    
    float const R = 1./(float)(rings-1);
    float const S = 1./(float)(sectors-1);
    unsigned int r, s;
    GLfloat* vertices = new GLfloat[rings * sectors * 3];
    GLfloat* normals = new GLfloat[rings * sectors * 3];
//    GLfloat* texcoords = new GLfloat[rings * sectors * 3];
    GLfloat* v = vertices;
    GLfloat* n = normals;
    for(r = 0; r < rings; r++) for(s = 0; s < sectors; s++) {
        float const y = sin( -M_PI_2 + M_PI * r * R );
        float const x = cos(2*M_PI * s * S) * sin( M_PI * r * R );
        float const z = sin(2*M_PI * s * S) * sin( M_PI * r * R );
        *v++ = x * radius;
        *v++ = y * radius;
        *v++ = z * radius;
        *n++ = x;
        *n++ = y;
        *n++ = z;
    }
    GLushort* indices = new GLushort[rings * sectors * 6];
    GLushort* i = indices;
    for(r = 0; r < rings-1; r++) for(s = 0; s < sectors-1; s++) {
        *i++ = r * sectors + (s+1);
        *i++ = r * sectors + s;
        *i++ = (r+1) * sectors + (s+1);
        *i++ = (r+1) * sectors + (s+1);
        *i++ = r * sectors + s;
        *i++ = (r+1) * sectors + s;
    }
    
    glGenVertexArrays(1, &m_sphere_vao);
    glBindVertexArray(m_sphere_vao);
	glGenBuffers(1, &m_sphere_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_sphere_vbo);
    glGenBuffers(1, &m_sphere_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_sphere_ibo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*rings*sectors*3, vertices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*rings*sectors*6, indices, GL_STATIC_DRAW);

//    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(normals), normals);
//    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(normals), sizeof(texcoords), texcoords);
//    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(normals), sizeof(indices), indices);
    
    glEnableVertexAttribArray(0);
//    glEnableVertexAttribArray(1);
//    glEnableVertexAttribArray(2);
//    glEnableVertexAttribArray(3);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
//    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)sizeof(vertices));
//    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(sizeof(vertices)+sizeof(normals)));
//    glVertexAttribPointer(2, 4, GL_UNSIGNED_SHORT, GL_FALSE, 0, (GLvoid *)(sizeof(vertices)+sizeof(normals)));
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    delete [] vertices;
    delete [] normals;
//    delete [] texcoords;
    delete [] indices;
}

void SkyDome::Draw( Renderer* renderer, const Camera& camera ) {    
    if ( vBuffer != NULL ) {
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilFunc(GL_GEQUAL, Stencil_Sky, 0xFF);
        renderer->RenderSkyBuffer(vBuffer, GetSunPos());
        glDisable(GL_STENCIL_TEST);
    }
}

void SkyDome::DrawClouds( Renderer *renderer ) {
    if ( renderClouds ) {
        if ( cloudBuffer != NULL ) {
//            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
            glEnable(GL_STENCIL_TEST);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            glStencilFunc(GL_ALWAYS, Stencil_Solid, 0xFF);
            renderer->RenderVertBuffer(cloudBuffer, numClouds);
            glDisable(GL_STENCIL_TEST);
        }
    } else {
        if ( cloudBuffer != NULL ) {
            delete cloudBuffer;
            cloudBuffer = NULL;
        }
    }
}
void SkyDome::Update( double delta ) {
    timeOfDay += delta*timeScale*0.1f;
//    cloudTimer += delta*timeScale*0.1f;

    if ( timeOfDay > 24.0f ) { timeOfDay -= 24.0f; }
    if ( !renderClouds ) { return; }
    if ( cloudBuffer == NULL ) {
        cloudBuffer = new VertexBuffer(-1, RType_CloudVerts);
        cloudBuffer->c_verts = new ColorVertexData[CLOUD_PARTICLES];
    }
    if ( timeScale != 0.0f ) { cloudPhase += delta*timeScale*cloudTimeScale; }
    int newClouds = 0;

    float noiseScale = 0.1f;
    float spacingH = CLOUD_SIZE/2;
    float spacingV = CLOUD_SIZE/4;
    int cc2 = CLOUD_CHUNK_WIDTH/2;
    int ch2 = CLOUD_CHUNK_HEIGHT/2;
    float windEffect = 0.5f;
    float windMove = windEffect*cloudPhase;

    while (newClouds < MAX_CLOUDS_UPDATES ) {
        float noise = glm::simplex( glm::vec4(lastCloudX*noiseScale+windMove, lastCloudY*noiseScale, lastCloudZ*noiseScale, cloudPhase ) );
        if ( noise > 0.01f) {
            newClouds++;
            float cloudSize = ((noise-0.01f)*CLOUD_SIZE)*cloudScale;
            cloudBuffer->c_verts[numCloudBuffer++] = {
                lastCloudX*spacingH,
                lastCloudY*spacingV+CLOUD_LEVEL,
                lastCloudZ*spacingH,
                cloudSize,
                0.8f,0.8f,0.8f,1.0f };
        }
        lastCloudZ++;
        if ( lastCloudZ==cc2 ) {
            lastCloudZ=-cc2; lastCloudY++;
        }
        if ( lastCloudY==ch2 ) {
            lastCloudY=-ch2; lastCloudX++;
        }
        if ( lastCloudX==cc2 || numCloudBuffer+1 == CLOUD_PARTICLES ) {
            lastCloudX=-cc2;
            lastCloudZ=-cc2;
            lastCloudY=-ch2;
            
            cloudBuffer->numVerts = CLOUD_PARTICLES;
            numClouds = numCloudBuffer;
            numCloudBuffer=0;
            cloudBuffer->updated = true;
            break;
        }
    }
}

