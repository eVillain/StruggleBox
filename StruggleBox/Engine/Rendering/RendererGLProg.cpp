#include "RendererGLProg.h"
#include "Locator.h"
#include "FileUtil.h"
#include "Timer.h"
#include "StatTracker.h"

#include "VertexBuffer.h"
#include "TextureManager.h"
#include "Texture.h"
#include "ShaderManager.h"
#include "Options.h"
#include "GFXDefines.h"
#include "LightRenderer2D.h"
#include "LightSystem3D.h"
#include "Sprite.h"
#include "SpriteBatch.h"
#include "Light3D.h"
#include "Camera.h"
#include "Frustum.h"
#include "Console.h"
#include "Log.h"

#include <SDL2/SDL.h>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define SHADOW_MAP_SIZE 4096
#define SHADOW_CUBE_SIZE 512
#define LIGHT_SCALE 0.5
#define SSAO_SCALE 0.5

// Light matrices to look in all 6 directions for cubemapping
static const glm::mat4 lightViewMatrix[6] = {
    glm::lookAt(glm::vec3(0.0f,0.0f,0.0f), glm::vec3( 1.0f, 0.0f, 0.0f), glm::vec3( 0.0f,-1.0f, 0.0f)),  // +x
    glm::lookAt(glm::vec3(0.0f,0.0f,0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3( 0.0f,-1.0f, 0.0f)),  // -x
    glm::lookAt(glm::vec3(0.0f,0.0f,0.0f), glm::vec3( 0.0f, 1.0f, 0.0f), glm::vec3( 0.0f, 0.0f, 1.0f)),  // +y
    glm::lookAt(glm::vec3(0.0f,0.0f,0.0f), glm::vec3( 0.0f,-1.0f, 0.0f), glm::vec3( 0.0f, 0.0f,-1.0f)),  // -y
    glm::lookAt(glm::vec3(0.0f,0.0f,0.0f), glm::vec3( 0.0f, 0.0f, 1.0f), glm::vec3( 0.0f,-1.0f, 0.0f)),  // +z
    glm::lookAt(glm::vec3(0.0f,0.0f,0.0f), glm::vec3( 0.0f, 0.0f,-1.0f), glm::vec3( 0.0f,-1.0f, 0.0f))   // -z
};


RendererGLProg::RendererGLProg()
{
    initialized = false;
    shouldClose = false;
    lightRenderer = NULL;
    ssao_depthOnly = false;
    renderMode = RM_Final_Image;
    vertex_vbo = -1;
    vertex_vao = -1;
    lines_vbo = -1;
    lines_vao = -1;
    numBuffered2DLines = 0;
    numBuffered3DLines = 0;
    numBufferedColorVerts = 0;
    numBufferedNormalVerts = 0;

    Console::AddVar((int&)renderMode, "renderMode");
}

RendererGLProg::~RendererGLProg()
{
    if (initialized) {
        ShutDown();
    }
}

void RendererGLProg::Initialize(Locator& locator)
{
    g_options = locator.Get<Options>();
    g_stats = locator.Get<StatTracker>();
    g_camera = locator.Get<Camera>();
    g_lights3D = locator.Get<LightSystem3D>();
    
    SetDefaults();
    
    // Reset previous frame timer and counter to zero
    r_frameStartTime = 0.0;
    r_frameCounterTime = 0.0;
    r_frameCounter = 0;
    SetScreenMode();    // Set up a window to render into
    
    if ( initialized ) {
        SetupFrameBuffer();
        SetupShaders();     // Setup all our shaders and render buffers
        SetupRenderBuffers();
        SetupGeometry();
    }
    if ( initialized ) {
        lightRenderer = new LightRenderer2D( this );
    }
}

void RendererGLProg::ShutDown()
{
    initialized = false;
    shouldClose = true;
    
    if ( lightRenderer ) {
        delete lightRenderer;
        lightRenderer = NULL;
    }
    
    CleanupFrameBuffer();
    CleanupRenderBuffers();
    // Clean up loaded shaders
    ShaderManager::ClearShaders();
    // Clean up geometry buffers
    CleanupGeometry();
    
    printf("[RendererGLProg] Clean shutdown complete\n");
}

void RendererGLProg::SetDefaults()
{
    windowWidth = DEFAULT_SCREEN_WIDTH;
    windowHeight = DEFAULT_SCREEN_HEIGHT;
    // Variables for the camera
    r_camX = 0.0;
    r_camY = 0.0;
    r_camZoom = 128.0;
    r_camTX = 0.0;
    r_camTY = 0.0;
    r_camTZoom = 128.0;
    
    lr_exposure = 0.005f;
    lr_decay = 1.0f;
    lr_density = 1.15f;
    lr_weight = 3.95f;
    if (g_options) {
        windowWidth = g_options->GetOptionDataPtr<int>("r_resolutionX");
        windowHeight = g_options->GetOptionDataPtr<int>("r_resolutionY");
    }
}

void RendererGLProg::SetScreenMode()
{
    ShaderManager::ClearShaders();                              // Clear old shaders

    const GLubyte * version = glGetString(GL_VERSION);
    printf("[RendererGLProg] OpenGL Version:%s\n", version);
    
    if(!GLEW_EXT_geometry_shader4) { printf("[RendererGLProg] No support for geometry shaders found\n"); }
    else { printf("[RendererGLProg] Geometry shaders are supported\n"); }

    //Use Vsync
    if( SDL_GL_SetSwapInterval( 1 ) < 0 )
    {
        Log::Warn( "[Renderer] Unable to set VSync! SDL Error: %s", SDL_GetError() );
    }
    
    printf("[RendererGLProg] started with programmable pipeline\n");
    initialized = true;
    
    GLint dims[2];
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, &dims[0]);
    printf("[RendererGLProg] Maximum render dimensions: %i, %i\n", dims[0],dims[1]);
}

//========================================================================
// ResizeWindow() - GLFW window resize callback function
//========================================================================

void RendererGLProg::Resize(const int width, const int height)
{
    int newY = width > 0 ? height : 1;   // Prevent division by zero in aspect calc.
    windowWidth = width;
    windowHeight = newY;
    
    bool updateRes = true;
//    double hw = windowWidth*0.5;
//    double hh = windowHeight*0.5;
    
    if ( updateRes ) {
        g_options->GetOptionDataPtr<int>("r_resolutionX") = windowWidth;
        g_options->GetOptionDataPtr<int>("r_resolutionY") = windowHeight;
//        glLineWidth(1.0f);
    } else {
//        GLfloat scaleX = (float)windowWidth/g_options->GetOptionDataPtr<int>("r_resolutionX");
//        GLfloat scaleY = (float)windowHeight/g_options->GetOptionDataPtr<int>("r_resolutionY");
//        GLfloat scale = fmin(scaleX, scaleY);
//        hw = windowWidth*(0.5/scale);
//        hh = windowHeight*(0.5/scale);
//        glLineWidth(scale);
    }

    // Refresh frambefuffers to new size
    GLuint old_rfbo = render_fbo;
    GLuint old_finalfbo = final_fbo;
    GLuint old_shfbo = shadow_fbo;
    GLuint old_shtex = shadow_texture;
    
    SetupRenderBuffers();
    glDeleteFramebuffers(1, &old_rfbo);
    glDeleteFramebuffers(1, &old_finalfbo);
    glDeleteFramebuffers(1, &old_shfbo);
    glDeleteRenderbuffers(1, &old_shtex);
}

// Matrix functionality
void RendererGLProg::GetUIMatrix( glm::mat4& target ) {
    // 2D projection with origin (0,0) as center of window
    GLfloat hw = windowWidth*0.5f;
    GLfloat hh = windowHeight*0.5f;
    target = glm::ortho<GLfloat>(-hw, hw, -hh, hh, ORTHO_NEARDEPTH, ORTHO_FARDEPTH);
}
void RendererGLProg::GetGameMatrix( glm::mat4 &target ) {
    GLfloat aspectRatio = (windowWidth > windowHeight) ?
    GLfloat(windowWidth)/GLfloat(windowHeight) : GLfloat(windowHeight)/GLfloat(windowWidth);
    target = glm::perspective(g_camera->fieldOfView, aspectRatio, g_camera->nearDepth, g_camera->farDepth);
    target = glm::rotate(target, -g_camera->rotation.x, glm::vec3(1.0, 0.0, 0.0));
    target = glm::rotate(target, -g_camera->rotation.y, glm::vec3(0.0, 1.0, 0.0));
    target = glm::rotate(target, -g_camera->rotation.z, glm::vec3(0.0, 0.0, 1.0));
    target = glm::translate(target, glm::vec3(-g_camera->position.x, -g_camera->position.y, -g_camera->position.z));
}
/*  --------------------    *
 *      SHADER STUFF        *
 *  --------------------    */
void RendererGLProg::SetupShaders() {
    // Used to draw lines and debugging
    d_shaderDefault_vColor = ShaderManager::LoadFromFile("d_default_vColor.vsh", "d_default.fsh");
    d_shaderDefault_uColor = ShaderManager::LoadFromFile("d_default_uColor.vsh", "d_default.fsh");
    // 2D rendering shaders
    f_shaderUI_tex = ShaderManager::LoadFromFile("f_ui_tex.vsh", "f_ui_tex.fsh");
    f_shaderUI_color = ShaderManager::LoadFromFile("f_ui_color.vsh", "f_ui_color.fsh");
    f_shaderUI_vColor = ShaderManager::LoadFromFile("f_ui_vcolor.vsh", "f_ui_vcolor.fsh");
    f_shaderUI_cubeMap = ShaderManager::LoadFromFile("f_ui_cubeMap.vsh", "f_ui_cubeMap.fsh");
    // Sunshine lens flares and halo
    d_shaderSunPP = ShaderManager::LoadFromFile("d_sunPostProcess.vsh", "d_sunPostProcess.fsh");
    d_shaderSunPP->Begin();
    d_shaderSunPP->setUniform1iv("LowBlurredSunTexture", 0);
    d_shaderSunPP->setUniform1iv("HighBlurredSunTexture", 1);
    d_shaderSunPP->setUniform1iv("DirtTexture", 2);
    d_shaderSunPP->setUniform1fv("Dispersal", 0.1875f);
    d_shaderSunPP->setUniform1fv("HaloWidth", 0.45f);
    d_shaderSunPP->setUniform1fv("Intensity", 2.25f);
    d_shaderSunPP->setUniform3fv("Distortion", 0.94f, 0.97f, 1.00f);
    d_shaderSunPP->End();
    // Vertical and horizontal blurs
	d_shaderBlurH = ShaderManager::LoadFromFile("d_blur.vsh", "d_blur_horizontal.fsh");
	d_shaderBlurV = ShaderManager::LoadFromFile("d_blur.vsh", "d_blur_vertical.fsh");
    // Deferred light pass, includes fog and noise
    d_shaderLight = ShaderManager::LoadFromFile("d_light_pass.vsh", "d_light_pass.fsh");
    d_shaderLightShadow = ShaderManager::LoadFromFile("d_light_pass_shadow.vsh", "d_light_pass_shadow.fsh");

    // Renders the cubemesh
    d_shaderMesh = ShaderManager::LoadFromFile("d_mesh_color.vsh", "d_mesh_color.fsh");
    // Renders dynamic objects in batched instances
    d_shaderInstance = ShaderManager::LoadFromFile("d_object_instance.vsh", "d_object_instance.fsh");
    // Renders single colored cubes in batched instances
    d_shaderCube = ShaderManager::LoadFromFile("d_cube_instance.vsh", "d_cube_instance.fsh");
    // Renders instanced spheres
    d_shaderSphere = ShaderManager::LoadFromFile( "d_impostor_sphere.gsh", "d_impostor_sphere.vsh", "d_impostor_sphere.fsh");
    d_shaderSprite = ShaderManager::LoadFromFile( "d_impostor_billboard.gsh", "d_impostor_billboard.vsh", "d_impostor_billboard.fsh");
    d_shaderCloud = ShaderManager::LoadFromFile( "d_impostor_cloud.gsh", "d_impostor_cloud.vsh", "d_impostor_cloud.fsh");
    f_shaderSprite = ShaderManager::LoadFromFile( "f_impostor_billboard.gsh", "f_impostor_billboard.vsh", "f_impostor_billboard.fsh");
    float Kr = 0.0030f;
	float Km = 0.0015f;
	float ESun = 16.0f;
	float g = -0.75f;
	float InnerRadius = 10.0f;
	float OuterRadius = 10.25f;
	float Scale = 1.0f / (OuterRadius - InnerRadius);
	float ScaleDepth = 0.25f;
	float ScaleOverScaleDepth = Scale / ScaleDepth;
    // Sky shader
    d_shaderSky = ShaderManager::LoadFromFile("d_sky_dome.vsh", "d_sky_dome.fsh");
    d_shaderSky->Begin();
    d_shaderSky->setUniform3fv("v3CameraPos", 0.0f, InnerRadius, 0.0f);
    d_shaderSky->setUniform3fv("v3InvWavelength", 1.0f / powf(0.650f, 4.0f), 1.0f / powf(0.570f, 4.0f), 1.0f / powf(0.475f, 4.0f));
    d_shaderSky->setUniform1fv("fCameraHeight", InnerRadius);
    d_shaderSky->setUniform1fv("fCameraHeight2", InnerRadius * InnerRadius);
    d_shaderSky->setUniform1fv("fInnerRadius", InnerRadius);
    d_shaderSky->setUniform1fv("fInnerRadius2", InnerRadius * InnerRadius);
    d_shaderSky->setUniform1fv("fOuterRadius", OuterRadius);
    d_shaderSky->setUniform1fv("fOuterRadius2", OuterRadius * OuterRadius);
    d_shaderSky->setUniform1fv("fKrESun", Kr * ESun);
    d_shaderSky->setUniform1fv("fKmESun", Km * ESun);
    d_shaderSky->setUniform1fv("fKr4PI", Kr * 4.0f * (float)M_PI);
    d_shaderSky->setUniform1fv("fKm4PI", Km * 4.0f * (float)M_PI);
    d_shaderSky->setUniform1fv("fScale", Scale);
    d_shaderSky->setUniform1fv("fScaleDepth", ScaleDepth);
    d_shaderSky->setUniform1fv("fScaleOverScaleDepth", ScaleOverScaleDepth);
    d_shaderSky->setUniform1fv("g", g);
    d_shaderSky->setUniform1fv("g2", g * g);
    d_shaderSky->setUniform1iv("Samples", 4);
    d_shaderSky->End();
    
    // Post-processing shaders
    d_shaderPost = ShaderManager::LoadFromFile("d_post_process.vsh", "d_post_process.fsh");
    d_shaderDepth = ShaderManager::LoadFromFile("d_depth.vsh", "d_depth.fsh");
    d_shaderLensFlare = ShaderManager::LoadFromFile("d_lensFlare.vsh", "d_lensFlare.fsh");
    d_shaderLightRays = ShaderManager::LoadFromFile("d_light_rays.vsh", "d_light_rays.fsh");
    if ( ssao_depthOnly ) {
        d_shaderSSAO = ShaderManager::LoadFromFile("d_ssao_depth.vsh", "d_ssao_depth.fsh");
    } else {
        d_shaderSSAO = ShaderManager::LoadFromFile("d_ssao_normal.vsh", "d_ssao_normal.fsh");
    }
    d_shaderEdgeSobel = ShaderManager::LoadFromFile("d_edge_sobel.vsh", "d_edge_sobel.fsh");
    d_shaderEdgeFreiChen = ShaderManager::LoadFromFile("d_edge_frei_chen.vsh", "d_edge_frei_chen.fsh");
    
    d_shaderShadowMapMesh = ShaderManager::LoadFromFile("d_shadowMap_mesh.vsh", "d_shadowMap.fsh");
    d_shaderShadowMapObject = ShaderManager::LoadFromFile("d_shadowMap_object.vsh", "d_shadowMap.fsh");
    d_shaderShadowMapSphere = ShaderManager::LoadFromFile("d_shadowMap_sphere.gsh", "d_shadowMap_sphere.vsh", "d_shadowMap_sphere.fsh");
    d_shaderShadowMapCube = ShaderManager::LoadFromFile("d_shadowMap_cube.vsh", "d_shadowMap.fsh");

//    d_shaderFXAA = ShaderManager::LoadFromFile("d_FXAA.vsh", "d_FXAA.fsh");
    
    // Load lens dirt texture
    tex_lens_dirt = TextureManager::Inst()->LoadTexture(FileUtil::GetPath().append("DATA/GFX/"),
                                                        "lens_dirt.png",
                                                        GL_CLAMP_TO_EDGE,
                                                        GL_NEAREST);
    // Load SSAO noise texture
    tex_ssao_noise = TextureManager::Inst()->LoadTexture(FileUtil::GetPath().append("DATA/GFX/"),
                                                         "noise.png",
                                                         GL_REPEAT,
                                                         GL_NEAREST);
}
/*  --------------------    *
 *   FRAMEBUFFER STUFF      *
 *  --------------------    */
