#ifndef RENDERER_GL_PROG_H
#define RENDERER_GL_PROG_H

#include "Renderer.h"
#include "TextureCache.h"
#include "TextureAtlasCache.h"
#include "ShaderCache.h"
#include "FrameCache.h"
#include "TextAtlasCache.h"
#include "LinearAllocator.h"
#include "TextureLoader.h"

#include "VertBuffer.h"
#include "VertexData.h"
#include "GBuffer.h"
#include "DebugCubeBuffer.h"
#include "ArrayBufferVertex.h"
#include "ArrayBufferInstance.h"
#include "ReflectionProbe.h"
#include "MaterialTexture.h"
#include "Material.h"
#include "LightSystem3D.h"
#include <vector>
#include <map>
#include <memory>

class Allocator;
class Options;
class StatTracker;
class Camera;
class Shader;
class Sprite;
class Mesh;
class ThreadPool;

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

constexpr size_t MAX_SPRITE_VERTS_PER_TEXTURE = 8 * 1024;
constexpr size_t MAX_UI_TEXTURES = 16;
constexpr size_t MAX_UI_VERTS_PER_TEXTURE = 8 * 1024;
constexpr size_t MAX_TEXT_VERTS_PER_TEXTURE = 128 * 1024;

class RendererGLProg : public Renderer
{
public:
    RendererGLProg(Allocator& allocator, Options& options, Camera& camera, ThreadPool& threadPool);
    ~RendererGLProg();
    
    void Initialize() override;
    void Terminate() override;

    void Resize(const int width, const int height);

    void BeginDraw() override;
    void EndDraw() override;    

    TextureID getTextureID(const std::string& textureName, bool load) override;
    void getTextureIDAsync(const std::string& textureName, const std::function<void(TextureID)>& callback) override;
    const Texture2D* getTextureByID(const TextureID textureID) override;

    TextureAtlasID getTextureAtlasID(const std::string& textureAtlasName) override;
    const TextureAtlas* getTextureAtlasByID(const TextureAtlasID textureAtlasID) override;
    TextureAtlasID getTextureAtlasIDForFrame(const std::string& frameName) override;

    ShaderID getShaderID(const std::string& shaderVertexName, const std::string& shaderFragName) override;
    ShaderID getShaderID(const std::string& shaderGeometryName, const std::string& shaderVertexName, const std::string& shaderFragName) override;
    const Shader* getShaderByID(const ShaderID shaderID) override;

    TextAtlasID getTextAtlasID(const std::string& textAtlasName, const uint8_t fontHeight) override;
    TextAtlas* getTextAtlasByID(const TextAtlasID textAtlasID) override;

    TexturedVertexData* queueTexturedVerts(const uint32_t numVerts, const TextureID textureID) override;
    TexturedVertexData* queueTextVerts(const uint32_t numVerts, const TextureID textureID) override;

    glm::ivec2 getWindowSize() const override;

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
    
    void PassStencil( const GLuint fbo );
    
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
    VertBuffer* addVertBuffer(const VertexDataType type) override;
    void destroyVertBuffer(VertBuffer* buffer) override;
    BaseVertexData* createVertexData(const size_t size, const VertexDataType type) override;
    void destroyVertexData(BaseVertexData* vertexData) override;
    template <typename T> VertexData<T>* allocVertexData(const size_t size, const VertexDataType type);

	//void queueMesh(std::shared_ptr<Mesh> mesh);

	void queueDeferredBuffer(
        const VertBuffer* buffer,
        const void* data,
        const unsigned int rangeStart,
        const unsigned int rangeEnd,
        const TextureID texture,
        const BlendMode blendMode,
        const DepthMode depthMode) override;

	void queueForwardBuffer(
        const VertBuffer* buffer,
        const void* data,
        const unsigned int rangeStart,
        const unsigned int rangeEnd,
        const TextureID texture,
        const BlendMode blendMode,
        const DepthMode depthMode) override;

	void queueDeferredInstances(
		const GLuint instanceBuffer,
		const unsigned int instanceCount,
		const VertexDataType type,
		const GLuint buffer,
		const unsigned int rangeStart,
		const unsigned int rangeEnd,
		const TextureID tex,
		const BlendMode blendMode,
		const DepthMode depthMode);

	void queueLights(
		const LightInstance* lights,
		const unsigned int lightCount);

	void flush();

    // Vertex buffer operations
	void renderVertBuffer(
		VertBuffer* buffer,
		const unsigned int rangeEnd,
		const unsigned int rangeStart = 0,
		const Texture2D* tex = NULL,
		const bool render3D = true) override;

    // General triangle buffering functions
    //void BufferVerts( const ColorVertexData* verts, const int numVerts );
    //void BufferVerts( const NormalVertexData* verts, const int numVerts );
    void BufferSpheres( const SphereVertexData* spheres, const int numSpheres ) override;
    void BufferFireballs( const SphereVertexData* spheres, const int numFireballs ) override;

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

    void bufferCubes(const CubeInstance* cubes, const size_t count) override;
    void renderCubes(const glm::mat4& view, const glm::mat4& projection, Shader& shader) override;

    void bufferColorCubes(const CubeInstanceColor* cubes, const size_t count) override;
    void renderColorCubes(const glm::mat4& view, const glm::mat4& projection, Shader& shader) override;

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

