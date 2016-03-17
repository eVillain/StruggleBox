#ifndef RENDERER_GL_PROG_H
#define RENDERER_GL_PROG_H

#include "Renderer.h"
#include <vector>

class Shader;
class Texture;
class Sprite;
class Light3D;
class Options;
class StatTracker;
class LightSystem3D;
class Camera;

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
    RendererGLProg();
    ~RendererGLProg();
    
    void Initialize(Locator& locator);
    void ShutDown();

    void Resize(const int width, const int height);
    // Start and end frame drawing
    void BeginDraw();
    void EndDraw();
    void UpdateStats();
    
    void RenderLighting( const Color& fogColor );
    void PostProcess();
    void RenderFromTexture( const GLuint tex );
    
    void PassDOF( const GLuint fbo );
    void PassSSAO( const GLuint fbo );
    void PassStencil( const GLuint fbo );
    void PassLightRays( glm::vec3 lightWorldPos, const GLuint fbo );
    
    // Matrix functionality
    void GetUIMatrix( glm::mat4& target );
    void SetUIMatrix() {};                  // Fixed-pipeline only
    void GetGameMatrix( glm::mat4& target );
    void SetGameMatrix() {};                // Fixed-pipeline only
    glm::mat4 GetLightMVP(Light3D& light);
    glm::mat4 GetLightModel(Light3D& light);
    glm::mat4 GetLightView(Light3D& light);
    glm::mat4 GetLightProjection(Light3D& light);

    // Vertex buffer operations
    void RenderVertBuffer( VertexBuffer* vBuffer,
                          const unsigned int range, const unsigned int first=0,
                          const Texture* tex=NULL, const bool render3D=true );
    void RenderInstancedBuffer( VertexBuffer* vBuffer, const std::vector<InstanceData>& instances,
                                       const unsigned int rangeEnd, const unsigned int rangeStart=0,
                                       const Texture* tex=NULL, const bool render3D=true );
    void RenderSkyBuffer( VertexBuffer* vBuffer, glm::vec3 sunDir );
    void RenderGroundPlane( Color& horizonColor );
    // ShadowMap vert buffer rendering
    void RenderVertBufferShadows( VertexBuffer* vBuffer,
                                 const unsigned int range, const unsigned int first=0 );
    void RenderInstancedBufferShadows( VertexBuffer* vBuffer, const std::vector<InstanceData>& instances,
                               const unsigned int rangeEnd, const unsigned int rangeStart=0 );
    // General triangle buffering functions
    void BufferVerts( const ColorVertexData* verts, const int numVerts );
    void BufferVerts( const NormalVertexData* verts, const int numVerts );
    void BufferSpheres( const ColorVertexData* spheres, const int numSpheres );

    void RenderVerts();
    void RenderSpheres();

    // 2D Buffering functions
    void Buffer2DLine( glm::vec2 a, glm::vec2 b, Color aColor, Color bColor, float z = 0.0f );
    void Render2DLines();
    
    // 3D Buffering functions
    void Buffer3DLine( glm::vec3 a, glm::vec3 b, Color aColor, Color bColor );
    void Render3DLines();
    void Buffer3DCube(CubeInstance& instance);
    void Render3DCubes();
    void Render2DCube( const glm::vec2& center, const Color color, const glm::vec3 rotation);
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
    void DrawGradientY( Rect2D rect, Color topColor, Color btmColor, float z = 0.0f );
    void DrawGradientX( Rect2D rect, Color leftColor, Color rightColor, float z = 0.0f );

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

    void DrawTexture( const Rect2D rect, const Rect2D texRect,
                   const GLuint tex, const float z = 0.0f, const Color color = COLOR_WHITE);
    void DrawCubeMap( const Rect2D rect, const GLfloat* texCoords,
                     const GLuint tex, const float z = 0.0f, const Color color = COLOR_WHITE);

    // 3D Drawing functions ( drawn immediately )
    void DrawBoxOutline( glm::vec3 center, glm::vec3 boxSize, Color color );
    void DrawSphere(float radius, unsigned int rings, unsigned int sectors);

    const glm::vec3 GetCursor3DPos( const glm::vec2 cursorPos ) const;
    
    // Common accessors
    const std::string GetInfo() const;