void RendererGLProg::SetupFrameBuffer() {
    /* --- Generate our frame buffer textures --- */
    glGenTextures(1, &diffuse_texture);  // Generate rgb color render texture
    glBindTexture(GL_TEXTURE_2D, diffuse_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glGenTextures(1, &specular_texture);  // Generate rgb color render texture
    glBindTexture(GL_TEXTURE_2D, specular_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glGenTextures(1, &depth_texture);   // Generate depth/stencil map render texture
    glBindTexture(GL_TEXTURE_2D, depth_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, windowWidth, windowHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glBindTexture(GL_TEXTURE_2D, 0);
    glGenTextures(1, &normal_texture); // Generate normal map texture
    glBindTexture(GL_TEXTURE_2D, normal_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Generate main rendering frame buffer
    glGenFramebuffers(1, &render_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, render_fbo); // Bind our frame buffer
    // Attach the texture render_texture to the color buffer in our frame buffer
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, diffuse_texture, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, specular_texture, 0);
    // Attach the normal buffer to our frame buffer
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, normal_texture, 0);
    // Attach the depth buffer depth_texture to our frame buffer
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depth_texture, 0);
    
    // Set the list of draw buffers.
    // Passing GL_DEPTH_ATTACHMENT as second buffer returns an invalid enum glerror
    GLenum DrawBuffers[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, (GLenum*)DrawBuffers);
    // Check that status of our generated frame buffer
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        printf( "[RendererGLProg] Couldn't create render frame buffer, code:%i", status );
        exit(0); // Exit the application, can't render without a framebuffer
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind our new frame buffer
}
void RendererGLProg::CleanupFrameBuffer() {
    glDeleteTextures( 1, &diffuse_texture );
    glDeleteTextures( 1, &specular_texture );
    glDeleteTextures( 1, &depth_texture );
    glDeleteTextures( 1, &normal_texture );
    glDeleteFramebuffers(1, &render_fbo);
}
void RendererGLProg::SetupRenderBuffers(void) {
    // Generate light textures
    glGenTextures(5, light_textures);
    glBindTexture(GL_TEXTURE_2D, light_textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth*LIGHT_SCALE, windowHeight*LIGHT_SCALE,
                 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, light_textures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth*LIGHT_SCALE, windowHeight*LIGHT_SCALE,
                 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_2D, light_textures[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth*LIGHT_SCALE, windowHeight*LIGHT_SCALE,
                 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, light_textures[3]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth*LIGHT_SCALE, windowHeight*LIGHT_SCALE,
                 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_2D, light_textures[4]);    // Light depth/stencil buffer
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, windowWidth*LIGHT_SCALE, windowHeight*LIGHT_SCALE,
                 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Generate Light frame buffer
    glGenFramebuffers(1, &light_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, light_fbo); // Bind our frame buffer
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, light_textures[0], 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, light_textures[4], 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // Generate shadow texture for directional and spot lights
    glGenTextures(1, &shadow_texture);
    glBindTexture(GL_TEXTURE_2D, shadow_texture);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
    float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
    glBindTexture(GL_TEXTURE_2D, 0);
    // Generate shadow cubemap for point lights
    glGenTextures(1, &shadow_cubeMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, shadow_cubeMap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_DEPTH_COMPONENT32F, SHADOW_CUBE_SIZE, SHADOW_CUBE_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_DEPTH_COMPONENT32F, SHADOW_CUBE_SIZE, SHADOW_CUBE_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_DEPTH_COMPONENT32F, SHADOW_CUBE_SIZE, SHADOW_CUBE_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_DEPTH_COMPONENT32F, SHADOW_CUBE_SIZE, SHADOW_CUBE_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_DEPTH_COMPONENT32F, SHADOW_CUBE_SIZE, SHADOW_CUBE_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_DEPTH_COMPONENT32F, SHADOW_CUBE_SIZE, SHADOW_CUBE_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Create a shadow framebuffer object
    glGenFramebuffers(1, &shadow_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
    // Instruct openGL that we won't bind a color texture with the currently binded FBO
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    // attach the texture to FBO depth attachment point
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_texture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // Create a shadow cubemap framebuffer object
    glGenFramebuffers(1, &shadow_cube_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_cube_fbo);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_cubeMap, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    glGenTextures(1, &final_texture);    // Generate final lit render texture
    glBindTexture(GL_TEXTURE_2D, final_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    // Generate final lit image frame buffer
    glGenFramebuffers(1, &final_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, final_fbo); // Bind our frame buffer
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, final_texture, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depth_texture, 0);  // Attach previous depth/stencil to it
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    glGenTextures(1, &ao_texture);    // Generate ambient occlusion texture
    glBindTexture(GL_TEXTURE_2D, ao_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, windowWidth*SSAO_SCALE, windowHeight*SSAO_SCALE, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    // Generate ambient occlusion image frame buffer
    glGenFramebuffers(1, &ao_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, ao_fbo); // Bind our frame buffer
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, ao_texture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RendererGLProg::CleanupRenderBuffers() {
    glDeleteTextures( 1, &final_texture );
    glDeleteFramebuffers(1, &final_fbo);
    glDeleteTextures( 1, &ao_texture );
    glDeleteFramebuffers(1, &ao_fbo);
    glDeleteTextures(1, &shadow_texture);
    glDeleteFramebuffers(1, &shadow_fbo);
    glDeleteTextures(1, &shadow_cubeMap);
    glDeleteFramebuffers(1, &shadow_cube_fbo);
    glDeleteTextures( 5, light_textures );
    glDeleteFramebuffers(1, &light_fbo);
    TextureManager::Inst()->UnloadTexture( tex_lens_dirt );
    TextureManager::Inst()->UnloadTexture( tex_ssao_noise );
}
/*  --------------------    *
 *      GEOMETRY STUFF      *
 *  --------------------    */
void RendererGLProg::SetupGeometry() {
    // --------- SIMPLE VERTEX BUFFER  ----------- //
    glGenVertexArrays(1, &vertex_vao);
    glGenBuffers(1, &vertex_vbo);
    glBindVertexArray(vertex_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*4, 0);
    glEnableVertexAttribArray(0);
    glGenBuffers(1, &vertex_ibo);
    glBindVertexArray(0);
	// --------- UI TEXTURED VERTEX BUFFER  ----------- //
	glGenVertexArrays(1, &ui_vao);
	glGenBuffers(1, &ui_vbo);
	glBindVertexArray(ui_vao);
	glBindBuffer(GL_ARRAY_BUFFER, ui_vbo);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*4, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)* 2, (GLvoid*)(sizeof(GLfloat)*4));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
    // --------- COLOR/NORMAL VERTEX BUFFERS  ----------- //
    glGenVertexArrays(1, &normalVerts_vao);
    glBindVertexArray(normalVerts_vao);
    glGenBuffers(1, &normalVerts_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, normalVerts_vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_BUFFERED_VERTS * sizeof(NormalVertexData), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(NormalVertexData), 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(NormalVertexData), (GLvoid*)(4*sizeof(GLfloat)));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(NormalVertexData), (GLvoid*)(8*sizeof(GLfloat)));
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(NormalVertexData), (GLvoid*)(9*sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glBindVertexArray(0);
    glGenVertexArrays(1, &colorVerts_vao);
    glBindVertexArray(colorVerts_vao);
    glGenBuffers(1, &colorVerts_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, colorVerts_vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_BUFFERED_VERTS * sizeof(ColorVertexData), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (GLvoid*)(4*sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    // ----------   SPHERE BUFFER   --------- //
    glGenVertexArrays(1, &sprite_vao);
    glBindVertexArray(sprite_vao);
    glGenBuffers(1, &sprite_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, sprite_vbo);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (GLvoid*)(4*sizeof(GLfloat)));
    glBufferData(GL_ARRAY_BUFFER, sizeof(spriteBuffer), NULL, GL_STATIC_DRAW);
    glBindVertexArray(0);
    // ----------   2D SQUARE BUFFER   --------- //
    glGenVertexArrays(1, &square2D_vao);
    glGenBuffers(1, &square2D_vbo);
    glBindVertexArray(square2D_vao);
    glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square2D_coords)+sizeof(square2D_texCoords), square2D_coords, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(square2D_coords), sizeof(square2D_texCoords), square2D_texCoords);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(sizeof(square2D_coords)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    // --------- COLORED LINE BUFFERS ----------- //
    glGenVertexArrays(1, &lines_vao);
    glGenBuffers(1, &lines_vbo);
    glBindVertexArray(lines_vao);
    glBindBuffer(GL_ARRAY_BUFFER, lines_vbo);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(ColorVertexData), 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ColorVertexData), (GLvoid*)(4*sizeof(GLfloat)));
    glBufferData(GL_ARRAY_BUFFER, sizeof(lineBuffer2D), NULL, GL_STATIC_DRAW);
    glBindVertexArray(0);
    // --------- COLORED CUBE BUFFERS -----------
    glGenVertexArrays(1, &cubes_vao);
    glGenBuffers(1, &cubes_vbo);
    glBindVertexArray(cubes_vao);
    glBindBuffer(GL_ARRAY_BUFFER, cubes_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(cube_vertices) +
                 sizeof(cube_normals)  +
                 sizeof(cubeBufferPos) +
                 sizeof(cubeBufferRot) +
                 sizeof(cubeBufferColDiff) +
                 sizeof(cubeBufferColSpec),
                 NULL,
                 GL_STATIC_DRAW);
    
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cube_vertices), cube_vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(cube_vertices), sizeof(cube_normals), cube_normals);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0,
                          (GLvoid *)sizeof(cube_vertices));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0,
                          (GLvoid *)(sizeof(cube_vertices)+sizeof(cube_normals)));
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0,
                          (GLvoid *)(sizeof(cube_vertices)+sizeof(cube_normals)+sizeof(cubeBufferPos)));
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0,
                          (GLvoid *)(sizeof(cube_vertices)+sizeof(cube_normals)+sizeof(cubeBufferPos)+sizeof(cubeBufferRot)));
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 0,
                          (GLvoid *)(sizeof(cube_vertices)+sizeof(cube_normals)+sizeof(cubeBufferPos)+sizeof(cubeBufferRot)+sizeof(cubeBufferColDiff)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    glVertexAttribDivisorARB(1, 0);
    glVertexAttribDivisorARB(2, 1);
    glVertexAttribDivisorARB(3, 1);
    glVertexAttribDivisorARB(4, 1);
    glVertexAttribDivisorARB(5, 1);
    glBindVertexArray(0);
    // -------- INSTANCING BUFFERS ----------
    glGenVertexArrays(1, &instancing_vao);
    glGenBuffers(1, &instancing_vbo);
    glBindVertexArray(instancing_vao);
    glBindBuffer(GL_ARRAY_BUFFER, instancing_vbo);
//    glBufferData(GL_ARRAY_BUFFER, MAX_BUFFERED_VERTS * sizeof(NormalVertexData), NULL, GL_DYNAMIC_DRAW);
    // Vertices with color and normal data
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(NormalVertexData), 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(NormalVertexData), (GLvoid*)(4*sizeof(GLfloat)));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(NormalVertexData), (GLvoid*)(8*sizeof(GLfloat)));
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(NormalVertexData), (GLvoid*)(9*sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    // Instance-specific data (position, rotation, scale)
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 10*sizeof(GLfloat), 0);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 10*sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
    glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, 10*sizeof(GLfloat), (GLvoid*)(7*sizeof(GLfloat)));
    glEnableVertexAttribArray(4);
    glEnableVertexAttribArray(5);
    glEnableVertexAttribArray(6);
    glVertexAttribDivisorARB(4, 1);
    glVertexAttribDivisorARB(5, 1);
    glVertexAttribDivisorARB(6, 1);
}
void RendererGLProg::CleanupGeometry() {
    glDeleteBuffers(1, &colorVerts_vbo);
    glDeleteVertexArrays(1, &colorVerts_vao);
    colorVerts_vbo = -1;
    colorVerts_vao = -1;
    glDeleteBuffers(1, &normalVerts_vbo);
    glDeleteVertexArrays(1, &normalVerts_vao);
    normalVerts_vbo = -1;
    normalVerts_vao = -1;
    glDeleteBuffers(1, & sprite_vbo);
    glDeleteVertexArrays(1, &sprite_vao);
    sprite_vbo = -1;
    sprite_vao = -1;
    glDeleteBuffers(1, &vertex_vbo);
    glDeleteBuffers(1, &vertex_ibo);
    glDeleteVertexArrays(1, &vertex_vao);
    vertex_vbo = -1;
    vertex_vao = -1;
    vertex_ibo = -1;
	glDeleteBuffers(1, &ui_vbo);
	glDeleteVertexArrays(1, &ui_vao);
	ui_vbo = -1;
	ui_vao = -1;
    glDeleteBuffers(1, &square2D_vbo);
    glDeleteVertexArrays(1, &square2D_vao);
    square2D_vbo = -1;
    square2D_vao = -1;
    glDeleteBuffers(1, &lines_vbo);
    glDeleteVertexArrays(1, &lines_vao);
    lines_vbo = -1;
    lines_vao = -1;
    glDeleteBuffers(1, &cubes_vbo);
    glDeleteVertexArrays(1, &cubes_vao);
    cubes_vbo = -1;
    cubes_vao = -1;
    glDeleteBuffers(1, & instancing_vbo);
    glDeleteVertexArrays(1, &instancing_vao);
    instancing_vbo = -1;
    instancing_vao = -1;
}


// Setup rendering for a new frame
void RendererGLProg::BeginDraw( void ) {
    // Save timestamp for beginning of this render cycle
    r_frameStartTime = Timer::Seconds();
    // Reset buffers
    numBuffered2DLines = 0;
    numBuffered3DLines = 0;
    numBufferedColorVerts = 0;
    numBufferedNormalVerts = 0;
    numBufferedSprites = 0;
    // Reset stats
    renderedSegs = 0;
    renderedTris = 0;
    renderedSprites = 0;
    
    // Enable back face culling
    glCullFace( GL_BACK );
    glEnable( GL_CULL_FACE );
    
    if ( g_options->GetOptionDataPtr<bool>("r_fsAA") ) {
        glEnable( GL_MULTISAMPLE_ARB );
        glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );   // Can also try GL_FASTEST as hint
        glHint( GL_POINT_SMOOTH_HINT, GL_NICEST );
        glEnable( GL_LINE_SMOOTH );
        glEnable( GL_POINT_SMOOTH );
    } else {
        glDisable( GL_MULTISAMPLE_ARB );
        glDisable( GL_LINE_SMOOTH );
        glDisable( GL_POINT_SMOOTH );
    }
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    
    glDepthFunc( GL_LEQUAL );
    glEnable( GL_DEPTH_TEST );
    glDepthMask( GL_TRUE );
    
    // Clear color and Z-buffer
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    glClearDepth( 1.0 );
    glClearStencil( Stencil_None );
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind screen buffer and clear it
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

    if ( g_options->GetOptionDataPtr<bool>("r_deferred") ) {
//        glBindFramebuffer(GL_FRAMEBUFFER, light_fbo);  // Bind and clear our light buffer for rendering
//        glClear(GL_STENCIL_BUFFER_BIT);
//        if ( g_options->GetOptionDataPtr<bool>("r_shadows") ) {
            glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);  // Bind and clear our shadow buffer for rendering
            glClear(GL_DEPTH_BUFFER_BIT);
//        }
        glBindFramebuffer(GL_FRAMEBUFFER, shadow_cube_fbo); // Bind and clear our shadow cubemap for rendering
        for (int side=0; side<6; side++) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, shadow_cubeMap, 0);
            glClear(GL_DEPTH_BUFFER_BIT);
        }
//        glBindFramebuffer(GL_FRAMEBUFFER, final_fbo);   // Bind and clear our final image buffer for rendering
//        glClear(GL_COLOR_BUFFER_BIT);
        glBindFramebuffer(GL_FRAMEBUFFER, render_fbo);  // Bind and clear our G buffer for rendering
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    
    // Grab model-view-projection matrices for shaders
    GetGameMatrix(mvp3D);
    GetUIMatrix(mvp2D);

    // Pass data to frustum for culling
    Frustum::Extract(mvp3D);
    // Pass matrices and fixed data for frame
    d_shaderMesh->Begin();
    d_shaderMesh->setUniformM4fv("MVP", mvp3D);
    d_shaderMesh->End();
    d_shaderInstance->Begin();
    d_shaderInstance->setUniformM4fv("MVP", mvp3D);
    d_shaderInstance->End();
    // Build matrices for billboards/impostors
    glm::mat4 rotMatrix  = glm::mat4();
    rotMatrix = glm::rotate(rotMatrix, -g_camera->rotation.x, glm::vec3(1.0, 0.0, 0.0));
    rotMatrix = glm::rotate(rotMatrix, -g_camera->rotation.y, glm::vec3(0.0, 1.0, 0.0));
    rotMatrix = glm::rotate(rotMatrix, -g_camera->rotation.z, glm::vec3(0.0, 0.0, 1.0));
    glm::mat4 view = glm::mat4();
    view = glm::translate(rotMatrix, glm::vec3(-g_camera->position.x, -g_camera->position.y, -g_camera->position.z));
    glm::mat4 projection;
    GLfloat aspectRatio = (windowWidth > windowHeight) ?
    GLfloat(windowWidth)/GLfloat(windowHeight) : GLfloat(windowHeight)/GLfloat(windowWidth);
    projection = glm::perspective(g_camera->fieldOfView, aspectRatio, g_camera->nearDepth, g_camera->farDepth);
    glm::mat3 normalMatrix = glm::inverse(glm::mat3(view));
    d_shaderSphere->Begin();
    d_shaderSphere->setUniformM4fv("View", view);
    d_shaderSphere->setUniformM4fv("Projection", projection);
    d_shaderSphere->setUniformM3fv("NormalMatrix", normalMatrix);
    d_shaderSphere->setUniform3fv("CameraPosition", g_camera->position);
    d_shaderSprite->End();
    d_shaderSprite->Begin();
    d_shaderSprite->setUniformM4fv("View", view);
    d_shaderSprite->setUniformM4fv("Projection", projection);
    d_shaderSprite->setUniform3fv("CameraPosition", g_camera->position);
    d_shaderSprite->End();
    d_shaderCloud->Begin();
    d_shaderCloud->setUniformM4fv("View", view);
    d_shaderCloud->setUniformM4fv("Projection", projection);
    d_shaderCloud->setUniformM3fv("NormalMatrix", normalMatrix);
    d_shaderCloud->setUniform3fv("CameraPosition", g_camera->position);
    d_shaderCloud->End();
    f_shaderSprite->Begin();
    f_shaderSprite->setUniformM4fv("View", view);
    f_shaderSprite->setUniformM4fv("Projection", projection);
    f_shaderSprite->setUniform3fv("CameraPosition", g_camera->position);
    f_shaderSprite->End();
}
void RendererGLProg::EndDraw( void ) {
    UpdateStats();
}

