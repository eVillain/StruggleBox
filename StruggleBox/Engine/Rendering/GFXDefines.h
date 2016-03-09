#ifndef NGN_GFX_DEFINES_H
#define NGN_GFX_DEFINES_H

#include "GFXIncludes.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include "glm/gtc/quaternion.hpp"

#define DEFAULT_SCREEN_WIDTH  1280
#define DEFAULT_SCREEN_HEIGHT 720
#define ORTHO_NEARDEPTH -100.0
#define ORTHO_FARDEPTH 100.0

#define MAX_BUFFERED_LINES 1000000
#define MAX_BUFFERED_VERTS 1000000
#define MAX_CUBE_INSTANCES 1000000
#define MAX_CHUNK_VERTS 100000
#define MAX_RENDER_VERTS 250000

// Structure for colored vertex data/impostor sphere
typedef struct  {
    GLfloat x,y,z,w;
    GLfloat r,g,b,a;
} ColorVertexData;
// Structure for normaled vertex data
typedef struct  {
    GLfloat x,y,z,w;        // World pos
    GLfloat dr,dg,db,da;    // Diffuse material
    GLfloat si;             // Specular intensity
    GLfloat nx,ny,nz;       // Normal
} NormalVertexData;
// Structure for object instance data
typedef struct {
    glm::vec3 position;     // Position in world coordinates
    glm::quat rotation;     // Rotation quaternion
    glm::vec3 scale;        // Scale
} InstanceData;

// Structure for colored cube instance data
typedef struct {
    GLfloat x, y, z;        // Cube coordinates
    GLfloat s;              // Cube size 
    GLfloat rx,ry,rz,rw;    // Cube rotation quaternion
    GLfloat dr,dg,db,da;    // Cube diffuse color
    GLfloat sr,sg,sb,sa;    // Cube specular color
} CubeInstance;

// Structure for textured cube instance data
typedef struct {
    GLfloat x, y, z;        // Cube coordinates
    GLfloat s;              // Cube size
    GLfloat rx,ry,rz,rw;    // Cube rotation
    GLfloat t;              // Cube texture/type
} CubeTexInstance;


enum StencilLayer {
    Stencil_None = 0,
    Stencil_Sky = 1,
    Stencil_Solid = 2,
    Stencil_Light = 3,
    Stencil_Transparent = 4,
};
// This structure is used for OpenGL colors with alpha channel

typedef struct Color {
	GLfloat r, g, b, a;
    inline bool operator==(const Color& rhs){ return (r==rhs.r&&g==rhs.g&&b==rhs.b&&a==rhs.a); }
    inline bool operator!=(const Color& rhs){ return (r!=rhs.r||g!=rhs.g||b!=rhs.b||a!=rhs.a); }
} Color;

//// Structure for light data
//typedef struct  {
//    GLfloat x,y,z,radius;
//    GLfloat ambR,ambG,ambB,ambI;
//    GLfloat difR,difG,difB,difI;
//    GLfloat specularIntensity;
//    GLfloat shininess;
//} LightData;

static inline Color RGBAByteColor(GLuint r, GLuint g, GLuint b, GLuint a){
	Color color = {r/255.0f, g/255.0f, b/255.0f, a/255.0f};
	return color;
}
static inline Color RGBAColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a){
	Color color = {r, g, b, a};
	return color;
}

static inline Color LAColor(GLfloat l, GLfloat a){
	Color color = {l, l, l, a};
	return color;
}

const Color COLOR_NONE = {0.0f, 0.0f, 0.0f, 0.0f};
const Color COLOR_BLACK = {0.0f, 0.0f, 0.0f, 1.0f};
const Color COLOR_WHITE = {1.0f, 1.0f, 1.0f, 1.0f};
const Color COLOR_GREY = {0.5f, 0.5f, 0.5f, 1.0f};

const Color COLOR_RED = {1.0f, 0.0f, 0.0f, 1.0f};
const Color COLOR_GREEN = {0.0f, 1.0f, 0.0f, 1.0f};
const Color COLOR_BLUE = {0.0f, 0.0f, 1.0f, 1.0f};
const Color COLOR_YELLOW = {1.0f, 1.0f, 0.0f, 1.0f};
const Color COLOR_PURPLE = {1.0f, 0.0f, 1.0f, 1.0f};
const Color COLOR_CYAN = {0.0f, 1.0f, 1.0f, 1.0f};

const Color COLOR_ORANGE = {1.0f,0.5f,0.0f,1.0f};
const Color COLOR_BROWN = {0.59f, 0.29f, 0.0f, 1.0f};

const Color COLOR_UI_TEXT = {0.5f,0.5f,0.5f,1.0f};
const Color COLOR_UI_TEXT_ACTIVE = {0.5f,0.5f,0.8f,1.0f};
const Color COLOR_UI_TEXT_HIGHLIGHT = {0.8f,0.8f,1.0f,1.0f};

