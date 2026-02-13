#pragma once

#include "CoreIncludes.h"
#include "RendererDefines.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <glm/gtc/matrix_transform.hpp>

const GLfloat ORTHO_NEARDEPTH = -100.0f;
const GLfloat ORTHO_FARDEPTH = 100.0f;

enum StencilLayer {
    Stencil_None = 0,
    Stencil_Sky = 1,
    Stencil_Solid = 2,
    Stencil_Light = 3,
    Stencil_Transparent = 4,
};

/*****************************************************
 *              STATIC VERTS
 *****************************************************/
static const GLfloat square3D_vertices[] = {
    -0.5, -0.5, 0, 1,   // bottom left
     0.5, -0.5, 0, 1,   // bottom right
     0.5,  0.5, 0, 1,   // top right
    -0.5,  0.5, 0, 1,   // top left
};
static const GLfloat square3D_texCoords[] = {
    0.0, 1.0,  // mapping coordinates for top left
    0.0, 0.0,  // mapping coordinates for bottom left
    1.0, 1.0,  // mapping coordinates for top right
    1.0, 0.0,  // mapping coordinates for bottom right
};
static const GLfloat square2D_coords[] = {
    -0.5,-0.5, 0.0,
     0.5,-0.5, 0.0,
     0.5, 0.5, 0.0,
    -0.5, 0.5, 0.0,
};
static const GLfloat square2D_texCoords[] = {
    0.0, 0.0,
    1.0, 0.0,
    1.0, 1.0,
    0.0, 1.0,
};

static const GLfloat square2D_verts[] = {
    -0.5, -0.5, 0.0, 0.0, 0.0,
     0.5, -0.5, 0.0, 1.0, 0.0,
     0.5,  0.5, 0.0, 1.0, 1.0,
    -0.5,  0.5, 0.0, 0.0, 1.0,
};

typedef struct {
	GLfloat x, y, z;
} CubeTexVert;

typedef struct {
	CubeTexVert v1, v2, v3, v4;
} CubeTexVerts;

const CubeTexVerts CubeMapTexCoords[6] =
{
	{   // +X
		1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f
	},
	{   // -X
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f
	},
	{   // +Y
		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f
	},
	{   // -Y
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f
	},
	{   // +Z
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f
	},
	{   // -Z
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f
	}
};

// These are nice static vertices for rendering circles
static const GLfloat circle2D_verts[] = {
    0.0000f,  1.0000f,
    0.2588f,  0.9659f,
    0.5000f,  0.8660f,
    0.7071f,  0.7071f,
    0.8660f,  0.5000f,
    0.9659f,  0.2588f,
    1.0000f,  0.0000f,
    0.9659f, -0.2588f,
    0.8660f, -0.5000f,
    0.7071f, -0.7071f,
    0.5000f, -0.8660f,
    0.2588f, -0.9659f,
    0.0000f, -1.0000f,
	-0.2588f, -0.9659f,
	-0.5000f, -0.8660f,
	-0.7071f, -0.7071f,
	-0.8660f, -0.5000f,
	-0.9659f, -0.2588f,
	-1.0000f, -0.0000f,
	-0.9659f,  0.2588f,
	-0.8660f,  0.5000f,
	-0.7071f,  0.7071f,
	-0.5000f,  0.8660f,
	-0.2588f,  0.9659f,
    0.0000f,  1.0000f,
    0.0f, 0.0f, // For an extra line to see the rotation.
};
static const int circle2D_count = sizeof(circle2D_verts)/sizeof(GLfloat)/2;

// These are nice static vertices for rendering fat lines
static const GLfloat pillVAR[] = {
    0.0000f,  1.0000f, 1.0f,
    0.2588f,  0.9659f, 1.0f,
    0.5000f,  0.8660f, 1.0f,
    0.7071f,  0.7071f, 1.0f,
    0.8660f,  0.5000f, 1.0f,
    0.9659f,  0.2588f, 1.0f,
    1.0000f,  0.0000f, 1.0f,
    0.9659f, -0.2588f, 1.0f,
    0.8660f, -0.5000f, 1.0f,
    0.7071f, -0.7071f, 1.0f,
    0.5000f, -0.8660f, 1.0f,
    0.2588f, -0.9659f, 1.0f,
    0.0000f, -1.0000f, 1.0f,
    
    0.0000f, -1.0000f, 0.0f,
	-0.2588f, -0.9659f, 0.0f,
	-0.5000f, -0.8660f, 0.0f,
	-0.7071f, -0.7071f, 0.0f,
	-0.8660f, -0.5000f, 0.0f,
	-0.9659f, -0.2588f, 0.0f,
	-1.0000f, -0.0000f, 0.0f,
	-0.9659f,  0.2588f, 0.0f,
	-0.8660f,  0.5000f, 0.0f,
	-0.7071f,  0.7071f, 0.0f,
	-0.5000f,  0.8660f, 0.0f,
	-0.2588f,  0.9659f, 0.0f,
    0.0000f,  1.0000f, 0.0f,
};
static const int pillVAR_count = sizeof(pillVAR)/sizeof(GLfloat)/3;

