#ifndef RENDERER_BASE_H
#define RENDERER_BASE_H

#include "GFXIncludes.h"
#include "GFXDefines.h"
#include "Color.h"
#include "Rect2D.h"
#include "MaterialData.h"
#include "Light3D.h"
#include <string>
#include <vector>
#include <memory>

class VertBuffer;
class Mesh;
class Texture;
class Sprite;
class SpriteBatch;
struct SDL_Window;

enum VertexDataType
{
	ColorVerts,
	MeshVerts,
	TexturedVerts,
	SpriteVerts,
	SphereVerts,
	InstanceVerts,
	InstancedCubeVerts,
};

struct BlendMode
{
	GLuint srcFunc;
	GLuint dstFunc;
	bool enable;
};

struct DepthMode
{
	bool enable;
	bool mask;
};

struct DrawPackage
{
	VertexDataType type;
	GLuint buffer;
	unsigned int rangeEnd;
	unsigned int rangeStart;
	GLuint texture;
	BlendMode blendMode;
	DepthMode depthMode;
	bool render3D;
};

struct InstancedDrawPackage
{
	GLuint instanceBuffer;
	unsigned int instanceCount;
	VertexDataType type;
	GLuint buffer;
	unsigned int rangeEnd;
	unsigned int rangeStart;
	GLuint texture;
	BlendMode blendMode;
	DepthMode depthMode;
};

class Renderer
{
public:
    virtual ~Renderer() { };
    
    virtual void Initialize() = 0;
    virtual void ShutDown() = 0;
    
    // Start and end frame drawing
    virtual void BeginDraw() = 0;
    virtual void EndDraw() = 0;

	virtual void RenderLightingFX(
		const glm::mat4& model,
		const glm::mat4& projection,
		const glm::vec4& viewPort,
		const glm::vec3& position,
		const glm::vec2& ratio,
		const float nearDepth,
		const float farDepth) = 0;

    virtual void PostProcess() = 0;
    virtual void RenderFromTexture( const GLuint tex ) = 0;
    // Matrix functionality
    virtual void GetUIMatrix( glm::mat4& target ) = 0;      // Shaders
    virtual void GetGameMatrix( glm::mat4& target ) = 0;    // Shaders

	// New vertex array methods
	virtual GLuint addVertexArray() = 0;

	// New vertex buffer methods
	virtual std::shared_ptr<VertBuffer> addVertBuffer(const VertexDataType type) = 0;
	virtual void queueMesh(std::shared_ptr<Mesh> mesh) = 0;
	virtual void queueDeferredBuffer(
		const VertexDataType type,
		const GLuint buffer,
		const unsigned int rangeEnd,
		const unsigned int rangeStart = 0,
		const GLuint tex = 0,
		const BlendMode blendMode = { 0,0,0 },
		const DepthMode depthMode = { true,true }) = 0;

	virtual void queueForwardBuffer(
		const VertexDataType type,
		const GLuint buffer,
		const unsigned int rangeEnd,
		const unsigned int rangeStart = 0,
		const GLuint tex = 0,
		const BlendMode blendMode = { 0,0,0 },
		const DepthMode depthMode = { true,true },
		const bool render3D = false) = 0;

	virtual void queueDeferredInstances(
		const GLuint instanceBuffer,
		const unsigned int instanceCount,
		const VertexDataType type,
		const GLuint buffer,
		const unsigned int rangeEnd,
		const unsigned int rangeStart = 0,
		const GLuint tex = 0,
		const BlendMode blendMode = { 0,0,0 },
		const DepthMode depthMode = { true,true }) = 0;
	
	// New lighting methods
	virtual void queueLights(
		const LightInstance* lights,
		const unsigned int lightCount) = 0;

	virtual void flush() = 0;

    // Vertex buffer operations
	virtual void renderVertBuffer(
		std::shared_ptr<VertBuffer> buffer,
		const unsigned int rangeEnd,
		const unsigned int rangeStart = 0,
		const Texture* tex = NULL,
		const bool render3D = true) = 0;

    // General triangle buffering functions
    //virtual void BufferVerts( const ColorVertexData* verts, const int numVerts ) = 0;
    //virtual void BufferVerts( const NormalVertexData* verts, const int numVerts ) = 0;
    virtual void BufferSpheres( const SphereVertexData* spheres, const int numSpheres ) = 0;
    virtual void RenderVerts() = 0;
    virtual void renderSpheres(
		const glm::mat4& view,
		const glm::mat4& projection,
		const glm::mat3& normalMatrix,
		const glm::vec3& position) = 0;
    // 2D Buffering functions
    virtual void Buffer2DLine( glm::vec2 a, glm::vec2 b, Color aColor, Color bColor, float z = 0.0f ) = 0;
    virtual void Render2DLines() = 0;
    // 3D Buffering functions
    virtual void Buffer3DLine( glm::vec3 a, glm::vec3 b, Color aColor, Color bColor ) = 0;
    virtual void render3DLines(const glm::mat4& mvp) = 0;