private:
    // Real global dependencies
    Options* g_options;
    StatTracker* g_stats;
    LightSystem3D* g_lights3D;
    Camera* g_camera;
    
    std::vector<VertexBuffer*> vertexBuffers;
    //  ----    DEFERRED RENDERING  ----    //
    GLuint render_fbo;          // The frame buffer object (G-Buffer)
    GLuint diffuse_texture;     // The texture object to write our diffuse color to
    GLuint specular_texture;    // The texture object to write our specular color to
    GLuint depth_texture;       // The temporary texture object to write our depth to
    GLuint normal_texture;      // The temporary texture object to write our normals to
    GLuint ao_fbo, ao_texture;  // The ambient occlusion FBO and texture

    GLuint final_fbo, final_texture;    // Final lit image FBO and texture
    
    GLuint light_fbo, light_textures[5];    // Light FBO and texures
    Shader *d_shaderDefault_vColor, *d_shaderDefault_uColor;
    Shader *d_shaderLight;
    Shader *d_shaderLightShadow;
    Shader *d_shaderMesh;
    Shader *d_shaderInstance;
    Shader *d_shaderCube;
    Shader *d_shaderSprite, *d_shaderSphere, *d_shaderCloud;
    Shader *d_shaderBlurH;
    Shader *d_shaderBlurV;
    // Post processing
    Shader *d_shaderSunPP;
    Shader *d_shaderSky;
    Shader *d_shaderPost;   // Final post-process shader
    Shader *d_shaderDepth;
    Shader *d_shaderLensFlare;
    // Light rays
    Shader *d_shaderLightRays;
    float lr_exposure, lr_decay, lr_density, lr_weight;
    Shader *d_shaderSSAO;
    Shader *d_shaderEdgeSobel;
    Shader *d_shaderEdgeFreiChen;
    Shader *d_shaderShadowMapMesh;
    Shader *d_shaderShadowMapObject;
    Shader *d_shaderShadowMapSphere;
    Shader *d_shaderShadowMapCube;

    Shader *d_shaderFXAA;

    //  ----    FORWARD RENDERING   ----    //
    Shader *f_shaderDefault;
    Shader *f_shaderUI_tex, *f_shaderUI_color, *f_shaderUI_vColor;
    Shader *f_shaderUI_cubeMap;
    Shader *f_shaderSprite;

    //  ----    //
    RenderMode renderMode;      // For debugging mainly
    
    // Matrices for 2D and 3D rendering, store them and only calculate once at beginning of frame
    glm::mat4 mvp3D, mvp2D;
    
    // General purpose polygon buffers
    ColorVertexData vertBufferColor[MAX_BUFFERED_VERTS*2];
    NormalVertexData vertBufferNormal[MAX_BUFFERED_VERTS*2];
    GLuint colorVerts_vao, colorVerts_vbo;
    GLuint normalVerts_vao, normalVerts_vbo;

    unsigned int numBufferedColorVerts, numBufferedNormalVerts;
    // 2D Square buffers and vertex array objects
    GLuint square2D_vao, square2D_vbo;
    // Line rendering buffers
    GLuint lines_vbo, lines_vao;
    ColorVertexData lineBuffer2D[MAX_BUFFERED_LINES*2];
    ColorVertexData lineBuffer3D[MAX_BUFFERED_LINES*2];
    unsigned int numBuffered2DLines, numBuffered3DLines;
    // Colored cube rendering buffers
    GLuint cubes_vao, cubes_vbo;
    GLfloat cubeBufferPos[MAX_CUBE_INSTANCES*4];        // x,y,z,size
    GLfloat cubeBufferRot[MAX_CUBE_INSTANCES*4];        // x,y,z,w (quaternion)
    GLfloat cubeBufferColDiff[MAX_CUBE_INSTANCES*4];    // r,g,b,a
    GLfloat cubeBufferColSpec[MAX_CUBE_INSTANCES*4];    // r,g,b,a

    unsigned int numBufferedCubes;
    // Billboard sprites and impostor sphere rendering
    GLuint sprite_vao, sprite_vbo;
    ColorVertexData spriteBuffer[MAX_BUFFERED_VERTS];
    int numBufferedSprites;
    
    // Polygon rendering vertex buffers
    GLuint vertex_vbo, vertex_vao, vertex_ibo;
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
    
    void SetupFrameBuffer();
    void CleanupFrameBuffer();
    void SetupRenderBuffers();
    void CleanupRenderBuffers();
    
    void SetupShaders();
    
    void SetupGeometry();
    void CleanupGeometry();
};


#endif /* defined(NGN_RENDERER_GL_PROG_H) */