void RendererGLProg::UpdateStats()
{
    double frameEndTime = Timer::Seconds();
    g_stats->SetRTime(frameEndTime-r_frameStartTime);
    g_stats->SetRNumSegs(renderedSegs);
    g_stats->SetRNumTris(renderedTris);
    g_stats->SetRNumSpheres(renderedSprites);
}
void RendererGLProg::PostProcess() {
    // Make sure all our buffers have been drawn
    if ( numBufferedColorVerts || numBufferedNormalVerts ) { RenderVerts(); }
    if ( numBufferedCubes ) { Render3DCubes(); }
    if ( numBuffered3DLines ) { Render3DLines(); }

    if ( g_options->GetOptionDataPtr<bool>("r_deferred") ) {
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        bool dof = g_options->GetOptionDataPtr<bool>("r_renderDOF");
        bool flare = g_options->GetOptionDataPtr<bool>("r_renderFlare");
        bool vignette = g_options->GetOptionDataPtr<bool>("r_renderVignette");
        bool correctGamma = g_options->GetOptionDataPtr<bool>("r_renderCorrectGamma");
        bool toneMap = g_options->GetOptionDataPtr<bool>("r_renderToneMap");

        if (!dof && !flare && !vignette && !correctGamma && !toneMap) {          // No post-processing
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            RenderFromTexture(final_texture);
        } else if ( renderMode == RM_Final_Image ) {
            glm::mat4 mvp2d = glm::ortho<GLfloat>(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
            
            d_shaderPost->Begin();
            d_shaderPost->setUniformM4fv("MVP", mvp2d);
            d_shaderPost->setUniform1iv( "textureMap", 0 );
            d_shaderPost->setUniform1iv( "depthMap", 1 );
            d_shaderPost->setUniform1iv( "dustMap", 2 );
            d_shaderPost->setUniform1fv( "textureWidth", windowWidth );
            d_shaderPost->setUniform1fv( "textureHeight", windowHeight );
            d_shaderPost->setUniform1fv( "focalDepth", g_camera->focalDepth );
            d_shaderPost->setUniform1fv( "focalLength", g_camera->focalLength );
            d_shaderPost->setUniform1fv( "fstop", g_camera->fStop );
            d_shaderPost->setUniform1fv( "exposure", g_camera->exposure );
            d_shaderPost->setUniform1iv( "showFocus", g_camera->debugLens );
            d_shaderPost->setUniform1iv( "autofocus", g_camera->autoFocus );
            d_shaderPost->setUniform1iv( "renderDOF", dof );
            d_shaderPost->setUniform1iv( "renderVignette", vignette );
            d_shaderPost->setUniform1iv( "correctGamma", correctGamma);
            d_shaderPost->setUniform1iv( "toneMap", toneMap);
            d_shaderPost->setUniform1fv( "znear", g_camera->nearDepth );
            d_shaderPost->setUniform1fv( "zfar", g_camera->farDepth );
            
            glDepthMask( GL_FALSE );

            glActiveTexture(GL_TEXTURE2);
            tex_lens_dirt->Bind();
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, depth_texture);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, final_texture);
//            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

            glBindVertexArray(square2D_vao);
            glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            d_shaderPost->End();
            glBindVertexArray(0);
            renderedTris += 2;
            
        } else {    // Render debug image
            glm::mat4 mvp2d = glm::ortho<GLfloat>(-0.5, 0.5, -0.5, 0.5, -1.0, 1.0);
            // Pick which image to show
            if ( renderMode == RM_Diffuse ) {
                // Nothing fancy here, just render to screen
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                RenderFromTexture(diffuse_texture);
            } else if ( renderMode == RM_Specular ) {
                // Nothing fancy here, just render to screen
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                RenderFromTexture(specular_texture);
            } else if ( renderMode == RM_Normal ) {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                RenderFromTexture(normal_texture);
            } else if ( renderMode == RM_Depth ) {
                d_shaderDepth->Begin();
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, depth_texture);
                d_shaderDepth->setUniformM4fv( "MVP", mvp2d );
                d_shaderDepth->setUniform4fv( "u_color" , COLOR_WHITE);
                d_shaderDepth->setUniform1fv( "nearDepth" , g_camera->nearDepth);
                d_shaderDepth->setUniform1fv( "farDepth" , g_camera->farDepth);
                glBindVertexArray(square2D_vao);
                glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
                d_shaderDepth->End();
            } else if ( renderMode == RM_SSAO ) {
                PassSSAO(ao_fbo);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                RenderFromTexture(ao_texture);
            } else if ( renderMode == RM_Shadowmap ) {
                d_shaderDepth->Begin();
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, shadow_texture);
                d_shaderDepth->setUniformM4fv( "MVP", mvp2d );
                d_shaderDepth->setUniform4fv( "u_color" , COLOR_WHITE);
                d_shaderDepth->setUniform1fv( "nearDepth" , g_camera->nearDepth);
                d_shaderDepth->setUniform1fv( "farDepth" , g_camera->farDepth);
                glBindVertexArray(square2D_vao);
                glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
                d_shaderDepth->End();
            } else if ( renderMode == RM_Stencil ) {
                PassStencil(0);
            }
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        if ( g_options->GetOptionDataPtr<bool>("r_debug") ) { // Render buffers to screen for debugging
            float ar = (float)windowHeight/windowWidth;
            int wh2 = windowHeight/2;
            glDisable(GL_BLEND);
            Rect2D tRect = Rect2D(0.0,0.0,1.0,1.0);
            // Render G-Buffer to screen
            DrawTexture(Rect2D(-640,wh2-(256*ar),256,256*ar), tRect, diffuse_texture);
            DrawTexture(Rect2D(-384,wh2-(256*ar),256,256*ar), tRect, specular_texture);
            DrawTexture(Rect2D(-128,wh2-(256*ar),256,256*ar), tRect, normal_texture);
            DrawTexture(Rect2D( 128,wh2-(256*ar),256,256*ar), tRect, depth_texture);
            DrawTexture(Rect2D( 384,wh2-(256*ar),256,256*ar), tRect, ao_texture);
            DrawTexture(Rect2D( 384,wh2-(768*ar),256,256*ar), tRect, final_texture);
            if ( g_options->GetOptionDataPtr<bool>("r_debugLights") ) { // Render light buffers to screen for debugging
                DrawTexture(Rect2D(-640,wh2-(512*ar),256,256*ar), tRect, light_textures[0]);
                DrawTexture(Rect2D(-384,wh2-(512*ar),256,256*ar), tRect, light_textures[1]);
                DrawTexture(Rect2D(-128,wh2-(512*ar),256,256*ar), tRect, light_textures[2]);
                DrawTexture(Rect2D(128,wh2-(512*ar),256,256*ar), tRect, light_textures[3]);
                DrawTexture(Rect2D(384,wh2-(512*ar),256,256*ar), tRect, light_textures[4]);
            } else if ( g_options->GetOptionDataPtr<bool>("r_debugShadows") ) { // Render shadow buffers to screen for debugging
                DrawTexture(Rect2D(384,wh2-(1024*ar),256,256*ar), tRect, shadow_texture);
                // Draw cubemaps
                float min = -1.0f;
                float max = 1.0f;
                GLfloat texCoordsXP[12] = {   // +X ( flipped )
                    max, min, max,
                    max, min, min,
                    max, max, min,
                    max, max, max
                };
                GLfloat texCoordsXN[12] = {   // -X
                    min, min, min,
                    min, min, max,
                    min, max, max,
                    min, max, min
                };
                GLfloat texCoordsZP[12] = {   // +Z
                    min, min, max,
                    max, min, max,
                    max, max, max,
                    min, max, max
                };
                GLfloat texCoordsZN[12] = {   // -Z ( flipped )
                    max, min, min,
                    min, min, min,
                    min, max, min,
                    max, max, min
                };
                GLfloat texCoordsYP[12] = {   // +Y
                    min, max, min,
                    max, max, min,
                    max, max, max,
                    min, max, max
                };
                GLfloat texCoordsYN[12] = {   // -Y ( flipped )
                    max, min, min,
                    min, min, min,
                    min, min, max,
                    max, min, max
                };
                DrawCubeMap(Rect2D(-640,wh2-(512),256,256), texCoordsXN, shadow_cubeMap);//-X
                DrawCubeMap(Rect2D(-384,wh2-(512),256,256), texCoordsZP, shadow_cubeMap);//+Z
                DrawCubeMap(Rect2D(-128,wh2-(512),256,256), texCoordsXP, shadow_cubeMap);//+X
                DrawCubeMap(Rect2D( 128,wh2-(512),256,256), texCoordsZN, shadow_cubeMap);//-Z
                DrawCubeMap(Rect2D(-384,wh2-(256),256,256), texCoordsYN, shadow_cubeMap);//-Y
                DrawCubeMap(Rect2D(-384,wh2-(768),256,256), texCoordsYP, shadow_cubeMap);//+Y
            }
        }
    } else {
       // Not deferred, no post processing?
    }
}
void RendererGLProg::RenderFromTexture( const GLuint tex ) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glm::mat4 mvp2d = glm::ortho<GLfloat>(-0.5, 0.5, -0.5, 0.5, -1.0, 1.0);
    f_shaderUI_tex->Begin();
    f_shaderUI_tex->setUniformM4fv("MVP", mvp2d);
    f_shaderUI_tex->setUniform4fv("u_color", COLOR_WHITE);
    glBindVertexArray(square2D_vao);
    glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    f_shaderUI_tex->End();
    glBindVertexArray(0);
    renderedTris += 2;
}

void RendererGLProg::PassSSAO( const GLuint fbo ) {
    int blur = g_options->GetOptionDataPtr<int>("r_SSAOblur");
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);
    if ( blur ) {
        glBindFramebuffer(GL_FRAMEBUFFER, light_fbo);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, light_textures[1], 0);
        glClear(GL_COLOR_BUFFER_BIT);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    }
    // Render SSAO to texture
    glm::mat4 mvp2d = glm::ortho<GLfloat>(-0.5, 0.5, -0.5, 0.5, -1.0, 1.0);
    d_shaderSSAO->Begin();
    d_shaderSSAO->setUniformM4fv( "MVP", mvp2d );
    d_shaderSSAO->setUniform1fv( "texScale", 1.0/SSAO_SCALE );
    d_shaderSSAO->setUniform1iv( "depthMap", 1 );
    d_shaderSSAO->setUniform1iv( "rnm", 0 );
    float total_strength = g_options->GetOptionDataPtr<float>("r_SSAOtotal_strength");
    float base = g_options->GetOptionDataPtr<float>("r_SSAObase");
    float area = g_options->GetOptionDataPtr<float>("r_SSAOarea");
    float falloff = g_options->GetOptionDataPtr<float>("r_SSAOfalloff");
    float radius = g_options->GetOptionDataPtr<float>("r_SSAOradius");
    d_shaderSSAO->setUniform1fv("total_strength", total_strength);
    d_shaderSSAO->setUniform1fv("base", base);
    d_shaderSSAO->setUniform1fv("area", area);
    d_shaderSSAO->setUniform1fv("falloff", falloff);
    d_shaderSSAO->setUniform1fv("radius", radius);

    if ( !ssao_depthOnly ) {
        d_shaderSSAO->setUniform1iv( "normalMap", 2 );
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, normal_texture);
    } else {
        d_shaderSSAO->setUniform2fv( "pixelSize", glm::vec2(1.0f/(windowWidth*SSAO_SCALE), 1.0f/(windowHeight*SSAO_SCALE)) );
    }
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depth_texture);
    glActiveTexture(GL_TEXTURE0);
    tex_ssao_noise->Bind();
    glBindVertexArray(square2D_vao);
    glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
    // Render SSAO to buffer
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    d_shaderSSAO->End();
    renderedTris += 2;
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);
    
    if ( blur ) {
        // OPTIONAL - Blur horizontally - wide (tex 1 to 3)
        glBindFramebuffer(GL_FRAMEBUFFER, light_fbo);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, light_textures[3], 0);
        glClear( GL_COLOR_BUFFER_BIT );
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, light_textures[1]);
        d_shaderBlurH->Begin();
        d_shaderBlurH->setUniform1iv("Width", blur);
        d_shaderBlurH->setUniform1fv("odw", 1.0f/(windowWidth*LIGHT_SCALE));
        glBindVertexArray(square2D_vao);
        glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        d_shaderBlurH->End();
        glBindVertexArray(0);
        // Blur vertically - wide (tex 3 to buffer)
        // Render SSAO to buffer
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glActiveTexture(GL_TEXTURE0);
        glClear( GL_COLOR_BUFFER_BIT );
        glBindTexture(GL_TEXTURE_2D, light_textures[3]);
        d_shaderBlurV->Begin();
        d_shaderBlurV->setUniform1iv("Width", blur);
        d_shaderBlurV->setUniform1fv("odh", 1.0f/(windowHeight*LIGHT_SCALE));
        glBindVertexArray(square2D_vao);
        glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        d_shaderBlurV->End();
        glBindVertexArray(0);
    }
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
}
void RendererGLProg::PassStencil( const GLuint fbo ) {
    if ( fbo != render_fbo ) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, render_fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
        glBlitFramebuffer(0, 0, windowWidth, windowHeight,
                          0, 0, windowWidth, windowHeight,
                          GL_STENCIL_BUFFER_BIT, GL_NEAREST);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    
    glEnable(GL_STENCIL_TEST);
    
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilFunc(GL_EQUAL, Stencil_None, 0xFF);        // Only draw current layer
    Draw2DRect(glm::vec2(), windowWidth, windowHeight, COLOR_NONE, COLOR_GREY, 0.0f);

    glStencilFunc(GL_EQUAL, Stencil_Sky, 0xFF);        // Only draw current layer
    Draw2DRect(glm::vec2(), windowWidth, windowHeight, COLOR_NONE, COLOR_BLUE, 0.0f);
    glStencilFunc(GL_EQUAL, Stencil_Solid, 0xFF);        // Only draw current layer
    Draw2DRect(glm::vec2(), windowWidth, windowHeight, COLOR_NONE, COLOR_BROWN, 0.0f);
    glStencilFunc(GL_EQUAL, Stencil_Light, 0xFF);        // Only draw current layer
    Draw2DRect(glm::vec2(), windowWidth, windowHeight, COLOR_NONE, COLOR_YELLOW, 0.0f);
    glStencilFunc(GL_EQUAL, Stencil_Transparent, 0xFF);        // Only draw current layer
    Draw2DRect(glm::vec2(), windowWidth, windowHeight, COLOR_NONE, COLOR_PURPLE, 0.0f);
    glDisable(GL_STENCIL_TEST);
}
void RendererGLProg::PassLightRays( glm::vec3 lightWorldPos, const GLuint fbo ) {
    // Render regular image to screen first
//    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
//    RenderFromTexture(diffuse_texture);

    // Project sun coordinate to screen space
    glm::mat4 model = glm::mat4();
    model = glm::rotate(model, -g_camera->rotation.x, glm::vec3(1.0, 0.0, 0.0));
    model = glm::rotate(model, -g_camera->rotation.y, glm::vec3(0.0, 1.0, 0.0));
    model = glm::rotate(model, -g_camera->rotation.z, glm::vec3(0.0, 0.0, 1.0));
    glm::mat4 proj = glm::mat4();
    GLfloat aspectRatio = (windowWidth > windowHeight) ?
    GLfloat(windowWidth)/GLfloat(windowHeight) : GLfloat(windowHeight)/GLfloat(windowWidth);
    proj = glm::perspective(g_camera->fieldOfView, aspectRatio, g_camera->nearDepth, g_camera->farDepth);
    glm::vec4 viewport = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    
    glm::mat4 mvp2d = glm::ortho<GLfloat>(-0.5, 0.5, -0.5, 0.5, -1.0, 1.0);

    std::vector<Light3D*>& lights = g_lights3D->GetLights();
    for (int i=0; i < lights.size(); i++) {
        Light3D& light = *lights[i];
        if ( !light.active ) continue;
        if ( light.lightType == Light3D_Sun ) {
            glm::vec3 lightPos = glm::vec3(light.position.x, light.position.y, light.position.z);
            glm::vec3 lightScrnPos = glm::project(lightPos, model, proj, viewport);

            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            d_shaderLightRays->Begin();
            d_shaderLightRays->setUniformM4fv( "MVP", mvp2d );
            d_shaderLightRays->setUniform4fv( "u_color" , COLOR_WHITE);
            d_shaderLightRays->setUniform2fv( "lightScrnPos", glm::vec2(lightScrnPos.x, lightScrnPos.y) );
            d_shaderLightRays->setUniform1fv( "exposure", lr_exposure);
            d_shaderLightRays->setUniform1fv( "decay", lr_decay);
            d_shaderLightRays->setUniform1fv( "density", lr_density);
            d_shaderLightRays->setUniform1fv( "weight", lr_weight);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, light_textures[0]);    // Read from light colors
            glBindVertexArray(square2D_vao);
            glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo); // Bind our render frame buffer
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);    // Screen blend
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);    // Blend light in with regular picture
            d_shaderLightRays->End();
        }

//        // Get light and occluders from stencil buffer and render texture
//        glBindFramebuffer(GL_READ_FRAMEBUFFER, render_fbo);
//        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, light_fbo);
//        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
//
//        glBlitFramebuffer(0, 0, windowWidth, windowHeight,
//                          0, 0, windowWidth*LIGHT_SCALE, windowHeight*LIGHT_SCALE,
//                          GL_STENCIL_BUFFER_BIT, GL_NEAREST);
//        glBindFramebuffer(GL_FRAMEBUFFER, light_fbo);
//
//        glDisable(GL_DEPTH_TEST);
//        glDepthMask(GL_FALSE);
//        glEnable(GL_STENCIL_TEST);
//        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
//        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
//
//        // Draw light layer in original texture colors
//        glStencilFunc(GL_EQUAL, Stencil_Light, 0xFF);        // Only draw light layer
//
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, diffuse_texture);
//        f_shaderUI_tex->Begin();
//        f_shaderUI_tex->setUniformM4fv("MVP", mvp2d);
//        f_shaderUI_tex->setUniform4fv("u_color", COLOR_WHITE);
//        f_shaderUI_tex->setUniform1fv("texScale", 2.0f);
//        glBindVertexArray(square2D_vao);
//        glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
//        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//        f_shaderUI_tex->setUniform1fv("texScale", 1.0f);
//        f_shaderUI_tex->End();
//        glBindVertexArray(0);
//
//        // Draw occlusion layer in black
//        glStencilFunc(GL_EQUAL, Stencil_Solid, 0xFF);        // Only draw solid layer
//        DrawRect(glm::vec2(), windowWidth, windowHeight, COLOR_NONE, COLOR_BLACK, 0.0f);
//        
//        glDisable(GL_STENCIL_TEST);
//        glBlendFunc(GL_ONE, GL_ONE);  // Additive
//        glBlendFunc(GL_DST_COLOR, GL_ZERO);   // Multiply
    }
}
glm::mat4 RendererGLProg::GetLightMVP(Light3D& light) {
    if ( light.lightType == Light3D_Directional ||
        light.lightType == Light3D_Sun ) {
        // Camera projection matrices
        glm::mat4 model = glm::mat4();
        glm::mat4 proj = glm::mat4();
        model = glm::rotate(model, -g_camera->rotation.x, glm::vec3(1.0, 0.0, 0.0));
        model = glm::rotate(model, -g_camera->rotation.y, glm::vec3(0.0, 1.0, 0.0));
        model = glm::rotate(model, -g_camera->rotation.z, glm::vec3(0.0, 0.0, 1.0));
        model = glm::translate(model, glm::vec3(-g_camera->position.x, -g_camera->position.y, -g_camera->position.z));
        glm::vec4 viewport = glm::vec4(0, 0, windowWidth, windowHeight);
        GLfloat aspectRatio = (windowWidth > windowHeight) ?
        GLfloat(windowWidth)/GLfloat(windowHeight) : GLfloat(windowHeight)/GLfloat(windowWidth);
        proj = glm::perspective(g_camera->fieldOfView, aspectRatio, g_camera->nearDepth, g_camera->farDepth);
        
        
        // Frustum far plane corner coordinates in world space
        glm::vec3 viewVerts[8];
        float cornerZ = 1.0f;    // Far plane
        viewVerts[0] = glm::unProject(glm::vec3(0.0f, 0.0f, cornerZ), model, proj, viewport);
        viewVerts[1] = glm::unProject(glm::vec3(windowWidth, 0.0f, cornerZ), model, proj, viewport);
        viewVerts[2] = glm::unProject(glm::vec3(windowWidth, windowHeight, cornerZ), model, proj, viewport);
        viewVerts[3] = glm::unProject(glm::vec3(0.0f, windowHeight, cornerZ), model, proj, viewport);
        cornerZ = 0.0f;          // Near plane
        viewVerts[4] = glm::unProject(glm::vec3(0.0f, 0.0f, cornerZ), model, proj, viewport);
        viewVerts[5] = glm::unProject(glm::vec3(windowWidth, 0.0f, cornerZ), model, proj, viewport);
        viewVerts[6] = glm::unProject(glm::vec3(windowWidth, windowHeight, cornerZ), model, proj, viewport);
        viewVerts[7] = glm::unProject(glm::vec3(0.0f, windowHeight, cornerZ), model, proj, viewport);
        
        glm::vec3 lightInvDir = glm::vec3(light.position.x,light.position.y,light.position.z);
        glm::mat4 lightViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,1.0f,0.0f));
        glm::mat4 lightModelMatrix = glm::mat4(1.0);
        glm::vec4 tempCorner = lightViewMatrix*glm::vec4(viewVerts[0].x,viewVerts[0].y,viewVerts[0].z,1.0f);
        glm::vec3 min = glm::vec3(tempCorner.x,tempCorner.y,tempCorner.z);  // Min for frustum in light view AABB
        glm::vec3 max = min;                                                // Max for frustum in light view AABB
        
        for (int i=1; i<8; i++) {
            // Project frustum corners into light view space
            tempCorner = lightViewMatrix*glm::vec4(viewVerts[i].x,viewVerts[i].y,viewVerts[i].z,1.0f);
            // Get max and min values for corners
            if ( tempCorner.x < min.x ) min.x = tempCorner.x;
            if ( tempCorner.y < min.y ) min.y = tempCorner.y;
            if ( tempCorner.z < min.z ) min.z = tempCorner.z;
            if ( tempCorner.x > max.x ) max.x = tempCorner.x;
            if ( tempCorner.y > max.y ) max.y = tempCorner.y;
            if ( tempCorner.z > max.z ) max.z = tempCorner.z;
        }
        // Add a bit more to max and min here to reduce shadow popping?
        max += glm::vec3(g_camera->farDepth*0.2f);
        min -= glm::vec3(g_camera->farDepth*0.2f);
        
        // Compute the MVP matrix from the light's point of view
        glm::mat4 lightProjectionMatrix = glm::ortho<float>(min.x,max.x,min.y,max.y,-max.z,-min.z);
        glm::mat4 lightMVP = lightProjectionMatrix * lightViewMatrix * lightModelMatrix;
        return lightMVP;
    } else if ( light.lightType == Light3D_Spot && light.position.w > 0.0f ) {
        glm::vec3 lightPos = glm::vec3(light.position.x,light.position.y,light.position.z);
        glm::mat4 lightViewMatrix = glm::lookAt(glm::normalize(-light.direction), glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0,1,0));
        glm::mat4 lightModelMatrix = glm::translate(glm::mat4(1.0f), -lightPos);
        glm::mat4 lightProjectionMatrix = glm::perspective(light.spotCutoff*2.0f, 1.0f, 0.05f, light.position.w);
        glm::mat4 lightMVP = lightProjectionMatrix * lightViewMatrix * lightModelMatrix;
        return lightMVP;
    }
    return glm::mat4(1.0f);
}

