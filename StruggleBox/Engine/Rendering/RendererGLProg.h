#ifndef RENDERER_GL_PROG_H
#define RENDERER_GL_PROG_H

#include "Renderer.h"
#include "VertBuffer.h"
#include "VertexData.h"
#include "GBuffer.h"
#include "ReflectionProbe.h"
#include "MaterialTexture.h"

#include <vector>
#include <map>
#include <memory>

class Options;
class StatTracker;
class LightSystem3D;
class Camera;
class ShaderManager;

class Shader;
class Texture;
class Sprite;
class Mesh;

// Rendering modes
enum RenderMode {
    RM_Final_Image  = 0,
    RM_Diffuse      = 1,
    RM_Specular     = 2,
    RM_Normal       = 3,
    RM_Depth        = 4,
    RM_Stencil      = 5,
    RM_SSAO         = 6,
    RM_Shadowmap    = 7,
};


class RendererGLProg : public Renderer
{
public:
    RendererGLProg(
		std::shared_ptr<Options> options,
		std::shared_ptr<Camera> camera,
		std::shared_ptr<LightSystem3D> lights,
		std::shared_ptr<ShaderManager> shaders);
    ~RendererGLProg();
    
    void Initialize();
    void ShutDown();

    void Resize(const int width, const int height);
    // Start and end frame drawing
    void BeginDraw();
    void EndDraw();
    void UpdateStats();
    


	void RenderLightingFX(
		const glm::mat4& model,
		const glm::mat4& projection,
		const glm::vec4& viewPort,
		const glm::vec3& position,
		const glm::vec2& ratio,
		const float nearDepth,
		const float farDepth);

    void PostProcess();
    void RenderFromTexture( const GLuint tex );
    
    void PassSSAO( const GLuint fbo );
    void PassStencil( const GLuint fbo );
    void PassLightRays( glm::vec3 lightWorldPos, const GLuint fbo );
    
    // Matrix functionality
    void GetUIMatrix( glm::mat4& target );
    void GetGameMatrix( glm::mat4& target );
    glm::mat4 GetLightMVP(LightInstance& light);
    glm::mat4 GetLightModel(LightInstance& light);
    glm::mat4 GetLightView(LightInstance& light);
    glm::mat4 GetLightProjection(LightInstance& light);

	// New vertex array methods
	GLuint addVertexArray();

	// New vertex buffer methods
	std::shared_ptr<VertBuffer> addVertBuffer(const VertexDataType type);
	void queueMesh(std::shared_ptr<Mesh> mesh);

	void queueDeferredBuffer(
		const VertexDataType type,
		const GLuint buffer,
		const unsigned int rangeEnd,
		const unsigned int rangeStart = 0,
		const GLuint tex = 0,
		const BlendMode blendMode = { 0,0,0 },
		const DepthMode depthMode = { true,true });

	void queueForwardBuffer(
		const VertexDataType type,
		const GLuint buffer,
		const unsigned int rangeEnd,
		const unsigned int rangeStart = 0,
		const GLuint tex = 0,
		const BlendMode blendMode = { 0,0,0 },
		const DepthMode depthMode = { true,true },
		const bool render3D = false);

	void queueDeferredInstances(
		const GLuint instanceBuffer,
		const unsigned int instanceCount,
		const VertexDataType type,
		const GLuint buffer,
		const unsigned int rangeEnd,
		const unsigned int rangeStart = 0,
		const GLuint tex = 0,
		const BlendMode blendMode = { 0,0,0 },
		const DepthMode depthMode = { true,true });

	void queueLights(
		const LightInstance* lights,
		const unsigned int lightCount);

	void flush();

    // Vertex buffer operations
	void renderVertBuffer(
		std::shared_ptr<VertBuffer> buffer,
		const unsigned int rangeEnd,
		const unsigned int rangeStart = 0,
		const Texture* tex = NULL,
		const bool render3D = true);

    // General triangle buffering functions
    //void BufferVerts( const ColorVertexData* verts, const int numVerts );
    //void BufferVerts( const NormalVertexData* verts, const int numVerts );
    void BufferSpheres( const SphereVertexData* spheres, const int numSpheres );

    void RenderVerts();
    void renderSpheres(
		const glm::mat4& view,
		const glm::mat4& projection,
		const glm::mat3& normalMatrix,
		const glm::vec3& position);

    // 2D Buffering functions
    void Buffer2DLine( glm::vec2 a, glm::vec2 b, Color aColor, Color bColor, float z = 0.0f );
    void Render2DLines();
    
    // 3D Buffering functions
    void Buffer3DLine( glm::vec3 a, glm::vec3 b, Color aColor, Color bColor );
    void render3DLines(const glm::mat4& mvp);

    void bufferCubes(const CubeInstance* cubes, const size_t count);
    void renderCubes(const glm::mat4& mvp);