const Color COLOR_UI_BORDER1 = {0.0f,0.0f,0.0f,1.0f};
const Color COLOR_UI_BORDER2 = {1.0f,1.0f,1.0f,0.1f};
const Color COLOR_UI_GRADIENT1 = {0.3125f,0.3125f,0.3125f,1.0f};
const Color COLOR_UI_GRADIENT2 = {0.25f,0.25f,0.25f,1.0f};

const Color COLOR_FOG_DEFAULT = {0.5f, 0.6f, 0.8f, 1.0f};

const Color COLOR_BLOCK_LEAVES = {0.05f, 0.6f, 0.1f, 0.8f};
const Color COLOR_BLOCK_LEAVES_JUNGLE = {0.25f, 0.7f, 0.2f, 0.8f};

const Color COLOR_LINE = {0.133f, 0.133f, 0.133f, 1.0f};

const Color COLOR_STATIC_LINE = {0.086f, 0.043f, 0.043f, 1.0f};
const Color COLOR_STATIC_DARK = {0.26f, 0.22f, 0.17f, 1.0f};
const Color COLOR_STATIC_MEDIUM = {0.35f, 0.30f, 0.26f, 1.0f};
const Color COLOR_STATIC_LIGHT = {0.39f, 0.35f, 0.30f, 1.0f};
const Color COLOR_STATIC_BRIGHT = {0.67f, 0.73f, 0.8f, 1.0f};

const Color COLOR_STATIC_LINE_VIBRANT = {0.13f, 0.043f, 0.0f, 1.0f};
const Color COLOR_STATIC_DARK_VIBRANT = {0.39f, 0.26f, 0.13f, 1.0f};
const Color COLOR_STATIC_MEDIUM_VIBRANT = {0.73f, 0.35f, 0.26f, 1.0f};
const Color COLOR_STATIC_LIGHT_VIBRANT = {0.67f, 0.8f, 0.35f, 1.0f};
const Color COLOR_STATIC_BRIGHT_VIBRANT = {0.93f, 0.93f, 0.93f, 1.0f};

const Color COLOR_GREY_DARK = {0.086f, 0.086f, 0.086f, 1.0f};
const Color COLOR_GREY_MEDIUM = {0.22f, 0.22f, 0.22f, 1.0f};
const Color COLOR_GREY_LIGHT = {0.26f, 0.26f, 0.26f, 1.0f};
const Color COLOR_GREY_BRIGHT = {0.67f, 0.73f, 0.8f, 1.0f};

const Color COLOR_GREY_DARK_VIBRANT = {0.13f, 0.086f, 0.13f, 1.0f};
const Color COLOR_GREY_MEDIUM_VIBRANT = {0.30f, 0.30f, 0.30f, 1.0f};
const Color COLOR_GREY_LIGHT_VIBRANT = {0.35f, 0.35f, 0.35f, 1.0f};
const Color COLOR_GREY_BRIGHT_VIBRANT = {0.9f, 0.9f, 0.9f, 1.0f};

const Color COLOR_DANGER = {0.73f, 0.266f, 0.333f, 1.0f};
const Color COLOR_DANGER_VIBRANT = {0.93f, 0.22f, 0.26f, 1.0f};

const Color COLOR_BG_BLUE = {0.20f, 0.24f, 0.28f, 1.0f};
const Color COLOR_BG_GREY = {0.4f, 0.4f, 0.4f, 1.0f};
const Color COLOR_GRID = {0.47f, 0.5f, 0.55f, 1.0f};
const Color COLOR_GOOD = {0.27f, 0.73f, 0.33f, 1.0f};
const Color COLOR_CONSTRAINT = {0.5f, 0.75f, 0.0f, 1.0f};
const Color COLOR_PLAYER = {0.5f, 1.0f, 0.5f, 1.0f};
const Color COLOR_AGENT = {1.0f, 1.0f, 0.5f, 1.0f};

// editor colors
const Color COLOR_CREATING_SHAPE = {0.8f, 0.7f, 0.65f, 0.5f};
const Color COLOR_SELECTED_BODY = {0.5f, 1.0f, 1.0f, 1.0f};
const Color COLOR_SELECTED_SHAPE = {0.0f, 0.4f, 1.0f, 0.5f};
const Color COLOR_SELECTED_CONSTRAINT = {0.5f, 1.0f, 0.7f, 1.0f};
const Color COLOR_SELECT  = {0.4f,0.6f,1.0f,1.0f};

const Color COLOR_BB = { 0.3f, 0.5f, 0.3f, 1.0f };
//const Color DEFAULT_BODY_COLOR = {0.5f, 0.5f, 1.0f, 1.0f};
//const Color DEFAULT_SHAPE_COLOR = {0.8f, 0.8f, 0.8f, 1.0f};
//const Color DEFAULT_STATIC_COLOR = {0.333f, 0.333f, 0.333f, 1.0f};
const Color COLOR_DEFAULT_LIGHT = {1.0f, 1.0f, 0.9f, 1.0f};

const Color COLOR_BULLET = { 1.0f, 1.0f, 0.0f, 1.0f };