glm::mat4 RendererGLProg::GetLightModel(Light3D& light) {
    glm::vec3 lightPos = glm::vec3(light.position.x,light.position.y,light.position.z);
    return glm::translate(glm::mat4(1.0f), -lightPos);
}
glm::mat4 RendererGLProg::GetLightView(Light3D& light) {
    if ( light.lightType == Light3D_Directional ||
        light.lightType == Light3D_Sun ) {
        glm::vec3 lightInvDir = glm::vec3(light.position.x,light.position.y,light.position.z);
        glm::mat4 lightViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,1.0f,0.0f));
        return lightViewMatrix;
    } else if ( light.lightType == Light3D_Spot ) {
        glm::mat4 lightViewMatrix = glm::lookAt(glm::normalize(-light.direction), glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0,1,0));
        return lightViewMatrix;
    }
    return glm::mat4(1.0f);
}
glm::mat4 RendererGLProg::GetLightProjection(Light3D& light) {
//    if ( light.lightType == Light3D_Directional ||
//        light.lightType == Light3D_Sun ) {
//        // Camera projection matrices
//        Camera& camera = m_hyperVisor->GetCamera();
//        glm::mat4 model = glm::mat4();
//        glm::mat4 proj = glm::mat4();
//        model = glm::rotate(model, -g_camera->rotation.x, glm::vec3(1.0, 0.0, 0.0));
//        model = glm::rotate(model, -g_camera->rotation.y, glm::vec3(0.0, 1.0, 0.0));
//        model = glm::rotate(model, -g_camera->rotation.z, glm::vec3(0.0, 0.0, 1.0));
//        model = glm::translate(model, glm::vec3(-g_camera->position.x, -g_camera->position.y, -g_camera->position.z));
//        glm::vec4 viewport = glm::vec4(0, 0, windowWidth, windowHeight);
//        GLfloat aspectRatio = (windowWidth > windowHeight) ?
//        GLfloat(windowWidth)/GLfloat(windowHeight) : GLfloat(windowHeight)/GLfloat(windowWidth);
//        proj = glm::perspective(g_camera->fieldOfView, aspectRatio, g_camera->nearDepth, g_camera->farDepth);
//        
//        // Frustum far plane corner coordinates in world space
//        glm::vec3 viewVerts[8];
//        float cornerZ = 1.0f;    // Far plane
//        viewVerts[0] = glm::unProject(glm::vec3(0.0f, 0.0f, cornerZ), model, proj, viewport);
//        viewVerts[1] = glm::unProject(glm::vec3(windowWidth, 0.0f, cornerZ), model, proj, viewport);
//        viewVerts[2] = glm::unProject(glm::vec3(windowWidth, windowHeight, cornerZ), model, proj, viewport);
//        viewVerts[3] = glm::unProject(glm::vec3(0.0f, windowHeight, cornerZ), model, proj, viewport);
//        cornerZ = 0.0f;          // Near plane
//        viewVerts[4] = glm::unProject(glm::vec3(0.0f, 0.0f, cornerZ), model, proj, viewport);
//        viewVerts[5] = glm::unProject(glm::vec3(windowWidth, 0.0f, cornerZ), model, proj, viewport);
//        viewVerts[6] = glm::unProject(glm::vec3(windowWidth, windowHeight, cornerZ), model, proj, viewport);
//        viewVerts[7] = glm::unProject(glm::vec3(0.0f, windowHeight, cornerZ), model, proj, viewport);
//        
//        glm::vec3 lightInvDir = glm::vec3(light.position.x,light.position.y,light.position.z);
//        glm::mat4 lightViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,1.0f,0.0f));
//        glm::vec4 tempCorner = lightViewMatrix*glm::vec4(viewVerts[0].x,viewVerts[0].y,viewVerts[0].z,1.0f);
//        glm::vec3 min = glm::vec3(tempCorner.x,tempCorner.y,tempCorner.z);  // Min for frustum in light view AABB
//        glm::vec3 max = min;                                                // Max for frustum in light view AABB
//        
//        for (int i=1; i<8; i++) {
//            // Project frustum corners into light view space
//            tempCorner = lightViewMatrix*glm::vec4(viewVerts[i].x,viewVerts[i].y,viewVerts[i].z,1.0f);
//            // Get max and min values for corners
//            if ( tempCorner.x < min.x ) min.x = tempCorner.x;
//            if ( tempCorner.y < min.y ) min.y = tempCorner.y;
//            if ( tempCorner.z < min.z ) min.z = tempCorner.z;
//            if ( tempCorner.x > max.x ) max.x = tempCorner.x;
//            if ( tempCorner.y > max.y ) max.y = tempCorner.y;
//            if ( tempCorner.z > max.z ) max.z = tempCorner.z;
//        }
//        // Add a bit more to max and min here to reduce shadow popping?
//        max += glm::vec3(g_camera->farDepth*0.1f);
//        min -= glm::vec3(g_camera->farDepth*0.1f);
//        
//        // Compute the MVP matrix from the light's point of view
//        glm::mat4 lightProjectionMatrix = glm::ortho<float>(min.x,max.x,min.y,max.y,-max.z,-min.z);
//        return lightProjectionMatrix;
//    } else if ( light.lightType == Light3D_Spot ) {
//        glm::mat4 lightProjectionMatrix = glm::perspective(light.spotCutoff*2.0f, 1.0f, 0.05f, light.position.w);
//        return lightProjectionMatrix;
//    }
    return glm::mat4(1.0f);
}
// Get lights from LightSystem3D
void RendererGLProg::RenderLighting( const Color& fogColor ) {
    // Get Ambient Occlusion if needed
    bool ssao = g_options->GetOptionDataPtr<bool>("r_renderSSAO");
    if ( ssao ) {  PassSSAO(ao_fbo); }
    bool lightRays = g_options->GetOptionDataPtr<bool>("r_lightRays");
    bool renderFog = g_options->GetOptionDataPtr<bool>("r_renderFog");
    bool shadowMultitap = g_options->GetOptionDataPtr<bool>("r_shadowMultitap");
    bool shadowNoise = g_options->GetOptionDataPtr<bool>("r_shadowNoise");
    bool shadows = g_options->GetOptionDataPtr<bool>("r_shadows");
    float fogDensity = g_options->GetOptionDataPtr<float>("r_fogDensity");
    float fogHeightFalloff = g_options->GetOptionDataPtr<float>("r_fogHeightFalloff");
    float fogExtinctionFalloff = g_options->GetOptionDataPtr<float>("r_fogExtinctionFalloff");
    float fogInscatteringFalloff = g_options->GetOptionDataPtr<float>("r_fogInscatteringFalloff");
    
    
    // Output to final image FBO
    glBindFramebuffer(GL_FRAMEBUFFER, final_fbo);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    
    if ( !g_options->GetOptionDataPtr<bool>("r_lighting3D") ) {
        RenderFromTexture(diffuse_texture);
        return;
    }

    // Draw sky layer without lighting and opaque layer in black
    glEnable(GL_STENCIL_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilFunc(GL_EQUAL, Stencil_Sky, 0xFF);             // Only draw sky layer
    RenderFromTexture(diffuse_texture);
    glStencilFunc(GL_EQUAL, Stencil_Solid, 0xFF);           // Only draw solid layer
    Draw2DRect(glm::vec2(), windowWidth, windowHeight, COLOR_NONE, COLOR_BLACK, 0.0f);
    glDisable(GL_STENCIL_TEST);

    // Parameters for linearizing depth value
    glm::vec2 depthParameter = glm::vec2( g_camera->farDepth / ( g_camera->farDepth - g_camera->nearDepth ),
                                         g_camera->farDepth * g_camera->nearDepth / ( g_camera->nearDepth - g_camera->farDepth ) );

    d_shaderLight->Begin();
    d_shaderLight->setUniform1iv("diffuseMap", 0);
    d_shaderLight->setUniform1iv("specularMap", 1);
    d_shaderLight->setUniform1iv("normalMap", 2);
    d_shaderLight->setUniform1iv("depthMap", 3);
    d_shaderLight->setUniform2fv("depthParameter", depthParameter);
    d_shaderLight->setUniform1fv("farDepth", g_camera->farDepth);
    d_shaderLight->setUniform1fv("nearDepth", g_camera->nearDepth);
    d_shaderLight->setUniform3fv("camPos", g_camera->position);
    d_shaderLight->setUniform1iv("renderSSAO", ssao );
    d_shaderLight->setUniform1iv("ssaoMap", 4 );
    d_shaderLight->setUniform1iv("renderFog", renderFog);
    d_shaderLight->setUniform1fv("fogDensity", fogDensity);
    d_shaderLight->setUniform1fv("fogHeightFalloff", fogHeightFalloff);
    d_shaderLight->setUniform1fv("fogExtinctionFalloff", fogExtinctionFalloff);
    d_shaderLight->setUniform1fv("fogInscatteringFalloff", fogInscatteringFalloff);
    d_shaderLight->setUniform3fv("fogColor", fogColor.r,fogColor.g,fogColor.b);
    d_shaderLight->End();
    
    d_shaderLightShadow->Begin();
    d_shaderLightShadow->setUniform1iv("diffuseMap", 0);
    d_shaderLightShadow->setUniform1iv("specularMap", 1);
    d_shaderLightShadow->setUniform1iv("normalMap", 2);
    d_shaderLightShadow->setUniform1iv("depthMap", 3);
    d_shaderLightShadow->setUniform1iv("ssaoMap", 4 );
    d_shaderLightShadow->setUniform1iv("shadowMap", 5);
    d_shaderLightShadow->setUniform1iv("shadowCubeMap", 6);
    d_shaderLightShadow->setUniform2fv("depthParameter", depthParameter);
    d_shaderLightShadow->setUniform1fv("farDepth", g_camera->farDepth);
    d_shaderLightShadow->setUniform1fv("nearDepth", g_camera->nearDepth);
    d_shaderLightShadow->setUniform3fv("camPos", g_camera->position);
    d_shaderLightShadow->setUniform1fv("shadowRes", SHADOW_MAP_SIZE);
    d_shaderLightShadow->setUniform1iv("renderSSAO", ssao );
    d_shaderLightShadow->setUniform1iv("shadowMultitap", shadowMultitap);
    d_shaderLightShadow->setUniform1iv("shadowNoise", shadowNoise);
    d_shaderLightShadow->setUniform1iv("renderFog", renderFog);
    d_shaderLightShadow->setUniform1fv("fogDensity", fogDensity);
    d_shaderLightShadow->setUniform1fv("fogHeightFalloff", fogHeightFalloff);
    d_shaderLightShadow->setUniform1fv("fogExtinctionFalloff", fogExtinctionFalloff);
    d_shaderLightShadow->setUniform1fv("fogInscatteringFalloff", fogInscatteringFalloff);
    d_shaderLightShadow->setUniform3fv("fogColor", fogColor.r,fogColor.g,fogColor.b);
    d_shaderLightShadow->End();

    glm::mat4 model = glm::mat4();
    model = glm::rotate(model, -g_camera->rotation.x, glm::vec3(1.0, 0.0, 0.0));
    model = glm::rotate(model, -g_camera->rotation.y, glm::vec3(0.0, 1.0, 0.0));
    model = glm::rotate(model, -g_camera->rotation.z, glm::vec3(0.0, 0.0, 1.0));
    glm::vec4 viewport = glm::vec4(0, 0, windowWidth, windowHeight);
    glm::mat4 proj = glm::mat4();
    GLfloat aspectRatio = (windowWidth > windowHeight) ?
    GLfloat(windowWidth)/GLfloat(windowHeight) : GLfloat(windowHeight)/GLfloat(windowWidth);
    proj = glm::perspective(g_camera->fieldOfView, aspectRatio, g_camera->nearDepth, g_camera->farDepth);

    // Frustum far plane corner coordinates
    glm::vec3 viewVerts[4];
    float cz = 1.0f;
    viewVerts[0] = glm::unProject(glm::vec3(0.0f, 0.0f, cz), model, proj, viewport);
    viewVerts[1] = glm::unProject(glm::vec3(windowWidth, 0.0f, cz), model, proj, viewport);
    viewVerts[2] = glm::unProject(glm::vec3(windowWidth, windowHeight, cz), model, proj, viewport);
    viewVerts[3] = glm::unProject(glm::vec3(0.0f, windowHeight, cz), model, proj, viewport);
    
    // Prepare VAO for light render
    glBindVertexArray(vertex_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square2D_coords)+sizeof(square2D_texCoords)+sizeof(GLfloat)*3*4, square2D_coords, GL_STATIC_DRAW);
    // Vertices & texcoords
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(square2D_coords), sizeof(square2D_texCoords), square2D_texCoords);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(square2D_coords)+sizeof(square2D_texCoords), sizeof(GLfloat)*3*4, viewVerts);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(sizeof(square2D_coords)));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(sizeof(square2D_coords)+sizeof(square2D_texCoords)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specular_texture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, normal_texture);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depth_texture);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, ao_texture);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, shadow_texture);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_CUBE_MAP, shadow_cubeMap);
    glBindFramebuffer(GL_FRAMEBUFFER, final_fbo);

    // Ready stencil to draw lighting only over solid geometry
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_LEQUAL, Stencil_Solid, 0xFF);        // Only draw on solid layer
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    
    // Render all lights
    std::vector<Light3D*>& lights = g_lights3D->GetLights();
    for (int i=0; i < lights.size(); i++) {
        Light3D& light = *lights[i];
        if ( !light.active ) continue;

        if ( light.shadowCaster && shadows ) {
            glm::mat4 biasMatrix(0.5, 0.0, 0.0, 0.0,
                                 0.0, 0.5, 0.0, 0.0,
                                 0.0, 0.0, 0.5, 0.0,
                                 0.5, 0.5, 0.5, 1.0);
            d_shaderLightShadow->Begin();
            if ( light.lightType == Light3D_Point ) {
                glm::vec3 lightPos = glm::vec3(light.position.x,light.position.y,light.position.z);
                glm::mat4 lightModelMatrix = glm::translate(glm::mat4(1.0f), -lightPos);
                glm::mat4 lightProjectionMatrix = glm::perspective(90.0f, 1.0f, 0.1f, light.position.w);
                glm::mat4 lightMVP[6] = {
                    (lightProjectionMatrix * lightViewMatrix[0] * lightModelMatrix),
                    (lightProjectionMatrix * lightViewMatrix[1] * lightModelMatrix),
                    (lightProjectionMatrix * lightViewMatrix[2] * lightModelMatrix),
                    (lightProjectionMatrix * lightViewMatrix[3] * lightModelMatrix),
                    (lightProjectionMatrix * lightViewMatrix[4] * lightModelMatrix),
                    (lightProjectionMatrix * lightViewMatrix[5] * lightModelMatrix)};
                d_shaderLightShadow->setUniform1iv("useCubeMap", true);
                GLuint matUniform = glGetUniformLocation(d_shaderLightShadow->GetProgram(), "shadowCubeMVP");
                glUniformMatrix4fv(matUniform, 6, GL_FALSE, (GLfloat*)lightMVP);
            } else {
                glm::mat4 lightMVP = GetLightMVP(light);
                glm::mat4 depthBiasMVP = biasMatrix*lightMVP;

                d_shaderLightShadow->setUniform1iv("useCubeMap", false);
                d_shaderLightShadow->setUniformM4fv("shadowMVP", depthBiasMVP);
            }
            d_shaderLightShadow->setUniform4fv("lightPosition", light.position);
            d_shaderLightShadow->setUniform4fv("lightAmbient", light.ambient);
            d_shaderLightShadow->setUniform4fv("lightDiffuse", light.diffuse);
            d_shaderLightShadow->setUniform4fv("lightSpecular", light.specular);
            d_shaderLightShadow->setUniform3fv("lightAttenuation", light.attenuation);
            d_shaderLightShadow->setUniform3fv("lightSpotDirection", light.direction);
            d_shaderLightShadow->setUniform1fv("lightSpotCutoff", light.spotCutoff);
            d_shaderLightShadow->setUniform1fv("lightSpotExponent", light.spotExponent);
//            d_shaderLightShadow->setUniform1fv("seed", glfwGetTime());
        } else {
            d_shaderLight->Begin();
            d_shaderLight->setUniform4fv("lightPosition", light.position);
            d_shaderLight->setUniform4fv("lightAmbient", light.ambient);
            d_shaderLight->setUniform4fv("lightDiffuse", light.diffuse);
            d_shaderLight->setUniform4fv("lightSpecular", light.specular);
            d_shaderLight->setUniform3fv("lightAttenuation", light.attenuation);
            d_shaderLight->setUniform3fv("lightSpotDirection", light.direction);
            d_shaderLight->setUniform1fv("lightSpotCutoff", light.spotCutoff);
            d_shaderLight->setUniform1fv("lightSpotExponent", light.spotExponent);
//            d_shaderLight->setUniform1fv("seed", glfwGetTime());
        }
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    
    d_shaderLight->End();
    glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_STENCIL_TEST);

    model = glm::translate(model, glm::vec3(-g_camera->position.x, -g_camera->position.y, -g_camera->position.z));
    bool renderSun = g_options->GetOptionDataPtr<bool>("r_sun");
    bool renderSunBlur = g_options->GetOptionDataPtr<bool>("r_sunBlur");
    bool renderSunFlare = g_options->GetOptionDataPtr<bool>("r_sunFlare");

    if ( renderSun ) {  // Copy stencil buffer at lower res to light FBO
        glBindFramebuffer(GL_FRAMEBUFFER, light_fbo);
        glClear( GL_STENCIL_BUFFER_BIT );   // Clear light stencil
        glBindFramebuffer(GL_READ_FRAMEBUFFER, render_fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, light_fbo);
        glBlitFramebuffer(0, 0, windowWidth, windowHeight,
                          0, 0, windowWidth*LIGHT_SCALE, windowHeight*LIGHT_SCALE,
                          GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
    }
    
    // STAGE 2
    // 2D Radial beam lights in 3D / add lens flare
    for (int i=0; i<lights.size(); i++) {
        Light3D& light = *lights[i];
        if ( !light.active ) continue;
        if ( light.rayCaster ) {
            // Project light coordinate to screen space
            int windowWidth = g_options->GetOptionDataPtr<int>("r_resolutionX");
            int windowHeight = g_options->GetOptionDataPtr<int>("r_resolutionY");
            glm::vec3 lightPosition = glm::vec3(light.position.x,light.position.y,light.position.z);
            float radius = 8.0f; // 8 pixel radius minimum?
            if ( light.lightType == Light3D_Directional ||
                light.lightType == Light3D_Sun ) {
                lightPosition = g_camera->position+(lightPosition*100.0f);
            }
            glm::vec3 lightScrnPos = glm::project(lightPosition, model, proj, viewport);
            glm::vec2 lightScrn2D = glm::vec2(lightScrnPos.x,lightScrnPos.y);
            if ( renderSun && lightScrnPos.z < 1.0005f) {
//                printf("Sun: %f,%f,%f\n", lightScrnPos.x,lightScrnPos.y,lightScrnPos.z);

                // Fix light screen position relative to light texture
                lightScrn2D *= (float)LIGHT_SCALE;
                lightScrn2D -= glm::vec2(windowWidth*0.5f,windowHeight*0.5f);
                // Render simple sun circle into light buffer 0
                glBindFramebuffer(GL_FRAMEBUFFER, light_fbo);
                glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, light_textures[0], 0);
                glClear( GL_COLOR_BUFFER_BIT );
                
                
                if ( light.position.w == 0.0f ) {   // Light has no radius, we give it an arbitrary one
                    radius = 32.0f; // Default sun radius
                } else {
                    glm::vec2 depthParameter = glm::vec2( g_camera->farDepth / ( g_camera->farDepth - g_camera->nearDepth ),
                                                         g_camera->farDepth * g_camera->nearDepth / ( g_camera->nearDepth - g_camera->farDepth ) );
                    float Z = depthParameter.y/(depthParameter.x - lightScrnPos.z);
                    // Scale light radius by radius/1.0-depth to keep constant relative size
                    radius = light.position.w*128.0f/(1.0-Z);
                }
                if ( radius < 4.0f ) continue;
//                radius = fmaxf(radius, 4.0f);
                radius = fminf(radius, 1024.0f);
                glEnable(GL_DEPTH_TEST);
                float lightZ = ORTHO_FARDEPTH+(ORTHO_NEARDEPTH-ORTHO_FARDEPTH)*fminf(lightScrnPos.z, 1.0f);
//                printf("Light Z: %f, ortho: %f\n", lightScrnPos.z, lightZ);
                // Draw a simple 2D circle at light position
                DrawCircle(lightScrn2D, 0.0f, radius*LIGHT_SCALE, COLOR_NONE, light.diffuse, lightZ, 1);
                glDisable(GL_DEPTH_TEST);

                // Carve out solid objects in front of light (not needed if using depth buffer)
//                glEnable(GL_STENCIL_TEST);
//                glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
//                glDisable(GL_BLEND);
//                glStencilFunc(GL_EQUAL, Stencil_Solid, 0xFF);        // Only draw solid layer
//                DrawRect(glm::vec2(), windowWidth, windowHeight, COLOR_NONE, COLOR_BLACK, 0.0f);
//                glDisable(GL_STENCIL_TEST);
                
                if ( renderSunBlur ) {
                    // Blur horizontally - narrow (tex 0 to 3)
                    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, light_textures[3], 0);
                    glClear( GL_COLOR_BUFFER_BIT );
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, light_textures[0]);
                    d_shaderBlurH->Begin();
                    d_shaderBlurH->setUniform1iv("Width", 1);
                    d_shaderBlurH->setUniform1fv("odw", 1.0f/(windowWidth*LIGHT_SCALE));
                    glBindVertexArray(square2D_vao);
                    glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
                    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
                    d_shaderBlurH->End();
                    glBindVertexArray(0);
                    // Blur horizontally - narrow (tex 3 to 1)
                    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, light_textures[1], 0);
                    glClear( GL_COLOR_BUFFER_BIT );
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, light_textures[3]);
                    d_shaderBlurV->Begin();
                    d_shaderBlurV->setUniform1iv("Width", 1);
                    d_shaderBlurV->setUniform1fv("odh", 1.0f/(windowHeight*LIGHT_SCALE));
                    glBindVertexArray(square2D_vao);
                    glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
                    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
                    d_shaderBlurV->End();
                    glBindVertexArray(0);
                    if ( !lightRays ) {
                        // Blur horizontally - wide (tex 0 to 3)
                        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, light_textures[3], 0);
                        glClear( GL_COLOR_BUFFER_BIT );
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, light_textures[0]);
                        d_shaderBlurH->Begin();
                        d_shaderBlurH->setUniform1iv("Width", 10);
                        d_shaderBlurH->setUniform1fv("odw", 1.0f/(windowWidth*LIGHT_SCALE));
                        glBindVertexArray(square2D_vao);
                        glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
                        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
                        d_shaderBlurH->End();
                        glBindVertexArray(0);
                        // Blur vertically - wide (tex 3 to 2)
                        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, light_textures[2], 0);
                        glActiveTexture(GL_TEXTURE0);
                        glClear( GL_COLOR_BUFFER_BIT );
                        glBindTexture(GL_TEXTURE_2D, light_textures[3]);
                        d_shaderBlurV->Begin();
                        d_shaderBlurV->setUniform1iv("Width", 10);
                        d_shaderBlurV->setUniform1fv("odh", 1.0f/(windowHeight*LIGHT_SCALE));
                        glBindVertexArray(square2D_vao);
                        glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
                        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
                        d_shaderBlurV->End();
                        glBindVertexArray(0);
                    }
                }
                if ( lightRays ) {  // Radial light beams
                    // Update sun into light buffer tex 2
                    glBindFramebuffer(GL_FRAMEBUFFER, light_fbo);
                    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, light_textures[2], 0);
                    glViewport(0, 0, windowWidth*LIGHT_SCALE, windowHeight*LIGHT_SCALE);
                    glClear( GL_COLOR_BUFFER_BIT );
                    glm::mat4 mvp2d = glm::ortho<GLfloat>(-0.5, 0.5, -0.5, 0.5, -1.0, 1.0);
                    glm::vec2 lightScrnUnit = glm::vec2(lightScrnPos.x,lightScrnPos.y)/glm::vec2(windowWidth,windowHeight);
                    d_shaderLightRays->Begin();
                    d_shaderLightRays->setUniformM4fv( "MVP", mvp2d );
                    d_shaderLightRays->setUniform4fv( "u_color" , COLOR_WHITE);
                    d_shaderLightRays->setUniform2fv( "lightScrnPos", lightScrnUnit );
                    d_shaderLightRays->setUniform1fv( "exposure", lr_exposure);
                    d_shaderLightRays->setUniform1fv( "decay", lr_decay);
                    d_shaderLightRays->setUniform1fv( "density", lr_density);
                    d_shaderLightRays->setUniform1fv( "weight", lr_weight);
                    glActiveTexture(GL_TEXTURE0);
                    if ( renderSunBlur ) {
                        glBindTexture(GL_TEXTURE_2D, light_textures[1]);    // Read from blurred light colors
                    } else {
                        glBindTexture(GL_TEXTURE_2D, light_textures[0]);    // Read from light colors
                    }
                    glBindVertexArray(square2D_vao);
                    glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
                    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);    // Blend light in with regular picture
                    d_shaderLightRays->End();
                    glViewport(0, 0, windowWidth, windowHeight);    // reset viewport
                }
                if ( renderSunFlare ) {
                    
                    // blur sun sphere radially and calculate lens flare and halo and apply dirt texture (to tex 3)
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, light_textures[3], 0);
                    glClear( GL_COLOR_BUFFER_BIT );
                    if ( renderSunBlur ) {
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, light_textures[1]);
                        glActiveTexture(GL_TEXTURE1);
                        glBindTexture(GL_TEXTURE_2D, light_textures[2]);
                    } else {
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, light_textures[0]);
                        glActiveTexture(GL_TEXTURE1);
                        if ( lightRays ) {
                            glBindTexture(GL_TEXTURE_2D, light_textures[2]);
                        } else {
                            glBindTexture(GL_TEXTURE_2D, light_textures[0]);
                        }
                    }
                    glActiveTexture(GL_TEXTURE2);
                    tex_lens_dirt->Bind();
                    d_shaderSunPP->Begin();
                    glm::vec2 lightScrnUnit = glm::vec2(lightScrnPos.x,lightScrnPos.y)/glm::vec2(windowWidth,windowHeight);
                    d_shaderSunPP->setUniform2fv("SunPosProj", lightScrnUnit);
                    glBindVertexArray(square2D_vao);
                    glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
                    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
                    d_shaderSunPP->End();
                    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, 0);
                    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);
                    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
                }
                // Render final light to FBO
                glBindFramebuffer(GL_FRAMEBUFFER, final_fbo);
                glViewport(0, 0, windowWidth, windowHeight);
                glEnable(GL_BLEND);
                glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
                if ( renderSunFlare ) {
                    RenderFromTexture(light_textures[3]);
                } else {
                    if ( lightRays ) {
                        RenderFromTexture(light_textures[2]);
                    } else {
                        if ( renderSunBlur ) {
                            RenderFromTexture(light_textures[2]);
                        } else {
                            RenderFromTexture(light_textures[0]);
                        }
                    }
                }
            }
        }
    }
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, final_fbo);
//    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, final_texture, 0);
//    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depth_texture, 0);  // Attach previous depth/stencil to it
//    glBindVertexArray(0);
    g_stats->SetRNumLights3D((int)lights.size());
}
void RendererGLProg::RenderVertBuffer( VertexBuffer* vBuffer,
                                      const unsigned int rangeEnd,
                                      const unsigned int rangeStart,
                                      const Texture* tex,
                                      const bool render3D ) {
    if ( vBuffer == NULL ) return;	// No buffer to render
    if ( vBuffer->numTVerts == 0 && vBuffer->numVerts == 0 )  return;	// No verts in buffer
	bool clearBufferOnUpdate = true;	// Do we dealloc the vertices after uploading to gpu


    if ( vBuffer->GetID() == -1 ) { // Vertex buffer not initialized in hardware yet
        GLuint newVBO = 0;
        glGenBuffers(1, &newVBO);
        vBuffer->SetID(newVBO);
        vertexBuffers.push_back(vBuffer);   // Add it to our list in case the renderer needs to close
    }
    // STAGE 1 - PREPARE BUFFER AND POINTERS, UPLOAD DATA IF UPDATED
	if ( vBuffer->bufferType == RType_NormalVerts ) {
        glBindVertexArray(normalVerts_vao);
        glBindBuffer(GL_ARRAY_BUFFER, vBuffer->GetID());
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(NormalVertexData), 0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(NormalVertexData), (GLvoid*)(4*sizeof(GLfloat)));
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(NormalVertexData), (GLvoid*)(8*sizeof(GLfloat)));
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(NormalVertexData), (GLvoid*)(9*sizeof(GLfloat)));
		if (vBuffer->updated && vBuffer->n_verts) {   // Upload vertex data to GPU
            glBufferData(GL_ARRAY_BUFFER, (vBuffer->numVerts+vBuffer->numTVerts)*sizeof(NormalVertexData), vBuffer->n_verts, GL_STATIC_DRAW);
            vBuffer->updated = false;
			if (clearBufferOnUpdate) {
				delete[] vBuffer->n_verts;
				vBuffer->n_verts = NULL;
			}
        }
    } else if ( vBuffer->bufferType == RType_ColorVerts ||
                vBuffer->bufferType == RType_SphereVerts ||
                vBuffer->bufferType == RType_CloudVerts ||
                vBuffer->bufferType == RType_SpriteVerts ) {
        glBindVertexArray(sprite_vao);
        glBindBuffer(GL_ARRAY_BUFFER, vBuffer->GetID());
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), 0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (GLvoid*)(4*sizeof(GLfloat)));
        if ( vBuffer->updated && vBuffer->c_verts ) {   // Upload vertex data to GPU
            glBufferData(GL_ARRAY_BUFFER, (vBuffer->numVerts+vBuffer->numTVerts)*sizeof(ColorVertexData), vBuffer->c_verts, GL_STATIC_DRAW);
			vBuffer->updated = false;
			// DONT DO THIS FOR PARTICLE SYSTEMS, THEY NEED THE BUFFER CONSTANTLY
			//if (clearBufferOnUpdate) {
			//	delete[] vBuffer->c_verts;
			//	vBuffer->c_verts = NULL;
			//}
        }
        if ( tex != NULL ) {    // Bind texture if necessary
            glActiveTexture(GL_TEXTURE0);
            tex->Bind();
        }
    }
    // Optionally render shadows here
    if ( g_options->GetOptionDataPtr<bool>("r_shadows") && render3D ) {
        GLint m_oldFrameBuffer=0;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &m_oldFrameBuffer);
        RenderVertBufferShadows(vBuffer, rangeEnd, rangeStart );
        glBindFramebuffer(GL_FRAMEBUFFER, m_oldFrameBuffer);
        glViewport(0, 0, windowWidth, windowHeight);
    }
    // STAGE 2 - RENDER BUFFER CONTENTS
    if ( vBuffer->bufferType == RType_NormalVerts ) {
        d_shaderMesh->Begin();
        glDrawArrays(GL_TRIANGLES, rangeStart, rangeEnd);
        d_shaderMesh->End();
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        renderedTris += (rangeEnd/3);
    } else if ( vBuffer->bufferType == RType_SpriteVerts ) {
        if ( render3D ) {
            d_shaderSprite->Begin();
        } else {
            f_shaderSprite->Begin();
            f_shaderSprite->setUniformM4fv("View", glm::mat4());
            f_shaderSprite->setUniformM4fv("Projection", mvp2D);
            f_shaderSprite->setUniform3fv("CameraPosition", glm::vec3());
        }
        glDrawArrays(GL_POINTS, rangeStart, rangeEnd);
        d_shaderSprite->End();
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        renderedSprites = rangeEnd;
    } else if ( vBuffer->bufferType == RType_SphereVerts ) {
        d_shaderSphere->Begin();
        if ( render3D == false ) {  // Pass 2D matrices to shader
            d_shaderSphere->setUniformM4fv("View", glm::mat4());
            d_shaderSphere->setUniformM4fv("Projection", mvp2D);
            d_shaderSphere->setUniformM3fv("NormalMatrix", glm::mat3());
            d_shaderSphere->setUniform3fv("CameraPosition", glm::vec3());
        }
        glDrawArrays(GL_POINTS, rangeStart, rangeEnd);
        d_shaderSphere->End();
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        renderedSprites = rangeEnd;
    } else if ( vBuffer->bufferType == RType_CloudVerts ) {
        d_shaderCloud->Begin();
        if ( render3D == false ) {  // Pass 2D matrices to shader
            d_shaderCloud->setUniformM4fv("View", glm::mat4());
            d_shaderCloud->setUniformM4fv("Projection", mvp2D);
            d_shaderCloud->setUniformM3fv("NormalMatrix", glm::mat3());
            d_shaderCloud->setUniform3fv("CameraPosition", glm::vec3());
        }
        glDrawArrays(GL_POINTS, rangeStart, rangeEnd);
        d_shaderCloud->End();
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        renderedSprites = rangeEnd;
    }
    
    if ( tex != NULL ) {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glBindVertexArray(0);
}