// Static verts for a spring shape
static const GLfloat springVAR[] = {
    0.00f, 0.0f,
    0.20f, 0.0f,
    0.25f, 3.0f,
    0.30f,-6.0f,
    0.35f, 6.0f,
    0.40f,-6.0f,
    0.45f, 6.0f,
    0.50f,-6.0f,
    0.55f, 6.0f,
    0.60f,-6.0f,
    0.65f, 6.0f,
    0.70f,-3.0f,
    0.75f, 6.0f,
    0.80f, 0.0f,
    1.00f, 0.0f,
};
static const int springVAR_count = sizeof(springVAR)/sizeof(GLfloat)/2;

//========================================================================
// Texture declarations (we hard-code them into the source code, since
// they are so simple)
//========================================================================

#define P_TEX_WIDTH  8    // Particle texture dimensions
#define P_TEX_HEIGHT 8
#define C_TEX_WIDTH  16   // Checkerboard texture dimensions
#define C_TEX_HEIGHT 16

// Particle texture (a simple spot)
const unsigned char particle_texture[ P_TEX_WIDTH * P_TEX_HEIGHT ] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x11, 0x22, 0x22, 0x11, 0x00, 0x00,
    0x00, 0x11, 0x33, 0x88, 0x77, 0x33, 0x11, 0x00,
    0x00, 0x22, 0x88, 0xff, 0xee, 0x77, 0x22, 0x00,
    0x00, 0x22, 0x77, 0xee, 0xff, 0x88, 0x22, 0x00,
    0x00, 0x11, 0x33, 0x77, 0x88, 0x33, 0x11, 0x00,
    0x00, 0x00, 0x11, 0x33, 0x22, 0x11, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Checker texture (your basic checkerboard textzre)
const unsigned char checker_texture[ C_TEX_WIDTH * C_TEX_HEIGHT ] = {
    0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0xff, 0xf0, 0xcc, 0xf0, 0xf0, 0xf0, 0xff, 0xf0, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0xf0, 0xcc, 0xee, 0xff, 0xf0, 0xf0, 0xf0, 0xf0, 0x30, 0x66, 0x30, 0x30, 0x30, 0x20, 0x30, 0x30,
    0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xee, 0xf0, 0xf0, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0xf0, 0xf0, 0xf0, 0xf0, 0xcc, 0xf0, 0xf0, 0xf0, 0x30, 0x30, 0x55, 0x30, 0x30, 0x44, 0x30, 0x30,
    0xf0, 0xdd, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0x33, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xf0, 0xf0, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x60, 0x30,
    0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0x33, 0x33, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x33, 0x30, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x20, 0x30, 0x30, 0xf0, 0xff, 0xf0, 0xf0, 0xdd, 0xf0, 0xf0, 0xff,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x55, 0x33, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xf0, 0xf0,
    0x30, 0x44, 0x66, 0x30, 0x30, 0x30, 0x30, 0x30, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0xf0, 0xf0, 0xf0, 0xaa, 0xf0, 0xf0, 0xcc, 0xf0,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0xff, 0xf0, 0xf0, 0xf0, 0xff, 0xf0, 0xdd, 0xf0,
    0x30, 0x30, 0x30, 0x77, 0x30, 0x30, 0x30, 0x30, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
};

// Light matrices to look in all 6 directions for cubemapping
static const glm::mat4 lightViewMatrix[6] = {
    glm::lookAt(glm::vec3(0.0f,0.0f,0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f,-1.0f, 0.0f)),  // +x
    glm::lookAt(glm::vec3(0.0f,0.0f,0.0f), glm::vec3(-1.0f,0.0f, 0.0f), glm::vec3(0.0f,-1.0f, 0.0f)),  // -x
    glm::lookAt(glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),  // +y
    glm::lookAt(glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,-1.0f, 0.0f), glm::vec3(0.0f, 0.0f,-1.0f)),  // -y
    glm::lookAt(glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f,-1.0f, 0.0f)),  // +z
    glm::lookAt(glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f, 0.0f,-1.0f), glm::vec3(0.0f,-1.0f, 0.0f))   // -z
};