// This here passes one of these color structs to a fixed OpenGL pipeline
static inline void
glColorFromColor(Color color){
	glColor4fv((GLfloat *)&color);
}


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
    -0.5,-0.5, 0.0, 1.0,
     0.5,-0.5, 0.0, 1.0,
     0.5, 0.5, 0.0, 1.0,
    -0.5, 0.5, 0.0, 1.0,
};
static const GLfloat square2D_texCoords[] = {
    0.0, 0.0,
    1.0, 0.0,
    1.0, 1.0,
    0.0, 1.0,
};
//enum CubeSides {
//    Side_Right  = 1,
//    Side_Left   = 2,
//    Side_Top    = 4,
//    Side_Bottom = 8,
//    Side_Front  = 16,
//    Side_Back   = 32,
//};
static const GLfloat cube_vertices[] = {
    // front
    -0.5, -0.5, 0.5, 1.0,
    0.5, -0.5, 0.5, 1.0,
    0.5, 0.5, 0.5, 1.0,
    0.5, 0.5, 0.5, 1.0,
    -0.5, 0.5, 0.5, 1.0,
    -0.5, -0.5, 0.5, 1.0,
    // right
    0.5, -0.5, 0.5, 1.0,
    0.5, -0.5, -0.5, 1.0,
    0.5, 0.5, -0.5, 1.0,
    0.5, 0.5, -0.5, 1.0,
    0.5, 0.5,  0.5, 1.0,
    0.5, -0.5, 0.5, 1.0,
    // back
    -0.5, 0.5, -0.5, 1.0,
    0.5, 0.5, -0.5, 1.0,
    0.5, -0.5, -0.5, 1.0,
    0.5, -0.5, -0.5, 1.0,
    -0.5, -0.5, -0.5, 1.0,
    -0.5, 0.5, -0.5, 1.0,
    // left
    -0.5, -0.5,-0.5, 1.0,
    -0.5, -0.5, 0.5, 1.0,
    -0.5, 0.5, 0.5, 1.0,
    -0.5, 0.5, 0.5, 1.0,
    -0.5, 0.5,-0.5, 1.0,
    -0.5, -0.5,-0.5, 1.0,
    // bottom
    -0.5, -0.5, -0.5, 1.0,
    0.5, -0.5, -0.5, 1.0,
    0.5, -0.5, 0.5, 1.0,
    0.5, -0.5, 0.5, 1.0,
    -0.5, -0.5, 0.5, 1.0,
    -0.5, -0.5, -0.5, 1.0,
    // top
    -0.5, 0.5, 0.5, 1.0,
    0.5, 0.5, 0.5, 1.0,
    0.5, 0.5, -0.5, 1.0,
    0.5, 0.5, -0.5, 1.0,
    -0.5, 0.5, -0.5, 1.0,
    -0.5, 0.5, 0.5, 1.0,
};
static const GLfloat cube_normals[] = {
    // front
    0.0, 0.0, 1.0, 1.0,
    0.0, 0.0, 1.0, 1.0,
    0.0, 0.0, 1.0, 1.0,
    0.0, 0.0, 1.0, 1.0,
    0.0, 0.0, 1.0, 1.0,
    0.0, 0.0, 1.0, 1.0,
    // right
    1.0, 0.0, 0.0, 1.0,
    1.0, 0.0, 0.0, 1.0,
    1.0, 0.0, 0.0, 1.0,
    1.0, 0.0, 0.0, 1.0,
    1.0, 0.0, 0.0, 1.0,
    1.0, 0.0, 0.0, 1.0,
    // back
    0.0, 0.0, -1.0, 1.0,
    0.0, 0.0, -1.0, 1.0,
    0.0, 0.0, -1.0, 1.0,
    0.0, 0.0, -1.0, 1.0,
    0.0, 0.0, -1.0, 1.0,
    0.0, 0.0, -1.0, 1.0,
    // left
    -1.0, 0.0, 0.0, 1.0,
    -1.0, 0.0, 0.0, 1.0,
    -1.0, 0.0, 0.0, 1.0,
    -1.0, 0.0, 0.0, 1.0,
    -1.0, 0.0, 0.0, 1.0,
    -1.0, 0.0, 0.0, 1.0,
    // bottom
    0.0, -1.0, 0.0, 1.0,
    0.0, -1.0, 0.0, 1.0,
    0.0, -1.0, 0.0, 1.0,
    0.0, -1.0, 0.0, 1.0,
    0.0, -1.0, 0.0, 1.0,
    0.0, -1.0, 0.0, 1.0,
    // top
    0.0, 1.0, 0.0, 1.0,
    0.0, 1.0, 0.0, 1.0,
    0.0, 1.0, 0.0, 1.0,
    0.0, 1.0, 0.0, 1.0,
    0.0, 1.0, 0.0, 1.0,
    0.0, 1.0, 0.0, 1.0,
};
static const GLfloat cube_texcoords[] = {
    // front
    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    // right
    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    // back
    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    // left
    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    // bottom
    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    // top
    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
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


#endif