void RendererGLProg::RenderInstancedBuffer( VertexBuffer* vBuffer, const std::vector<InstanceData>& instances,
                                           const unsigned int rangeEnd, const unsigned int rangeStart,
                                           const Texture* tex, const bool render3D ) {
    if ( vBuffer == NULL ) return;
    if ( vBuffer->numTVerts == 0 && vBuffer->numVerts == 0 )  return;
    if ( instances.size() == 0 ) { return; };
    if ( vBuffer->GetID() == -1 ) { // Vertex buffer not initialized in hardware yet
        GLuint newVBO = 0;
        glGenBuffers(1, &newVBO);
        vBuffer->SetID(newVBO);
        vertexBuffers.push_back(vBuffer);   // Add it to our list in case the renderer needs to close
    }
    // STAGE 1 - PREPARE BUFFER AND POINTERS, UPLOAD DATA IF UPDATED
    glBindVertexArray(instancing_vao);

    if ( vBuffer->bufferType == RType_NormalVerts ) {
        glBindBuffer(GL_ARRAY_BUFFER, vBuffer->GetID());
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(NormalVertexData), 0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(NormalVertexData), (GLvoid*)(4*sizeof(GLfloat)));
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(NormalVertexData), (GLvoid*)(8*sizeof(GLfloat)));
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(NormalVertexData), (GLvoid*)(9*sizeof(GLfloat)));
        if ( vBuffer->updated ) {   // Upload vertex data to GPU
            glBufferData(GL_ARRAY_BUFFER, (vBuffer->numVerts+vBuffer->numTVerts)*sizeof(NormalVertexData), vBuffer->n_verts, GL_STATIC_DRAW);
            vBuffer->updated = false;
        }
        // Upload instance data
        glBindBuffer(GL_ARRAY_BUFFER, instancing_vbo);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), 0);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (GLvoid*)(3*sizeof(GLfloat)));
        glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (GLvoid*)(7*sizeof(GLfloat)));
        glEnableVertexAttribArray(4);
        glEnableVertexAttribArray(5);
        glEnableVertexAttribArray(6);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);
        glBufferData(GL_ARRAY_BUFFER, sizeof(InstanceData)*instances.size(), &instances[0], GL_STATIC_DRAW);
    } else if ( vBuffer->bufferType == RType_ColorVerts ||
               vBuffer->bufferType == RType_SphereVerts ||
               vBuffer->bufferType == RType_CloudVerts ||
               vBuffer->bufferType == RType_SpriteVerts ) {
        glBindBuffer(GL_ARRAY_BUFFER, vBuffer->GetID());
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), 0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (GLvoid*)(4*sizeof(GLfloat)));
        if ( vBuffer->updated ) {   // Upload vertex data to GPU
            glBufferData(GL_ARRAY_BUFFER, (vBuffer->numVerts+vBuffer->numTVerts)*sizeof(ColorVertexData), vBuffer->c_verts, GL_STATIC_DRAW);
            vBuffer->updated = false;
        }
        if ( tex != NULL ) {    // Bind texture if necessary
            glActiveTexture(GL_TEXTURE0);
            tex->Bind();
        }
    }
    // Optionally render shadows here
    if ( g_options->GetOptionDataPtr<bool>("r_shadows") && render3D ) {
        GLint m_oldFrameBuffer=0;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &m_oldFrameBuffer);
        RenderInstancedBufferShadows(vBuffer, instances, rangeEnd, rangeStart );
        glBindFramebuffer(GL_FRAMEBUFFER, m_oldFrameBuffer);
        glViewport(0, 0, windowWidth, windowHeight);
    }
    // STAGE 2 - RENDER BUFFER CONTENTS
    if ( vBuffer->bufferType == RType_NormalVerts ) {
        d_shaderInstance->Begin();
        if ( render3D == false ) { d_shaderInstance->setUniformM4fv("MVP", mvp2D); }
        else { d_shaderInstance->setUniformM4fv("MVP", mvp3D); }
        glDrawArraysInstanced(GL_TRIANGLES, rangeStart, rangeEnd, (GLsizei)instances.size());
        d_shaderInstance->End();
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        renderedTris += instances.size()*(rangeEnd/3);
    } else if ( vBuffer->bufferType == RType_SphereVerts ) {
        d_shaderSphere->Begin();
        if ( render3D == false ) {
            d_shaderSphere->setUniformM4fv("View", glm::mat4());
            d_shaderSphere->setUniformM4fv("Projection", mvp2D);
            d_shaderSphere->setUniformM3fv("NormalMatrix", glm::mat3());
            d_shaderSphere->setUniform3fv("CameraPosition", glm::vec3());
        }
        glDrawArrays(GL_POINTS, rangeStart, rangeEnd);
        d_shaderSphere->End();
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        renderedSprites = rangeEnd;
    }
    if ( tex != NULL ) {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glBindVertexArray(0);
}