    virtual void bufferCubes(const CubeInstance* cubes, const size_t count) = 0;
    virtual void renderCubes(const glm::mat4& mvp) = 0;

	// 2D Drawing functions ( drawn immediately )
    virtual void DrawPolygon(const int count,
                             const GLfloat *verts,
                             const Color lineColor,
                             const Color fillColor) = 0;
    virtual void DrawPolygon(const int count,
                             const glm::vec2 *verts,
                             const Color lineColor,
                             const Color fillColor,
                             const float z = 0.0f) = 0;
    virtual void Draw2DRect( Rect2D rect,
                            Color lineColor = COLOR_NONE, Color fillColor = COLOR_WHITE, float z = 0.0f ) = 0;
    virtual void Draw2DRect(glm::vec2 center, float width, float height,
                            Color lineColor = COLOR_NONE, Color fillColor = COLOR_WHITE, float z = 0.0f ) = 0;
    virtual void Draw2DRect3D( glm::vec3 center, float width, float height,
                              Color lineColor = COLOR_NONE, Color fillColor = COLOR_WHITE, float z = 0.0f ) = 0;
    virtual void Draw2DProgressBar( glm::vec3 center, float width, float height, float amount,
                           Color lineColor = COLOR_NONE, Color fillColor = COLOR_WHITE, float z = 0.0f ) = 0;

    // Circles and grids
    virtual void DrawCircle( glm::vec2 center, float angle, float radius,
                    Color lineColor, Color fillColor, float z = 0.0f, const int pixelPerSeg=8 ) = 0;
    virtual void DrawRing( glm::vec2 center, float radius1, float radius2, int segs,
                  Color lineColor = COLOR_NONE, Color fillColor = COLOR_WHITE, float z = 0.0f ) = 0;
    virtual void DrawGrid( float gridSize, Rect2D rect, int subDivisions, Color color = COLOR_WHITE ) = 0;
    virtual void Draw3DGrid( const glm::vec3& pos, const float size, const int divisions ) = 0;
    // 2D Drawing textured
    virtual void DrawSprite( const Sprite& sprite ) = 0;
    virtual void DrawSpriteBatch( const SpriteBatch& batch ) = 0;

    virtual void DrawImage( const glm::vec2 center, const float width, const float height,
                           const std::string texName, const float z = 0.0f, const Color color = COLOR_WHITE) = 0;
    virtual void DrawTexture(
		const Rect2D rect,
		const Rect2D texRect,
		const GLuint tex,
		const glm::mat4& mvp,
		const float z = 0.0f,
		const Color color = COLOR_WHITE) = 0;
    virtual void DrawTextureArray( const Rect2D rect, const Rect2D texRect,
                                  const int width, const int height,
                                 const GLbyte* data, const float z = 0.0f, const Color color = COLOR_WHITE) = 0;
    // 3D Drawing functions ( drawn immediately )
    virtual void DrawBoxOutline( glm::vec3 center, glm::vec3 boxSize, Color color ) = 0;
    // Get cursor 3D coordinates from screen pos
    virtual const glm::vec3 GetCursor3DPos( const glm::vec2 cursorPos ) const = 0;
    
    // Common accessors
    virtual const std::string GetInfo() const = 0;
	virtual MaterialData& getMaterials() = 0;
	virtual void refreshMaterials() = 0;

	virtual void setRoomSize(float size) = 0;

protected:
    // Variables    
    bool initialized;
    bool shouldClose;

    int windowWidth, windowHeight;
    // Variables for the current camera attributes
    GLfloat r_camX, r_camY, r_camZoom;
    // Variables for the wanted camera attributes
    GLfloat r_camTX, r_camTY, r_camTZoom;
    
    // Statistics (per-frame)
    double r_frameStartTime;        // Timestamp from previous frame for delta time calculation
    double r_frameCounterTime;      // Timestamp from last time FPS counter was updated
    int r_frameCounter;             // Total number of frames rendered
    
    unsigned int renderedTris;      // Total number of triangles rendered this frame
    unsigned int renderedSegs;      // Total number of line segments rendered this frame
    unsigned int renderedSprites;   // Total number of sprites rendered this frame
};

#endif