    const glm::vec3 GetCursor3DPos( const glm::vec2 cursorPos ) const;
    
	// This is stupid but needed for now
	void setRoomSize(float size) { m_reflectionProbe.setSize(size); }
    // Common accessors
    const std::string GetInfo() const;
	MaterialData& getMaterials() { return _materials; }
	void refreshMaterials();

private:
    Allocator& m_allocator;
    Options& m_options;
	Camera& m_camera;
    ThreadPool& m_threadPool;
    LinearAllocator m_frameAllocator;

    TextureLoader m_textureLoader;
    TextureCache m_textureCache;
    TextureAtlasCache m_atlasCache;
    ShaderCache m_shaderCache;
    FrameCache m_frameCache;
    TextAtlasCache m_textAtlasCache;

    LightSystem3D m_lightSystem3D;
    
	GBuffer m_gBuffer;
	ReflectionProbe m_reflectionProbe;

	std::map<GLuint, VertBuffer*> _vertBuffers;
	std::vector<GLuint> _vertArrays;

	std::vector<DrawPackage> _deferredQueue;
	std::vector<InstancedDrawPackage> _deferredInstanceQueue;
	std::vector<DrawPackage> _forwardQueue;
	std::vector<LightInstance> _lightsQueue;

	MaterialData _materials;
	MaterialTexture _materialTexture;
    Material _material;

	// Cube rendering
    ArrayBufferInstance<CubeInstance> m_cubeBuffer;
    ArrayBufferInstance<CubeInstanceColor> m_colorCubeBuffer;
    ArrayBufferVertex<SphereVertexData> m_sphereBuffer;
    ArrayBufferVertex<SphereVertexData> m_fireBallBuffer;
    //DebugCubeBuffer m_debugCube;

	// Billboard sprites
	GLuint m_sprite_vao, m_sprite_vbo;

    GLuint final_fbo, final_texture;    // Final lit image FBO and texture
    
    GLuint light_fbo, light_textures[5];    // Light FBO and texures
    Shader* f_shaderDefault_vColor;
    Shader* d_shaderDefault_uColor;
    Shader* d_shaderLightShadow;
    Shader* d_shaderMesh;
    Shader* d_shaderInstance;
    Shader* d_shaderCubeSimple;
    Shader* d_shaderCubeFancy; // does surface displacement
    Shader* d_shaderCubeColor;
    Shader* f_shaderDebugCube;
    Shader* _shaderDeferredSprite;
    Shader* _shaderDeferredSphere;
    Shader* _shaderForwardFireball;
    Shader* d_shaderCloud;
	Shader* _shaderDeferredMesh;
    Shader* d_shaderBlurH;
    Shader* d_shaderBlurV;
    // Post processing
    Shader* f_shaderLensFlare;
    Shader* d_shaderSky;
    Shader* d_shaderPost;   // Final post-process shader
    Shader* d_shaderDepth;
    Shader* d_shaderLensFlare;
    // Light rays
    Shader* d_shaderLightRays;
    float lr_exposure, lr_decay, lr_density, lr_weight;
    Shader* d_shaderEdgeSobel;
    Shader* d_shaderEdgeFreiChen;
    Shader* d_shaderShadowMapMesh;
    Shader* d_shaderShadowMapObject;
    Shader* d_shaderShadowMapSphere;
    Shader* d_shaderShadowMapCube;
    // Light bloom
    Shader* f_shaderBloomBrightPass;
    Shader* f_shaderBlurGaussian;

    //  ----    FORWARD RENDERING   ----    //
    Shader* f_shaderDefault;
    Shader* f_shaderUI_tex;
    Shader* f_shaderUI_text;
    Shader* f_shaderUI_color;
    Shader* f_shaderUI_vColor;
    Shader* f_shaderUI_cubeMap;
    Shader* f_shaderSprite;

    // Lighting shaders
    Shader* m_lightShaderEmissive;
    Shader* m_lightShaderBasic;
    Shader* m_lightShaderBadass;
    Shader* m_lightShaderDisney;

    //  ----    //
    RenderMode renderMode;      // For debugging mainly
    
    // Matrices for 2D and 3D rendering, store them and only calculate once at beginning of frame
    glm::mat4 mvp3D, mvp2D;

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
	VertBuffer* _meshVB;

    // UI textured rendering buffers
	GLuint ui_vbo, ui_vao;
    // Object instancing buffers
    GLuint instancing_vao, instancing_vbo;

    float fxaaOffset;
    
    // Lens dirt texture for lens flare effect
    TextureID tex_lens_dirt;
    TextureID tex_noise;

    GLuint shadow_fbo, shadow_cube_fbo;
    GLuint shadow_texture, shadow_cubeMap;

    struct TexturedVertsDataPackage {
        TexturedVertexData* data;
        uint32_t count;
        uint32_t capacity;
    };
    std::map<TextureID, TexturedVertsDataPackage> m_texturedVertexPackages;
    std::map<TextureID, TexturedVertsDataPackage> m_textVertexPackages;

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

    void flushTexturedVertsQueue();
    void flushTextVertsQueue();

    void renderBloom();
	// helpers
	void prepareFinalFBO(const int width, const int height);
    void renderDebug();
};

#endif /* RENDERER_GL_PROG_H */