void RendererGLProg::RenderSkyBuffer( VertexBuffer *vBuffer, glm::vec3 sunPos ) {
    if ( vBuffer == NULL ) return;
    if ( vBuffer->numVerts == 0 )  return;
    if ( vBuffer->GetID() == -1 ) { // Vertex buffer not initialized in hardware yet
        GLuint newVBO = 0;
        glGenBuffers(1, &newVBO);
        vBuffer->SetID(newVBO);
        vertexBuffers.push_back(vBuffer);   // Add it to our list in case the renderer needs to close
    }
    if ( vBuffer->bufferType == RType_SkyVerts ) {
        glm::mat4 skyMVP;
        skyMVP = glm::translate(mvp3D, g_camera->position-glm::vec3(0,10.0f,0));
        glBindVertexArray(vertex_vao);
        glBindBuffer(GL_ARRAY_BUFFER, vBuffer->GetID());
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
        if ( vBuffer->updated ) {   // Upload vertex data to GPU
            glBufferData(GL_ARRAY_BUFFER, (vBuffer->numVerts)*sizeof(glm::vec4), vBuffer->p_verts, GL_STATIC_DRAW);
            vBuffer->updated = false;
        }
        d_shaderSky->Begin();
        d_shaderSky->setUniformM4fv("MVP", skyMVP);
        d_shaderSky->setUniform3fv("v3LightPos", sunPos);
        glDrawArrays(GL_TRIANGLES, 0, vBuffer->numVerts);
        d_shaderSky->End();
        renderedTris += (vBuffer->numVerts/3);
        glBindVertexArray(0);
    }
}
//  Ground plane should be is a disc of triangles
//  Center point is below camera, edges are at horizon
//  ____
// |\  /|
// | \/ |
// | /\ |
// |/__\|

void RendererGLProg::RenderGroundPlane( Color& horizonColor ) {
    float horizonHeight = g_camera->position.y+(2.0f);
    float centerHeight = g_camera->position.y-(g_camera->nearDepth*2.0f);

    int segs = 16;
    // Set coefficient for each triangle fan
    const float coef = (float)(2.0f * (M_PI/segs));
    int numVerts = (segs)+2;
    const float radius = g_camera->farDepth;
    const glm::vec3 center = g_camera->position;
    NormalVertexData verts[numVerts];
    verts[0]  = {
        g_camera->position.x, centerHeight, g_camera->position.z, 1.0f,
        horizonColor.r, horizonColor.g, horizonColor.b, 1.0f, 0.0f
        , 0.0f, 1.0f, 0.0f
    }; //Center

    // Loop through each segment and store the vert and color
    for( int i = 0;i <= segs; i++ ) {
        float rads = (i)*coef;
        verts[(i+1)] = { (radius * sinf(rads))+center.x, horizonHeight, (radius * cosf(rads))+center.z, 1.0f,
            horizonColor.r, horizonColor.g, horizonColor.b, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f };
    }
    
    
    d_shaderMesh->Begin();
    d_shaderMesh->setUniformM4fv("MVP", mvp3D);
    glBindVertexArray(normalVerts_vao);
    glBindBuffer(GL_ARRAY_BUFFER, normalVerts_vbo);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(NormalVertexData), 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(NormalVertexData), (GLvoid*)(4*sizeof(GLfloat)));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(NormalVertexData), (GLvoid*)(8*sizeof(GLfloat)));
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(NormalVertexData), (GLvoid*)(9*sizeof(GLfloat)));
    glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(NormalVertexData), verts, GL_STATIC_DRAW);
    glDrawArrays(GL_TRIANGLE_FAN, 0, numVerts);
    glBindVertexArray(0);
    d_shaderMesh->End();
    renderedTris += 4;
}
void RendererGLProg::RenderVertBufferShadows( VertexBuffer* vBuffer,
                             const unsigned int rangeEnd, const unsigned int rangeStart ) {
    std::vector<Light3D*>& lights = g_lights3D->GetLights();
    for ( int i=0; i < lights.size(); i++ ) {  // Render to shadow buffer
        Light3D& light = *lights[i];
        if ( !light.active ) continue;
        if ( !light.shadowCaster ) continue;
        if ( light.lightType == Light3D_Point ) {
            glm::vec3 lightPos = glm::vec3(light.position.x,light.position.y,light.position.z);
            glm::mat4 lightModelMatrix = glm::translate(glm::mat4(1.0f), -lightPos);
            glm::mat4 lightProjectionMatrix = glm::perspective(90.0f, 1.0f, 0.1f, light.position.w);
            glBindFramebuffer(GL_FRAMEBUFFER, shadow_cube_fbo);
            glViewport(0, 0, SHADOW_CUBE_SIZE, SHADOW_CUBE_SIZE);

            for (int side=0; side<6; side++) {
                glm::mat4 lightMVP = lightProjectionMatrix * lightViewMatrix[side] * lightModelMatrix;
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, shadow_cubeMap, 0);

                if ( vBuffer->bufferType == RType_NormalVerts ) {
                    d_shaderShadowMapMesh->Begin();
                    d_shaderShadowMapMesh->setUniformM4fv("MVP", lightMVP);
                    glDrawArrays(GL_TRIANGLES, rangeStart, rangeEnd);
                    d_shaderShadowMapMesh->End();
                } else if ( vBuffer->bufferType == RType_SpriteVerts ) {
                    // TODO:: Add billboard shadow rendering
                } else if ( vBuffer->bufferType == RType_SphereVerts ) {
                    d_shaderShadowMapSphere->Begin();
                    d_shaderSphere->setUniformM4fv("View", lightViewMatrix[side]);
                    d_shaderSphere->setUniformM4fv("Projection", lightProjectionMatrix);
                    f_shaderSprite->setUniform3fv("CameraPosition", g_camera->position);
                    glDrawArrays(GL_POINTS, rangeStart, rangeEnd);
                    d_shaderShadowMapSphere->End();
                } else if ( vBuffer->bufferType == RType_CloudVerts ) {
                    d_shaderShadowMapSphere->Begin();
                    d_shaderSphere->setUniformM4fv("View", lightViewMatrix[side]);
                    d_shaderSphere->setUniformM4fv("Projection", lightProjectionMatrix);
                    f_shaderSprite->setUniform3fv("CameraPosition", g_camera->position);
                    glDrawArrays(GL_POINTS, rangeStart, rangeEnd);
                    d_shaderShadowMapSphere->End();
                }
            }
        } else {    // Not point light
            glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
            glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
            if ( vBuffer->bufferType == RType_NormalVerts ) {
                glm::mat4 lightMVP = GetLightMVP(*lights[i]);
                d_shaderShadowMapMesh->Begin();
                d_shaderShadowMapMesh->setUniformM4fv("MVP", lightMVP);
                glDrawArrays(GL_TRIANGLES, rangeStart, rangeEnd);
                d_shaderShadowMapMesh->End();
            } else if ( vBuffer->bufferType == RType_SpriteVerts ) {
                // TODO:: Add billboard shadow rendering
            } else if ( vBuffer->bufferType == RType_SphereVerts ) {
                glm::mat4 lightView = GetLightView(*lights[i]);
                glm::mat4 lightProjection = GetLightProjection(*lights[0]);
                d_shaderShadowMapSphere->Begin();
                d_shaderSphere->setUniformM4fv("View", lightView);
                d_shaderSphere->setUniformM4fv("Projection", lightProjection);
                f_shaderSprite->setUniform3fv("CameraPosition", g_camera->position);
                glDrawArrays(GL_POINTS, rangeStart, rangeEnd);
                d_shaderShadowMapSphere->End();
            } else if ( vBuffer->bufferType == RType_CloudVerts ) {
                glm::mat4 lightView = GetLightView(*lights[i]);
                glm::mat4 lightProjection = GetLightProjection(*lights[0]);
                d_shaderShadowMapSphere->Begin();
                d_shaderSphere->setUniformM4fv("View", lightView);
                d_shaderSphere->setUniformM4fv("Projection", lightProjection);
                f_shaderSprite->setUniform3fv("CameraPosition", g_camera->position);
                glDrawArrays(GL_POINTS, rangeStart, rangeEnd);
                d_shaderShadowMapSphere->End();
            }
        }
    }
}
void RendererGLProg::RenderInstancedBufferShadows( VertexBuffer* vBuffer, const std::vector<InstanceData>& instances,
                                  const unsigned int rangeEnd, const unsigned int rangeStart ) {
    std::vector<Light3D*>& lights = g_lights3D->GetLights();
    for ( int i=0; i < lights.size(); i++ ) {  // Render to shadow buffer
        Light3D& light = *lights[i];
        if ( !light.active ) continue;
        if ( !light.shadowCaster ) continue;
        if ( light.lightType == Light3D_Point ) {
            glm::vec3 lightPos = glm::vec3(light.position.x,light.position.y,light.position.z);
            glm::mat4 lightModelMatrix = glm::translate(glm::mat4(1.0f), -lightPos);
            glm::mat4 lightProjectionMatrix = glm::perspective(90.0f, 1.0f, 0.1f, light.position.w);
                        glBindFramebuffer(GL_FRAMEBUFFER, shadow_cube_fbo);
            glViewport(0, 0, SHADOW_CUBE_SIZE, SHADOW_CUBE_SIZE);
            
            for (int side=0; side<6; side++) {
                glm::mat4 lightMVP = lightProjectionMatrix * lightViewMatrix[side] * lightModelMatrix;
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, shadow_cubeMap, 0);
                if ( vBuffer->bufferType == RType_NormalVerts ) {
                    d_shaderShadowMapObject->Begin();
                    d_shaderShadowMapObject->setUniformM4fv("MVP", lightMVP);
                    glDrawArraysInstanced(GL_TRIANGLES, rangeStart, rangeEnd, (GLsizei)instances.size());
                    d_shaderShadowMapObject->End();
                } else if ( vBuffer->bufferType == RType_SphereVerts ) {
                    d_shaderShadowMapSphere->Begin();
                    d_shaderSphere->setUniformM4fv("View", lightViewMatrix[side]);
                    d_shaderSphere->setUniformM4fv("Projection", lightProjectionMatrix);
                    f_shaderSprite->setUniform3fv("CameraPosition", g_camera->position);
                    glDrawArrays(GL_POINTS, rangeStart, rangeEnd);
                    d_shaderShadowMapSphere->End();
                }
            }
        } else {
            glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
            if ( vBuffer->bufferType == RType_NormalVerts ) {
                glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
                glm::mat4 lightMVP = GetLightMVP(*lights[i]);
                glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
                d_shaderShadowMapObject->Begin();
                d_shaderShadowMapObject->setUniformM4fv("MVP", lightMVP);
                glDrawArraysInstanced(GL_TRIANGLES, rangeStart, rangeEnd, (GLsizei)instances.size());
                d_shaderShadowMapObject->End();
            } else if ( vBuffer->bufferType == RType_SphereVerts ) {
                glm::mat4 lightView = GetLightView(*lights[i]);
                glm::mat4 lightProjection = GetLightProjection(*lights[i]);
                glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
                d_shaderShadowMapSphere->Begin();
                d_shaderSphere->setUniformM4fv("View", lightView);
                d_shaderSphere->setUniformM4fv("Projection", lightProjection);
                f_shaderSprite->setUniform3fv("CameraPosition", g_camera->position);
                glDrawArrays(GL_POINTS, rangeStart, rangeEnd);
                d_shaderShadowMapSphere->End();
            }
        }
//        glBindFramebuffer(GL_FRAMEBUFFER, render_fbo);  // Switch back to G-Buffer rendering
//        glViewport(0, 0, windowWidth, windowHeight);
    }
}
void RendererGLProg::BufferVerts( const ColorVertexData* verts, const int numVerts ) {
    if ( numVerts > 0 ) {
        if ( numVerts+numBufferedColorVerts > MAX_BUFFERED_VERTS ) {
            RenderVerts();
        }
    }
    memcpy( &vertBufferColor[numBufferedColorVerts], verts, numVerts*sizeof(ColorVertexData));
    numBufferedColorVerts += numVerts;
}
void RendererGLProg::BufferVerts( const NormalVertexData* verts, const int numVerts ) {
    if ( numVerts > 0 ) {
        if ( numVerts+numBufferedNormalVerts > MAX_BUFFERED_VERTS ) {
            RenderVerts();
        }
    }
    memcpy( &vertBufferNormal[numBufferedNormalVerts], verts, numVerts*sizeof(NormalVertexData));
    numBufferedNormalVerts += numVerts;
}
void RendererGLProg::BufferSpheres( const ColorVertexData *spheres, const int numSpheres ) {
    if ( numSpheres > 0 ) {
        if ( numSpheres+numBufferedSprites > MAX_BUFFERED_VERTS ) {
            RenderSpheres();
        }
    }
    memcpy( &spriteBuffer[numBufferedSprites], spheres, numSpheres*sizeof(ColorVertexData));
    numBufferedSprites += numSpheres;

}
void RendererGLProg::RenderVerts() {
    if ( numBufferedColorVerts ) {
        d_shaderDefault_vColor->Begin();
        d_shaderDefault_vColor->setUniformM4fv("MVP", mvp3D);
        glBindVertexArray(colorVerts_vao);
        glBindBuffer(GL_ARRAY_BUFFER, colorVerts_vbo);
        glBufferData(GL_ARRAY_BUFFER, numBufferedColorVerts * sizeof(ColorVertexData), vertBufferColor, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, numBufferedColorVerts);
        glBindVertexArray(0);
        d_shaderDefault_vColor->End();
        renderedTris += numBufferedColorVerts/3;
        numBufferedColorVerts = 0;
    }
    if ( numBufferedNormalVerts ) {
        d_shaderMesh->Begin();
        d_shaderMesh->setUniformM4fv("MVP", mvp3D);
        glBindVertexArray(normalVerts_vao);
        glBindBuffer(GL_ARRAY_BUFFER, normalVerts_vbo);
        glBufferData(GL_ARRAY_BUFFER, numBufferedNormalVerts * sizeof(NormalVertexData), vertBufferNormal, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, numBufferedNormalVerts);
        glBindVertexArray(0);
        d_shaderMesh->End();
        renderedTris += numBufferedNormalVerts/3;
        numBufferedNormalVerts = 0;
    }
}
void RendererGLProg::RenderSpheres() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, Stencil_Solid, 0xFF);
    if ( numBufferedSprites ) {
        d_shaderSphere->Begin();
        glBindVertexArray(sprite_vao);
        glBindBuffer(GL_ARRAY_BUFFER, sprite_vbo);
        glBufferData(GL_ARRAY_BUFFER, numBufferedSprites * sizeof(ColorVertexData), spriteBuffer, GL_STATIC_DRAW);
        glDrawArrays(GL_POINTS, 0, numBufferedSprites);
        d_shaderSphere->End();
        glBindVertexArray(0);
        renderedSprites += numBufferedSprites;
        numBufferedSprites = 0;
    }
    glDisable(GL_STENCIL_TEST);
}
//========================================================================
// 2D Colored shape rendering functions
//========================================================================
void RendererGLProg::Buffer2DLine( const glm::vec2 a, const glm::vec2 b,
                                const Color aColor, const Color bColor, float z ) {
    lineBuffer2D[numBuffered2DLines++] = {
        a.x,a.y,z,1.0f,
        aColor.r,aColor.g,aColor.b,aColor.a,
         };
    lineBuffer2D[numBuffered2DLines++] = {
        b.x,b.y,z,1.0f,
        bColor.r,bColor.g,bColor.b,bColor.a,
        };
    if ( numBuffered2DLines >= MAX_BUFFERED_LINES*2 ) Render2DLines();
}
void RendererGLProg::Render2DLines() {
    if ( numBuffered2DLines == 0 ) return;
//    glDisable(GL_BLEND);
//    glEnable(GL_DEPTH_TEST);

    d_shaderDefault_vColor->Begin();
    d_shaderDefault_vColor->setUniformM4fv("MVP", mvp2D);

    glBindVertexArray(lines_vao);
    glBindBuffer(GL_ARRAY_BUFFER, lines_vbo);
    // Buffer instance data to GPU
    glBufferData(GL_ARRAY_BUFFER, sizeof(ColorVertexData)*numBuffered2DLines, lineBuffer2D, GL_STATIC_DRAW);
    glDrawArrays(GL_LINES, 0, numBuffered2DLines);
    d_shaderDefault_vColor->End();
    glBindVertexArray(0);
    renderedSegs += numBuffered2DLines/2;
    numBuffered2DLines = 0;
}