	// 2D Drawing functions ( drawn immediately )
    void DrawPolygon(const int count,
                     const GLfloat *verts,
                     const Color lineColor,
                     const Color fillColor);
    void DrawPolygon(const int count,
                     const glm::vec2 *verts,
                     const Color lineColor,
                     const Color fillColor,
                     const float z = 0.0f);

    void Draw2DRect( Rect2D rect,
                  Color lineColor = COLOR_NONE, Color fillColor = COLOR_WHITE, float z = 0.0f );
    void Draw2DRect( glm::vec2 center, float width, float height,
                  Color lineColor = COLOR_NONE, Color fillColor = COLOR_WHITE, float z = 0.0f );
    void Draw2DRect3D( glm::vec3 center, float width, float height,
                  Color lineColor = COLOR_NONE, Color fillColor = COLOR_WHITE, float z = 0.0f );
    void Draw2DProgressBar( glm::vec3 center, float width, float height, float amount,
                      Color lineColor = COLOR_NONE, Color fillColor = COLOR_WHITE, float z = 0.0f );

    void DrawCircle( glm::vec2 center, float angle, float radius,
                    Color lineColor, Color fillColor, float z = 0.0f, const int pixelPerSeg=8 );
    void DrawRing( glm::vec2 center, float radius1, float radius2, int segs,
                  Color lineColor = COLOR_NONE, Color fillColor = COLOR_WHITE, float z = 0.0f );
    void DrawGrid( float gridSize, Rect2D rect, int subDivisions, Color color );
    void Draw3DGrid( const glm::vec3& pos, const float size, const int divisions );
    // 2D Drawing textured
    void DrawSprite( const Sprite& sprite );
    void DrawSpriteBatch( const SpriteBatch& batch );

    void DrawImage( const glm::vec2 center, const float width, const float height,
                   const std::string texName, const float z = 0.0f, const Color color = COLOR_WHITE);
    void DrawTextureArray( const Rect2D rect, const Rect2D texRect,
                          const int width, const int height,
                     const GLbyte* data, const float z = 0.0f, const Color color = COLOR_WHITE);

    void DrawTexture(
		const Rect2D rect,
		const Rect2D texRect,
		const GLuint tex,
		const glm::mat4& mvp,
		const float z = 0.0f,
		const Color color = COLOR_WHITE);
    void DrawCubeMap( const Rect2D rect, const CubeTexVerts& texCoords,
                     const GLuint tex, const float z = 0.0f, const Color color = COLOR_WHITE);

    // 3D Drawing functions ( drawn immediately )
    void DrawBoxOutline( glm::vec3 center, glm::vec3 boxSize, Color color );
    void DrawSphere(float radius, unsigned int rings, unsigned int sectors);

    const glm::vec3 GetCursor3DPos( const glm::vec2 cursorPos ) const;
    
	// This is stupid but needed for now
	void setRoomSize(float size) { _reflectionProbe.setSize(size); }
    // Common accessors
    const std::string GetInfo() const;
	MaterialData& getMaterials() { return _materials; }
	void refreshMaterials();

private:
    // Real global dependencies
	std::shared_ptr<Options> _options;
	std::shared_ptr<Camera> _camera;
	std::shared_ptr<LightSystem3D> _lighting;
	std::shared_ptr<ShaderManager> _shaders;
    
	GBuffer _gBuffer;
	ReflectionProbe _reflectionProbe;

	std::map<GLuint, std::shared_ptr<VertBuffer>> _vertBuffers;
	std::vector<GLuint> _vertArrays;

	std::vector<std::shared_ptr<Mesh>> _renderQueueDeferred;
	GLuint _vaoNormalVerts, _vboNormalVerts;	// TODO: Remove vbo from here

	std::vector<DrawPackage> _deferredQueue;
	std::vector<InstancedDrawPackage> _deferredInstanceQueue;
	std::vector<DrawPackage> _forwardQueue;
	std::vector<LightInstance> _lightsQueue;

	MaterialData _materials;
	MaterialTexture _materialTexture;

	// Cube rendering
	GLuint _cube_vao, _cube_vbo;
	std::shared_ptr<VertBuffer> _cubeVB;
	VertexData<CubeInstance> _cubeData;

	// Billboard sprites and impostor sphere rendering
	GLuint _sphere_vao;
	std::shared_ptr<VertBuffer> _sphereVB;
	VertexData<SphereVertexData> _sphereData;

	GLuint _sprite_vao;
	std::shared_ptr<VertBuffer> _spriteVB;
	VertexData<TexturedVertexData> _spriteData;
	
	// Colored verts - forward rendered
	GLuint _colorVert_vao;
	std::shared_ptr<VertBuffer> _colorVertVB;
	VertexData<TexturedVertexData> _colorVertData;