void RendererGLProg::DrawPolygon( const int count, const GLfloat *verts,
                                 const Color lineColor, const Color fillColor, const float z ) {
    f_shaderUI_color->Begin();
    f_shaderUI_color->setUniformM4fv("MVP", mvp2D);
    // Bind buffer and upload verts
    glBindVertexArray(vertex_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
    glBufferData(GL_ARRAY_BUFFER, 4*count*sizeof(GLfloat), verts, GL_STATIC_DRAW);
    // Render polygon to screen
    if ( fillColor.a > 0.0f ) {
        f_shaderUI_color->setUniform4fv("u_color", fillColor);
        glDrawArrays(GL_TRIANGLE_FAN, 0, count);
        renderedTris += count-2;
    }
    if( lineColor.a > 0.0f ) {
        f_shaderUI_color->setUniform4fv("u_color", lineColor);
        glDrawArrays(GL_LINE_LOOP, 0, count);
        renderedSegs += count;
    }
    f_shaderUI_color->End();
    glBindVertexArray(0);
}
void RendererGLProg::DrawPolygon( const int count, const glm::vec2 *verts,
                                 const Color lineColor, const Color fillColor, const float z ) {
    f_shaderUI_color->Begin();
    f_shaderUI_color->setUniformM4fv("MVP", mvp2D);
    // Bind buffer and upload verts
    glBindVertexArray(vertex_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
    glm::vec4* vertices = new glm::vec4[count];
	if (!vertices) {
		printf("DrawPolygon out of memory");
	}
    for (int i=0; i<count; i++) {
        vertices[i].x = verts[i].x;
        vertices[i].y = verts[i].y;
        vertices[i].z = z;
        vertices[i].w = 1.0f;
    }
    glBufferData(GL_ARRAY_BUFFER, 4*count*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    // Render polygon to screen
    if ( fillColor.a > 0.0f ) {
        f_shaderUI_color->setUniform4fv("u_color", fillColor);
        glDrawArrays(GL_TRIANGLE_FAN, 0, count);
        renderedTris += count-2;
    }
    if( lineColor.a > 0.0f ) {
        f_shaderUI_color->setUniform4fv("u_color", lineColor);
        glDrawArrays(GL_LINE_LOOP, 0, count);
        renderedSegs += count;
    }
    f_shaderUI_color->End();
    glBindVertexArray(0);
    delete [] vertices;
}
void RendererGLProg::Draw2DRect( Rect2D rect,
                               Color lineColor, Color fillColor, float z ) {
    GLfloat verts[] = {
        rect.x          , rect.y,           z, 1,
        rect.x + rect.w , rect.y,           z, 1,
        rect.x + rect.w , rect.y + rect.h,  z, 1,
        rect.x          , rect.y + rect.h,  z, 1,
    };
    DrawPolygon(4, verts, lineColor, fillColor, 0.0f);
}

void RendererGLProg::Draw2DRect( glm::vec2 center, float width, float height,
                               Color lineColor, Color fillColor, float z ) {
    GLfloat hw = (GLfloat)(width*0.5f);
    GLfloat hh = (GLfloat)(height*0.5f);
    GLfloat verts[] = {
        -hw + center.x, -hh + center.y, z, 1,
        +hw + center.x, -hh + center.y, z, 1,
        +hw + center.x, +hh + center.y, z, 1,
        -hw + center.x, +hh + center.y, z, 1,
    };
    DrawPolygon(4, verts, lineColor, fillColor, z);
}
void RendererGLProg::Draw2DRect3D( glm::vec3 center, float width, float height,
                                   Color lineColor, Color fillColor, float z ) {
    
    glm::mat4 model = glm::mat4();
    model = glm::rotate(model, -g_camera->rotation.x, glm::vec3(1.0, 0.0, 0.0));
    model = glm::rotate(model, -g_camera->rotation.y, glm::vec3(0.0, 1.0, 0.0));
    model = glm::rotate(model, -g_camera->rotation.z, glm::vec3(0.0, 0.0, 1.0));
    glm::mat4 proj = glm::mat4();
    GLfloat aspectRatio = (windowWidth > windowHeight) ?
    GLfloat(windowWidth)/GLfloat(windowHeight) : GLfloat(windowHeight)/GLfloat(windowWidth);
    proj = glm::perspective(g_camera->fieldOfView, aspectRatio, g_camera->nearDepth, g_camera->farDepth);
    int ww2 = windowWidth/2;
    int wh2 = windowHeight/2;
    glm::vec4 viewport = glm::vec4(0, 0, windowWidth, windowHeight);
    //  Calculate screen position of rectangle
    glm::vec3 scrnPos = glm::project(center, model, proj, viewport);
    scrnPos += glm::vec3(-ww2,-wh2,0.0f);
    Draw2DRect(glm::vec2(scrnPos.x,scrnPos.y), width, height, lineColor, fillColor, scrnPos.z);
}
void RendererGLProg::Draw2DProgressBar(glm::vec3 center, float width, float height, float amount,
                                     Color lineColor, Color fillColor, float z) {
    
    glm::mat4 model = glm::mat4();
    model = glm::rotate(model, -g_camera->rotation.x, glm::vec3(1.0, 0.0, 0.0));
    model = glm::rotate(model, -g_camera->rotation.y, glm::vec3(0.0, 1.0, 0.0));
    model = glm::rotate(model, -g_camera->rotation.z, glm::vec3(0.0, 0.0, 1.0));
    model = glm::translate(model, glm::vec3(-g_camera->position.x, -g_camera->position.y, -g_camera->position.z));
    glm::mat4 proj = glm::mat4();
    GLfloat aspectRatio = (windowWidth > windowHeight) ?
    GLfloat(windowWidth)/GLfloat(windowHeight) : GLfloat(windowHeight)/GLfloat(windowWidth);
    proj = glm::perspective(g_camera->fieldOfView, aspectRatio, g_camera->nearDepth, g_camera->farDepth);
    int ww2 = windowWidth/2;
    int wh2 = windowHeight/2;
    glm::vec4 viewport = glm::vec4(0, 0, windowWidth, windowHeight);
    //  Calculate screen position of rectangle
    glm::vec3 scrnPos = glm::project(center, model, proj, viewport);
    scrnPos += glm::vec3(-ww2,-wh2,0.0f);
    float barWidth = width*amount;
    Draw2DRect(glm::vec2(scrnPos.x,scrnPos.y), width, height, lineColor, COLOR_NONE, scrnPos.z);    // Outline
    Draw2DRect(glm::vec2(scrnPos.x-(width*0.5f)+(barWidth*0.5f),scrnPos.y), barWidth, height, COLOR_NONE, fillColor, scrnPos.z);     // Bar
}
void RendererGLProg::DrawGradientY( Rect2D rect, Color topColor, Color btmColor, float z ) {
    ColorVertexData vertices[] = {
        rect.x          , rect.y,           z, 1.0f,
        btmColor.r, btmColor.g, btmColor.b, btmColor.a,
        rect.x + rect.w , rect.y,           z, 1.0f,
        btmColor.r, btmColor.g, btmColor.b, btmColor.a,
        rect.x + rect.w , rect.y + rect.h,  z, 1.0f,
        topColor.r, topColor.g, topColor.b, topColor.a,
        rect.x          , rect.y + rect.h,  z, 1.0f,
        topColor.r, topColor.g, topColor.b, topColor.a,
    };
    f_shaderUI_vColor->Begin();
    f_shaderUI_vColor->setUniformM4fv("MVP", mvp2D);
    glBindVertexArray(colorVerts_vao);
    glBindBuffer(GL_ARRAY_BUFFER, colorVerts_vbo);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(ColorVertexData), vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    f_shaderUI_vColor->End();
    glBindVertexArray(0);
    renderedTris += 2;
}
void RendererGLProg::DrawGradientX( Rect2D rect, Color leftColor, Color rightColor, float z ) {
    ColorVertexData vertices[4] = {
        rect.x          , rect.y,           z, 1,
        leftColor.r, leftColor.g, leftColor.b, leftColor.a,
        rect.x + rect.w , rect.y,           z, 1,
        rightColor.r, rightColor.g, rightColor.b, rightColor.a,
        rect.x + rect.w , rect.y + rect.h,  z, 1,
        rightColor.r, rightColor.g, rightColor.b, rightColor.a,
        rect.x          , rect.y + rect.h,  z, 1,
        leftColor.r, leftColor.g, leftColor.b, leftColor.a,
    };
    f_shaderUI_vColor->Begin();
    f_shaderUI_vColor->setUniformM4fv("MVP", mvp2D);
    glBindVertexArray(colorVerts_vao);
    glBindBuffer(GL_ARRAY_BUFFER, colorVerts_vbo);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(ColorVertexData), vertices, GL_STATIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 4);
    f_shaderUI_vColor->End();
    glBindVertexArray(0);
    renderedTris += 2;
}

void RendererGLProg::DrawCircle( glm::vec2 center, float angle, float radius,
                                Color lineColor, Color fillColor, float z, const int pixelsPerSeg ) {
    int segs = radius / pixelsPerSeg;
    if ( segs < 8 ) segs = 8;
    // Set coefficient for each triangle fan
    const float coef = (float)(2.0f * (M_PI/segs));
    int numVerts = (segs)+2;
    // Create buffers for verts and colors
    GLfloat* vertices = new GLfloat[4*numVerts];
    if( ! vertices ) return;
    vertices[0] = center.x; vertices[1] = center.y;
    vertices[2] = z;        vertices[3] = 1.0f;
    // Loop through each segment and store the vert and color
    for( int i = 0;i <= segs; i++ ) {
        float rads = (i)*coef;
        vertices[(i+1)*4] = (GLfloat)(radius * cosf(rads))+center.x;    // X
        vertices[(i+1)*4+1] = (GLfloat)(radius * sinf(rads))+center.y;  // Y
        vertices[(i+1)*4+2] = z;
        vertices[(i+1)*4+3] = 1;
    }
    f_shaderUI_color->Begin();
    f_shaderUI_color->setUniformM4fv("MVP", mvp2D);
    // Bind buffer and upload verts
    glBindVertexArray(vertex_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
    glBufferData(GL_ARRAY_BUFFER, 4*numVerts*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    // Render polygon to screen
    if ( fillColor.a > 0.0f ) {
        f_shaderUI_color->setUniform4fv("u_color", fillColor);
        glDrawArrays(GL_TRIANGLE_FAN, 0, numVerts);
        renderedTris += (numVerts-2);
    }
    if( lineColor.a > 0.0f ) {
        // Outer line
        f_shaderUI_color->setUniform4fv("u_color", lineColor);
        glDrawArrays(GL_LINE_LOOP, 1, numVerts-1);
        renderedSegs += numVerts-1;
    }
    f_shaderUI_color->End();
    glBindVertexArray(0);
    // Reset OpenGL client state
    delete vertices;
}
void RendererGLProg::DrawRing( glm::vec2 center, float radius1, float radius2, int segs,
                              Color lineColor, Color fillColor, float z ) {
    // Set coefficient for each triangle fan
    const float coef = (float)(2.0f * (M_PI/segs));
    int numVerts = (2*segs)+2;
    // Create buffers for verts and colors
    GLfloat* vertices = new GLfloat[4*numVerts];
    if( ! vertices ) return;
    // Loop through each segment and store the vert and color
    for( int i = 0;i <= segs; i++ ) {
        float rads = (i)*coef;
        // Ring outer vertice
        vertices[(i*8)]   = (GLfloat)(radius1 * sinf(rads))+center.x;
        vertices[(i*8)+1] = (GLfloat)(radius1 * cosf(rads))+center.y;
        vertices[(i*8)+2] = z;
        vertices[(i*8)+3] = 1.0f;
        // Ring inner vertice
        GLfloat l = (GLfloat)(radius2 * sinf(rads))+center.x;
        GLfloat m = (GLfloat)(radius2 * cosf(rads))+center.y;
        vertices[(i*8)+4] = l;
        vertices[(i*8)+5] = m;
        vertices[(i*8)+6] = z;
        vertices[(i*8)+7] = 1.0f;
    }
    f_shaderUI_color->Begin();
    f_shaderUI_color->setUniformM4fv("MVP", mvp2D);
    // Bind buffer and upload verts
    glBindVertexArray(vertex_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
    glBufferData(GL_ARRAY_BUFFER, 4*(numVerts)*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    // Render polygon to screen
    if ( fillColor.a > 0.0f ) {
        f_shaderUI_color->setUniform4fv("u_color", fillColor);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, numVerts);
        renderedTris += (numVerts-2);
    }
    if( lineColor.a > 0.0f ) {
        int halfVerts = (int)(numVerts * 0.5);
        // Outer line TODO: SKIP VERTICES
        glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), 0 );
        f_shaderUI_color->setUniform4fv("u_color", lineColor);
        glDrawArrays(GL_LINE_LOOP, 0, halfVerts);
        // Inner line
        glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (GLvoid*)(4*sizeof(GLfloat)) );
        glDrawArrays(GL_LINE_LOOP, 0, halfVerts);
        renderedSegs += numVerts;
        glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 0, 0 );
    }
    f_shaderUI_color->End();
    glBindVertexArray(0);
    delete[] vertices;
}
void RendererGLProg::DrawGrid( float gridSize, Rect2D rect, int subDivisions, Color color ) {
    int horzLines = (int)(rect.w/gridSize) + 1;
    int vertLines = (int)(rect.h/gridSize) + 1;
    int clampedWidth = (int)(horzLines*gridSize);
    int clampedHeight = (int)(vertLines*gridSize);
    if ( subDivisions ) {
        float subGridSize = gridSize/subDivisions;
        int horzSubLines = (int)(rect.w/subGridSize) + 1;
        int vertSubLines = (int)(rect.h/subGridSize) + 1;
        int clampedSubWidth = (int)(horzSubLines*subGridSize);
        int clampedSubHeight = (int)(vertSubLines*subGridSize);
        for (int i = 0; i < vertSubLines; i++) {
            glm::vec2 a = glm::vec2(rect.x, rect.y+(i*subGridSize));
            glm::vec2 b = glm::vec2(rect.x+clampedSubWidth, rect.y+(i*subGridSize));
            Buffer2DLine(a, b, color, color);
        }
        for (int i = 0; i < horzSubLines; i++) {
            glm::vec2 a = glm::vec2(rect.x+(i*subGridSize), rect.y);
            glm::vec2 b = glm::vec2(rect.x+(i*subGridSize), rect.y+clampedSubHeight);
            Buffer2DLine(a, b, color, color);
        }
    }
    for (int i = 0; i <= vertLines; i++) {
        glm::vec2 a = glm::vec2(rect.x-1, rect.y+(i*gridSize));
        glm::vec2 b = glm::vec2(rect.x+clampedWidth, rect.y+(i*gridSize));
        Buffer2DLine(a, b, color, color);
    }
    for (int i = 0; i <= horzLines; i++) {
        glm::vec2 a = glm::vec2(rect.x+(i*gridSize), rect.y);
        glm::vec2 b = glm::vec2(rect.x+(i*gridSize), rect.y+clampedHeight);
        Buffer2DLine(a, b, color, color);
    }
    Render2DLines();
}
void RendererGLProg::Draw3DGrid(const glm::vec3& pos, const float size, const int divisions) {
	// ----------   GRID BUFFER --------- //
	struct point {
		GLfloat x;
		GLfloat y;
		GLfloat z;
		GLfloat w;
	};

	int gridSize = divisions + 1;
	int gridHalf = gridSize / 2;
    const int numVertices = gridSize*gridSize;
	point* grid_vertices = new point[numVertices];

	for (int i = 0; i < gridSize; i++) {
		for (int j = 0; j < gridSize; j++) {
			grid_vertices[i*gridSize + j].x = (float)(j - gridHalf) / gridHalf*size;
			grid_vertices[i*gridSize + j].y = 0.0f;
			grid_vertices[i*gridSize + j].z = (float)(i - gridHalf) / gridHalf*size;
			grid_vertices[i*gridSize + j].w = 0.0f;
		}
	}
    const int numIndices = 2 * (gridSize - 1) * gridSize * 2;
	GLushort* indices = new GLushort[numIndices];
	int i = 0;
	// Horizontal grid lines
	for (int y = 0; y < gridSize; y++) {
		for (int x = 0; x < gridSize - 1; x++) {
			indices[i++] = y * gridSize + x;
			indices[i++] = y * gridSize + x + 1;
		}
	}
	// Vertical grid lines
	for (int x = 0; x < gridSize; x++) {
		for (int y = 0; y < gridSize - 1; y++) {
			indices[i++] = y * gridSize + x;
			indices[i++] = (y + 1) * gridSize + x;

		}
	}

	// Grid line rendering
	d_shaderDefault_uColor->Begin();
	glm::mat4 mvp = mvp3D;
	mvp = glm::translate(mvp, pos);
	d_shaderDefault_uColor->setUniformM4fv("MVP", mvp);
	d_shaderDefault_uColor->setUniform4fv("u_color", COLOR_WHITE);
	glBindVertexArray(vertex_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_ibo);

//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//	glDisable(GL_CULL_FACE);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBufferData(GL_ARRAY_BUFFER, sizeof(point)*numVertices, grid_vertices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*numIndices, indices, GL_STATIC_DRAW);
    glDrawElements(GL_LINES, numIndices, GL_UNSIGNED_SHORT, 0);
    d_shaderDefault_uColor->End();
    
//    glEnable(GL_CULL_FACE);
//    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    glBindVertexArray(0);
    renderedSegs += numIndices/2;
    
	delete [] grid_vertices;
	grid_vertices = NULL;
	delete [] indices;
	indices = NULL;
}

//========================================================================
// 2D Textured shape rendering functions
//========================================================================
void RendererGLProg::DrawSprite( const Sprite& sprite ) {
    glActiveTexture(GL_TEXTURE0);
    if ( sprite.batched ) {
        if ( sprite.spriteBatch && sprite.spriteBatch->texture ) {
            sprite.spriteBatch->texture->Bind();
        } else {    // Couldn't bind texture, fail
            printf("[RendererGLProg] Error drawing sprite, no batch or texture\n");
            return;
        }
    } else {
        sprite.texture->Bind();
    }

    GLfloat halfWidth = (GLfloat)(sprite.rect.w)*0.5f;
    GLfloat halfHeight = (GLfloat)(sprite.rect.h)*0.5f;

    GLfloat posX = (GLfloat)(sprite.rect.x);
    GLfloat posY = (GLfloat)(sprite.rect.y);
    GLfloat posZ = (GLfloat)(sprite.z);

    const GLfloat vertices[] = {
        -halfWidth, -halfHeight, 0.0f, 1.0f,
         halfWidth, -halfHeight, 0.0f, 1.0f,
         halfWidth,  halfHeight, 0.0f, 1.0f,
        -halfWidth,  halfHeight, 0.0f, 1.0f,
    };

    const GLfloat texCoords[] = {
        sprite.texRect.x                    , sprite.texRect.y,
        sprite.texRect.x + sprite.texRect.w , sprite.texRect.y,
        sprite.texRect.x + sprite.texRect.w , sprite.texRect.y + sprite.texRect.h,
        sprite.texRect.x                    , sprite.texRect.y + sprite.texRect.h
    };
    
    glm::vec3 center = glm::vec3(posX+(halfWidth*sprite.scale.x), posY+(halfHeight*sprite.scale.y), posZ);
    glm::mat4 mvp = glm::translate(mvp2D, center);
    mvp = glm::scale(mvp, glm::vec3(sprite.scale.x,sprite.scale.y,1.0f));
    mvp = glm::rotate(mvp, sprite.rot.x*GLfloat(180.0f/M_PI), glm::vec3(0.0, 0.0, 1.0));
    
    f_shaderUI_tex->Begin();
    f_shaderUI_tex->setUniformM4fv("MVP", mvp);
    f_shaderUI_tex->setUniform4fv("u_color", sprite.color);
    f_shaderUI_tex->setUniform1iv("textureMap", 0);
    glBindVertexArray(ui_vao);
    glBindBuffer(GL_ARRAY_BUFFER, ui_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(texCoords), NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(texCoords), texCoords);
    glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 0, 0 );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)sizeof(vertices) );
    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    f_shaderUI_tex->End();
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0 );
    renderedTris += 2;
}
void RendererGLProg::DrawSpriteBatch( const SpriteBatch& batch ) {
    glActiveTexture(GL_TEXTURE0);
    if ( batch.texture ) {
        batch.texture->Bind();
    } else {
        printf("[RendererGLProg] Error drawing sprite batch, no texture\n");
        return;
    }
    glBindVertexArray(ui_vao);
    glBindBuffer(GL_ARRAY_BUFFER, ui_vbo);
    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 0, 0 );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(sizeof(GLfloat)*16) );

    f_shaderUI_tex->Begin();
    for ( unsigned int i=0; i < batch.children.size(); i++ ) {
        Sprite& sprite = *batch.children[i];
        GLfloat halfWidth = (GLfloat)(sprite.rect.w)*0.5f;
        GLfloat halfHeight = (GLfloat)(sprite.rect.h)*0.5f;
        
        GLfloat posX = (GLfloat)(sprite.rect.x);
        GLfloat posY = (GLfloat)(sprite.rect.y);
        GLfloat posZ = (GLfloat)(sprite.z);
        
        const GLfloat vertices[] = {
            -halfWidth, -halfHeight, 0.0f, 1.0f,
            halfWidth, -halfHeight, 0.0f, 1.0f,
            halfWidth,  halfHeight, 0.0f, 1.0f,
            -halfWidth,  halfHeight, 0.0f, 1.0f,
        };
        
        const GLfloat texCoords[] = {
            sprite.texRect.x                    , sprite.texRect.y,
            sprite.texRect.x + sprite.texRect.w , sprite.texRect.y,
            sprite.texRect.x + sprite.texRect.w , sprite.texRect.y + sprite.texRect.h,
            sprite.texRect.x                    , sprite.texRect.y + sprite.texRect.h
        };
        
        glm::vec3 center = glm::vec3(posX+(halfWidth*sprite.scale.x), posY+(halfHeight*sprite.scale.y), posZ);
        glm::mat4 mvp = glm::translate(mvp2D, center);
        mvp = glm::scale(mvp, glm::vec3(sprite.scale.x,sprite.scale.y,1.0f));
        mvp = glm::rotate(mvp, sprite.rot.x*GLfloat(180.0f/M_PI), glm::vec3(0.0, 0.0, 1.0));

        f_shaderUI_tex->setUniformM4fv("MVP", mvp);
        f_shaderUI_tex->setUniform4fv("u_color", sprite.color);
        f_shaderUI_tex->setUniform1iv("textureMap", 0);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(texCoords), NULL, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(texCoords), texCoords);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
    f_shaderUI_tex->End();
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0 );
    renderedTris += 2;
}
void RendererGLProg::DrawImage( const glm::vec2 center, const float width, const float height,
                                const std::string texName, const float z, const Color color ) {
    glActiveTexture(GL_TEXTURE0);
    bool textureBound = TextureManager::Inst()->BindTexture(texName);
    if (textureBound) {
        GLfloat hw = (GLfloat)(width*0.5f);
        GLfloat hh = (GLfloat)(height*0.5f);
        GLfloat vertices[] = {
            -hw + center.x, -hh + center.y, z, 1.0f,
			+hw + center.x, -hh + center.y, z, 1.0f,
			+hw + center.x, +hh + center.y, z, 1.0f,
			-hw + center.x, +hh + center.y, z, 1.0f,
        };
        const GLfloat texCoords[] = {
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f
        };
        f_shaderUI_tex->Begin();
        f_shaderUI_tex->setUniformM4fv("MVP", mvp2D);
        f_shaderUI_tex->setUniform4fv("u_color", color);
        f_shaderUI_tex->setUniform1iv("textureMap", 0);
        glBindVertexArray(ui_vao);
        glBindBuffer(GL_ARRAY_BUFFER, ui_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(texCoords), NULL, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(texCoords), texCoords);
        glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 0, 0 );
        glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)sizeof(vertices) );
        glEnableVertexAttribArray( 0 );
        glEnableVertexAttribArray( 1 );
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        f_shaderUI_tex->End();
        glBindVertexArray(0);
        renderedTris += 2;
    }
}
void RendererGLProg::DrawTexture( const Rect2D rect, const Rect2D texRect,
                               const GLuint tex, const float z, const Color color ) {
    GLfloat vertices[] = {
        rect.x          , rect.y,           z, 1,
        rect.x + rect.w , rect.y,           z, 1,
        rect.x + rect.w , rect.y + rect.h,  z, 1,
        rect.x          , rect.y + rect.h,  z, 1,
    };
    GLfloat texCoords[8] = {
        texRect.x,           texRect.y,
        texRect.x+texRect.w, texRect.y,
        texRect.x+texRect.w, texRect.y+texRect.h,
        texRect.x,           texRect.y+texRect.h
        
    };
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    f_shaderUI_tex->Begin();
    f_shaderUI_tex->setUniformM4fv("MVP", mvp2D);
    f_shaderUI_tex->setUniform4fv("u_color", color);
    f_shaderUI_tex->setUniform1iv("textureMap", 0);
    glBindVertexArray(ui_vao);
    glBindBuffer(GL_ARRAY_BUFFER,ui_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(texCoords), NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(texCoords), texCoords);
    glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 0, 0 );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)sizeof(vertices) );
    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    f_shaderUI_tex->End();
    glBindVertexArray(0);
    renderedTris += 2;
}
void RendererGLProg::DrawTextureArray( const Rect2D rect, const Rect2D texRect,
                                      const int width, const int height,
                                      const GLbyte* data, const float z, const Color color) {
    GLfloat vertices[] = {
        rect.x          , rect.y,           z, 1,
        rect.x + rect.w , rect.y,           z, 1,
        rect.x + rect.w , rect.y + rect.h,  z, 1,
        rect.x          , rect.y + rect.h,  z, 1,
    };
    GLfloat texCoords[8] = {
        texRect.x,           texRect.y,
        texRect.x+texRect.w, texRect.y,
        texRect.x+texRect.w, texRect.y+texRect.h,
        texRect.x,           texRect.y+texRect.h
        
    };

    // Generate and bind temporary texture
    GLuint tempTexture;
    glGenTextures(1, &tempTexture);
	glBindTexture(GL_TEXTURE_2D, tempTexture);

//    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Upload texture data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
//    glBlendFunc(GL_ONE, GL_ONE);
    
    glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tempTexture);

    f_shaderUI_tex->Begin();
    f_shaderUI_tex->setUniformM4fv("MVP", mvp2D);
    f_shaderUI_tex->setUniform4fv("u_color", color);
    f_shaderUI_tex->setUniform1iv("textureMap", 0);
    glBindVertexArray(ui_vao);
    glBindBuffer(GL_ARRAY_BUFFER,ui_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(texCoords), NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(texCoords), texCoords);
    glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 0, 0 );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)sizeof(vertices) );
    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    f_shaderUI_tex->End();
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    renderedTris += 2;
    glDeleteTextures(1, &tempTexture);
}
void RendererGLProg::DrawCubeMap( const Rect2D rect, const GLfloat* texCoords,
                                 const GLuint tex, const float z, const Color color ) {
    GLfloat vertices[] = {
        rect.x          , rect.y,           z, 1,
        rect.x + rect.w , rect.y,           z, 1,
        rect.x + rect.w , rect.y + rect.h,  z, 1,
        rect.x          , rect.y + rect.h,  z, 1,
    };

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
    f_shaderUI_cubeMap->Begin();
    f_shaderUI_cubeMap->setUniformM4fv("MVP", mvp2D);
    f_shaderUI_cubeMap->setUniform4fv("u_color", color);
    f_shaderUI_cubeMap->setUniform1iv("textureMap", 0);
    glBindVertexArray(ui_vao);
    glBindBuffer(GL_ARRAY_BUFFER,ui_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(GLfloat)*12, NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(GLfloat)*12, texCoords);
    glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 0, 0 );
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)sizeof(vertices) );
    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    f_shaderUI_cubeMap->End();
    glBindVertexArray(0);
    renderedTris += 2;
}
//========================================================================
// 3D Object rendering functions
//========================================================================
void RendererGLProg::Buffer3DLine( const glm::vec3 a, const glm::vec3 b,
                                  const Color aColor, const Color bColor ) {
    lineBuffer3D[numBuffered3DLines++] = {
        a.x,a.y,a.z,1.0f,
        aColor.r,aColor.g,aColor.b,aColor.a,
    };
    lineBuffer3D[numBuffered3DLines++] = {
        b.x,b.y,b.z,1.0f,
        bColor.r,bColor.g,bColor.b,bColor.a,
    };
    if ( numBuffered3DLines >= MAX_BUFFERED_LINES*2 ) Render3DLines();
}
void RendererGLProg::Render3DLines() {
    if ( numBuffered3DLines == 0 ) return;
    d_shaderDefault_vColor->Begin();
    d_shaderDefault_vColor->setUniformM4fv("MVP", mvp3D);
    glBindVertexArray(lines_vao);
    glBindBuffer(GL_ARRAY_BUFFER, lines_vbo);
    // Buffer instance data to GPU
    glBufferData(GL_ARRAY_BUFFER, sizeof(ColorVertexData)*numBuffered3DLines, lineBuffer3D, GL_STATIC_DRAW);
    glDrawArrays(GL_LINES, 0, numBuffered3DLines);
    glBindVertexArray(0);
    renderedSegs += numBuffered3DLines/2;
    numBuffered3DLines = 0;
    d_shaderDefault_vColor->End();
}

void RendererGLProg::Buffer3DCube(CubeInstance& instance) {
    if ( numBufferedCubes > MAX_CUBE_INSTANCES ) return; // Buffer full
    cubeBufferPos[numBufferedCubes*4+0] = instance.x;
    cubeBufferPos[numBufferedCubes*4+1] = instance.y;
    cubeBufferPos[numBufferedCubes*4+2] = instance.z;
    cubeBufferPos[numBufferedCubes*4+3] = instance.s*2.0f;
    cubeBufferRot[numBufferedCubes*4+0] = instance.rx;
    cubeBufferRot[numBufferedCubes*4+1] = instance.ry;
    cubeBufferRot[numBufferedCubes*4+2] = instance.rz;
    cubeBufferRot[numBufferedCubes*4+3] = instance.rw;
    cubeBufferColDiff[numBufferedCubes*4+0] = instance.dr;
    cubeBufferColDiff[numBufferedCubes*4+1] = instance.dg;
    cubeBufferColDiff[numBufferedCubes*4+2] = instance.db;
    cubeBufferColDiff[numBufferedCubes*4+3] = instance.da;
    cubeBufferColSpec[numBufferedCubes*4+0] = instance.sr;
    cubeBufferColSpec[numBufferedCubes*4+1] = instance.sg;
    cubeBufferColSpec[numBufferedCubes*4+2] = instance.sb;
    cubeBufferColSpec[numBufferedCubes*4+3] = instance.sa;
    numBufferedCubes++;
    if ( numBufferedCubes >= MAX_CUBE_INSTANCES ) {
        Render3DCubes();
    }
}
void RendererGLProg::Render3DCubes() {
    if ( numBufferedCubes == 0 ) return;
    // Buffer cube instance data to GPU
    glBindVertexArray(cubes_vao);
    glBindBuffer(GL_ARRAY_BUFFER, cubes_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(cube_vertices)+sizeof(cube_normals),
                    sizeof(GLfloat)*4*numBufferedCubes, cubeBufferPos);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(cube_vertices)+sizeof(cube_normals)+sizeof(cubeBufferPos),
                    sizeof(GLfloat)*4*numBufferedCubes, cubeBufferRot);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(cube_vertices)+sizeof(cube_normals)+sizeof(cubeBufferPos)+sizeof(cubeBufferRot),
                    sizeof(GLfloat)*4*numBufferedCubes, cubeBufferColDiff);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(cube_vertices)+sizeof(cube_normals)+sizeof(cubeBufferPos)+sizeof(cubeBufferRot),
                    sizeof(GLfloat)*4*numBufferedCubes, cubeBufferColSpec);

    GLint fbo = render_fbo; // Grab previous FBO
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fbo);
    // Render shadow pass
    if ( g_options->GetOptionDataPtr<bool>("r_shadows") ) {
        d_shaderShadowMapCube->Begin();
        std::vector<Light3D*>& lights = g_lights3D->GetLights();
        for ( int i=0; i < lights.size(); i++ ) {  // Render to shadow buffer
            Light3D& light = *lights[i];
            if ( !light.active ) continue;
            if ( !light.shadowCaster ) continue;
            if ( light.lightType == Light3D_Point ) {
                glBindFramebuffer(GL_FRAMEBUFFER, shadow_cube_fbo);
            } else {
                glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
            }
            glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
            glm::mat4 lightMVP = GetLightMVP(*lights[i]);
            d_shaderShadowMapCube->setUniformM4fv("MVP", lightMVP);
            glDrawArraysInstanced(GL_TRIANGLES, 0, 36, numBufferedCubes);
        }
        d_shaderShadowMapCube->End();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, windowWidth, windowHeight);
    // Shader cube rendering
    d_shaderCube->Begin();
    d_shaderCube->setUniformM4fv("MVP", mvp3D);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, numBufferedCubes);
    glBindVertexArray(0);
    d_shaderCube->End();
    renderedTris += numBufferedCubes*12;
    numBufferedCubes = 0;
}
void RendererGLProg::Render2DCube( const glm::vec2& center, const Color color, const glm::vec3 rotation) {
    if ( numBufferedCubes != 0 ) Render3DCubes();
    
	glm::quat orientation = glm::quat(rotation);
	glm::normalize(orientation);
    
    cubeBufferPos[numBufferedCubes*4+0] = center.x;
    cubeBufferPos[numBufferedCubes*4+1] = center.y;
    cubeBufferPos[numBufferedCubes*4+2] = -7.0f;
    cubeBufferPos[numBufferedCubes*4+3] = 1.0f; //size
    cubeBufferRot[numBufferedCubes*4+0] = orientation.x;
    cubeBufferRot[numBufferedCubes*4+1] = orientation.y;
    cubeBufferRot[numBufferedCubes*4+2] = orientation.z;
    cubeBufferRot[numBufferedCubes*4+3] = orientation.w;
    cubeBufferColDiff[numBufferedCubes*4+0] = color.r;
    cubeBufferColDiff[numBufferedCubes*4+1] = color.g;
    cubeBufferColDiff[numBufferedCubes*4+2] = color.b;
    cubeBufferColDiff[numBufferedCubes*4+3] = color.a;
    cubeBufferColSpec[numBufferedCubes*4+0] = color.r;
    cubeBufferColSpec[numBufferedCubes*4+1] = color.g;
    cubeBufferColSpec[numBufferedCubes*4+2] = color.b;
    cubeBufferColSpec[numBufferedCubes*4+3] = color.a;
    numBufferedCubes++;
    
    GLfloat aspectRatio = (windowWidth > windowHeight) ? float(windowWidth)/float(windowHeight) : float(windowHeight)/float(windowWidth);
    glm::mat4 mvp = glm::perspective(g_camera->fieldOfView, aspectRatio, g_camera->nearDepth, g_camera->farDepth);
    // Bind textured cube shader
    d_shaderCube->Begin();
    d_shaderCube->setUniformM4fv("MVP", mvp);
    // Bind textured cube vbo
    glBindVertexArray(cubes_vao);
    glBindBuffer(GL_ARRAY_BUFFER, cubes_vbo);
    // Buffer instance data to GPU
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(cube_vertices)+sizeof(cube_normals),
                    sizeof(GLfloat)*4*numBufferedCubes, cubeBufferPos);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(cube_vertices)+sizeof(cube_normals)+sizeof(cubeBufferPos),
                    sizeof(GLfloat)*4*numBufferedCubes, cubeBufferRot);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(cube_vertices)+sizeof(cube_normals)+sizeof(cubeBufferPos)+sizeof(cubeBufferRot),
                    sizeof(GLfloat)*4*numBufferedCubes, cubeBufferColDiff);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(cube_vertices)+sizeof(cube_normals)+sizeof(cubeBufferPos)+sizeof(cubeBufferRot)+sizeof(cubeBufferColDiff),
                    sizeof(GLfloat)*4*numBufferedCubes, cubeBufferColSpec);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, numBufferedCubes);
    d_shaderCube->End();
    glBindVertexArray(0);
    renderedTris += numBufferedCubes*12;
    numBufferedCubes = 0;
}

void RendererGLProg::DrawBoxOutline( glm::vec3 center, glm::vec3 boxSize, Color color ) {
    glm::vec3 topLeftBack = center + glm::vec3(boxSize.x,-boxSize.y,-boxSize.z);
    glm::vec3 topRightBack = center + glm::vec3(boxSize.x,boxSize.y,-boxSize.z);
    glm::vec3 topLeftFront = center + glm::vec3(boxSize.x,-boxSize.y,boxSize.z);
    glm::vec3 topRightFront = center + glm::vec3(boxSize.x,boxSize.y,boxSize.z);
    glm::vec3 bottomLeftBack = center + glm::vec3(-boxSize.x,-boxSize.y,-boxSize.z);
    glm::vec3 bottomRightBack = center + glm::vec3(-boxSize.x,boxSize.y,-boxSize.z);
    glm::vec3 bottomLeftFront = center + glm::vec3(-boxSize.x,-boxSize.y,boxSize.z);
    glm::vec3 bottomRightFront = center + glm::vec3(-boxSize.x,boxSize.y,boxSize.z);
    Buffer3DLine(topLeftBack, topLeftFront, color, color);
    Buffer3DLine(topLeftFront, topRightFront, color, color);
    Buffer3DLine(topRightFront, topRightBack, color, color);
    Buffer3DLine(topRightBack, topLeftBack, color, color);
    Buffer3DLine(bottomLeftBack, bottomLeftFront, color, color);
    Buffer3DLine(bottomLeftFront, bottomRightFront, color, color);
    Buffer3DLine(bottomRightFront, bottomRightBack, color, color);
    Buffer3DLine(bottomRightBack, bottomLeftBack, color, color);
    Buffer3DLine(topLeftBack, bottomLeftBack, color, color);
    Buffer3DLine(topLeftFront, bottomLeftFront, color, color);
    Buffer3DLine(topRightBack, bottomRightBack, color, color);
    Buffer3DLine(topRightFront, bottomRightFront, color, color);
//    RenderLines();
}
void RendererGLProg::DrawSphere(float radius, unsigned int rings, unsigned int sectors)
{
    float const R = 1./(float)(rings-1);
    float const S = 1./(float)(sectors-1);
    int r, s;
    
    NormalVertexData* nverts = new NormalVertexData[rings * sectors];

//    GLfloat* vertices = new GLfloat[rings * sectors * 3];
//    GLfloat* normals = new GLfloat[rings * sectors * 3];
//    GLfloat* texcoords = new GLfloat[rings * sectors * 3];

//    GLfloat* v = vertices;
//    GLfloat* n = normals;
//    GLfloat* t = texcoords;


    for(r = 0; r < rings; r++) for(s = 0; s < sectors; s++) {
        float const y = sin( -M_PI_2 + M_PI * r * R );
        float const x = cos(2*M_PI * s * S) * sin( M_PI * r * R );
        float const z = sin(2*M_PI * s * S) * sin( M_PI * r * R );
        int i = r*sectors + s;
        nverts[i] = {x * radius,y * radius, z * radius, 1,
                    1.0f, 1.0f, 0.0f, 1.0f,
                    x, y, z
        };
//        *t++ = s*S;
//        *t++ = r*R;
        
//        *v++ = x * radius;
//        *v++ = y * radius;
//        *v++ = z * radius;
        
//        *n++ = x;
//        *n++ = y;
//        *n++ = z;
    }
    GLushort* indices = new GLushort[rings * sectors * 4];
    GLushort* i = indices;
    for(r = 0; r < rings-1; r++) for(s = 0; s < sectors-1; s++) {
        *i++ = r * sectors + s;
        *i++ = r * sectors + (s+1);
        *i++ = (r+1) * sectors + (s+1);
        *i++ = (r+1) * sectors + s;
    }
//    delete [] vertices;
//    delete [] normals;
//    delete [] texcoords;
//    delete [] indices;
    
    BufferVerts(nverts, rings*sectors);
    delete [] nverts;
	delete [] indices;
    RenderVerts();
}


const glm::vec3 RendererGLProg::GetCursor3DPos( glm::vec2 cursorPos ) const {
    const int hw = windowWidth/2;
    const int hh = windowHeight/2;

    GLfloat cursorDepth=0;
    // Obtain the Z position (not world coordinates but in range 0 ~ 1)
    glReadPixels(cursorPos.x+hw, cursorPos.y+hh, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &cursorDepth);
//    printf("cursor depth: %f\n", cursorDepth);
    // Grab pixel under cursor color
//    GLfloat col[4];
//    glReadPixels(cursorPos.x, cursorPos.y, 1, 1, GL_RGBA, GL_FLOAT, &col);
//    printf("cursor color:%.1f, %.1f %.1f\n", col[0],col[1],col[2]);
    
    // Prepare matrices to unproject cursor coordinates
    glm::mat4 model = glm::mat4();
    model = glm::rotate(model, -g_camera->rotation.x, glm::vec3(1.0, 0.0, 0.0));
    model = glm::rotate(model, -g_camera->rotation.y, glm::vec3(0.0, 1.0, 0.0));
    model = glm::rotate(model, -g_camera->rotation.z, glm::vec3(0.0, 0.0, 1.0));
    model = glm::translate(model, glm::vec3(-g_camera->position.x, -g_camera->position.y, -g_camera->position.z));
    
    glm::mat4 proj = glm::mat4();
    GLfloat aspectRatio = (windowWidth > windowHeight) ?
    GLfloat(windowWidth)/GLfloat(windowHeight) : GLfloat(windowHeight)/GLfloat(windowWidth);
    proj = glm::perspective(g_camera->fieldOfView, aspectRatio, g_camera->nearDepth, g_camera->farDepth);
    glm::vec4 viewport = glm::vec4(0, 0, windowWidth, windowHeight);
    glm::vec3 crosshairPos  = glm::unProject(glm::vec3(cursorPos.x+hw, cursorPos.y+hh, cursorDepth), model, proj, viewport);
    // Linearize depth
//    glm::vec2 depthParameter = glm::vec2( g_camera->farDepth / ( g_camera->farDepth - g_camera->nearDepth ),
//                                         g_camera->farDepth * g_camera->nearDepth / ( g_camera->nearDepth - g_camera->farDepth ) );
//    float Z = depthParameter.y/(depthParameter.x - cursorDepth);

//    printf("Cursor depth: %f, pos: %f, %f, %f\n", Z, crosshairPos.x, crosshairPos.y, crosshairPos.z);
    return crosshairPos;
}

const std::string RendererGLProg::GetInfo() const {
    std::string info = "Renderer: OpenGL Programmable pipeline\n";
    if ( g_options->GetOptionDataPtr<bool>("r_fullScreen") ) {
        info.append("Full screen, resolution: ");
    } else {
        info.append("Window, resolution: ");
    }
    info.append(intToString(windowWidth));
    info.append(", ");
    info.append(intToString(windowHeight));
    info.append("\n");

    return info;
}