    GLuint ao_fbo, ao_texture;  // The ambient occlusion FBO and texture

    GLuint final_fbo, final_texture;    // Final lit image FBO and texture
    
    GLuint light_fbo, light_textures[5];    // Light FBO and texures
    std::shared_ptr<Shader> f_shaderDefault_vColor, d_shaderDefault_uColor;
    //std::shared_ptr<Shader> d_shaderLight;
    std::shared_ptr<Shader> d_shaderLightShadow;
    std::shared_ptr<Shader> d_shaderMesh;
    std::shared_ptr<Shader> d_shaderInstance;
    std::shared_ptr<Shader> d_shaderCube;
    std::shared_ptr<Shader> _shaderDeferredSprite, _shaderDeferredSphere, d_shaderCloud;
	std::shared_ptr<Shader> _shaderDeferredMesh;
    std::shared_ptr<Shader> d_shaderBlurH;
    std::shared_ptr<Shader> d_shaderBlurV;
    // Post processing
    std::shared_ptr<Shader> d_shaderSunPP;
    std::shared_ptr<Shader> d_shaderSky;
    std::shared_ptr<Shader> d_shaderPost;   // Final post-process shader
    std::shared_ptr<Shader> d_shaderDepth;
    std::shared_ptr<Shader> d_shaderLensFlare;
    // Light rays
    std::shared_ptr<Shader> d_shaderLightRays;
    float lr_exposure, lr_decay, lr_density, lr_weight;
    std::shared_ptr<Shader> d_shaderSSAO;
    std::shared_ptr<Shader> d_shaderEdgeSobel;
    std::shared_ptr<Shader> d_shaderEdgeFreiChen;
    std::shared_ptr<Shader> d_shaderShadowMapMesh;
    std::shared_ptr<Shader> d_shaderShadowMapObject;
    std::shared_ptr<Shader> d_shaderShadowMapSphere;
    std::shared_ptr<Shader> d_shaderShadowMapCube;

    std::shared_ptr<Shader> d_shaderFXAA;

    //  ----    FORWARD RENDERING   ----    //
    std::shared_ptr<Shader> f_shaderDefault;
    std::shared_ptr<Shader> f_shaderUI_tex, f_shaderUI_color, f_shaderUI_vColor;
    std::shared_ptr<Shader> f_shaderUI_cubeMap;
    std::shared_ptr<Shader> f_shaderSprite;

    //  ----    //
    RenderMode renderMode;      // For debugging mainly
    
    // Matrices for 2D and 3D rendering, store them and only calculate once at beginning of frame
    glm::mat4 mvp3D, mvp2D;
    
    // General purpose polygon buffers TODO: MAKE INTO PROPER VERTBUFFER
    //ColorVertexData vertBufferColor[MAX_BUFFERED_VERTS*2];
    //GLuint colorVerts_vao, colorVerts_vbo;
    //unsigned int numBufferedColorVerts;

    // 2D Square buffers and vertex array objects
    GLuint square2D_vao, square2D_vbo;
    // Line rendering buffers
    GLuint lines_vbo, lines_vao;
    ColorVertexData lineBuffer2D[MAX_BUFFERED_LINES*2];
    ColorVertexData lineBuffer3D[MAX_BUFFERED_LINES*2];
    unsigned int numBuffered2DLines, numBuffered3DLines;
    
    // Polygon rendering
    GLuint vertex_vbo, vertex_vao, vertex_ibo;

	// Triangle mesh rendering
	GLuint _mesh_vao;
	std::shared_ptr<VertBuffer> _meshVB;

    // UI textured rendering buffers
	GLuint ui_vbo, ui_vao;
    // Object instancing buffers
    GLuint instancing_vao, instancing_vbo;

    float fxaaOffset;
    
    bool ssao_depthOnly;
    // SSAO noise texture
    Texture* tex_ssao_noise;
    // Lens dirt texture for lens flare effect
    Texture* tex_lens_dirt;

    GLuint shadow_fbo, shadow_cube_fbo;
    GLuint shadow_texture, shadow_cubeMap;
    
    
    void SetDefaults();
    void SetScreenMode();
    
    void SetupRenderBuffers();
    void CleanupRenderBuffers();
    
    void SetupShaders();
    
    void SetupGeometry();
    void CleanupGeometry();

	void updateReflections();
	void flushDeferredQueue(
		const glm::mat4& view,
		const glm::mat4& projection,
		const glm::vec3& position);
	void flushForwardQueue(
		const glm::mat4& view,
		const glm::mat4& projection,
		const glm::vec3& position);

	// helpers
	void setBlendMode(BlendMode mode);
	void setDepthMode(DepthMode mode);

	void prepareFinalFBO(const int width, const int height);
};

#endif /* RENDERER_GL_PROG_H */
