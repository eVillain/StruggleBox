#include "RendererGLProg.h"
#include "GLErrorUtil.h"
#include "RenderUtils.h"
#include "FileUtil.h"
#include "PathUtil.h"
#include "Timer.h"

#include "ShaderManager.h"
#include "Shader.h"
#include "VertBuffer.h"
#include "Mesh.h"
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
const float LIGHT_SCALE = 0.5;
const float SSAO_SCALE = 0.5;

const int CUBE_BUFFER_MAX = 32 * 32 * 32 * 32;
const int SPHERE_BUFFER_MAX = 32 * 32 * 32;
const int SPRITE_BUFFER_MAX = 32 * 32 * 32;
const int COLORVERT_BUFFER_MAX = 2048;

// Light matrices to look in all 6 directions for cubemapping
static const glm::mat4 lightViewMatrix[6] = {
    glm::lookAt(glm::vec3(0.0f,0.0f,0.0f), glm::vec3( 1.0f, 0.0f, 0.0f), glm::vec3( 0.0f,-1.0f, 0.0f)),  // +x
    glm::lookAt(glm::vec3(0.0f,0.0f,0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3( 0.0f,-1.0f, 0.0f)),  // -x
    glm::lookAt(glm::vec3(0.0f,0.0f,0.0f), glm::vec3( 0.0f, 1.0f, 0.0f), glm::vec3( 0.0f, 0.0f, 1.0f)),  // +y
    glm::lookAt(glm::vec3(0.0f,0.0f,0.0f), glm::vec3( 0.0f,-1.0f, 0.0f), glm::vec3( 0.0f, 0.0f,-1.0f)),  // -y
    glm::lookAt(glm::vec3(0.0f,0.0f,0.0f), glm::vec3( 0.0f, 0.0f, 1.0f), glm::vec3( 0.0f,-1.0f, 0.0f)),  // +z
    glm::lookAt(glm::vec3(0.0f,0.0f,0.0f), glm::vec3( 0.0f, 0.0f,-1.0f), glm::vec3( 0.0f,-1.0f, 0.0f))   // -z
};


RendererGLProg::RendererGLProg(
	std::shared_ptr<Options> options,
	std::shared_ptr<Camera> camera,
	std::shared_ptr<LightSystem3D> lights,
	std::shared_ptr<ShaderManager> shaders) :
	_options(options),
	_camera(camera),
	_lighting(lights),
	_shaders(shaders),
	_cubeData(CUBE_BUFFER_MAX),
	_sphereData(SPHERE_BUFFER_MAX),
	_spriteData(SPRITE_BUFFER_MAX),
	_colorVertData(COLORVERT_BUFFER_MAX)
{
	Log::Info("[Renderer] constructor, instance at %p", this);

    initialized = false;
    shouldClose = false;
    ssao_depthOnly = false;
    renderMode = RM_Final_Image;
    vertex_vbo = -1;
    vertex_vao = -1;
    lines_vbo = -1;
    lines_vao = -1;
    numBuffered2DLines = 0;
    numBuffered3DLines = 0;

    Console::AddVar((int&)renderMode, "renderMode");

	_reflectionProbe.setup(SHADOW_CUBE_SIZE);

	Initialize();
}

RendererGLProg::~RendererGLProg()
{
	Log::Info("[Renderer] destructor, instance at %p", this);

    if (initialized) {
        ShutDown();
    }
}

void RendererGLProg::Initialize()
{
	Log::Info("[Renderer] initializing...");

    SetDefaults();
    
    // Reset previous frame timer and counter to zero
    r_frameStartTime = 0.0;
    r_frameCounterTime = 0.0;
    r_frameCounter = 0;
    SetScreenMode();    // Set up a window to render into
    
    if (initialized)
	{
		_gBuffer.Initialize(windowWidth, windowHeight);
		SetupShaders();     // Setup all our shaders and render buffers
        SetupRenderBuffers();
        SetupGeometry();
		_materials.load(PathUtil::MaterialsPath() + "default.plist");
		refreshMaterials();
    }
}

void RendererGLProg::ShutDown()
{
    initialized = false;
    shouldClose = true;

	for (GLuint vao : _vertArrays)
	{
		glDeleteVertexArrays(1, &vao);
	}
    
	_gBuffer.Terminate();
	
	CleanupRenderBuffers();
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
    lr_weight = 1.95f;
    if (_options) {
        windowWidth = _options->getOption<int>("r_resolutionX");
        windowHeight = _options->getOption<int>("r_resolutionY");
    }
}

void RendererGLProg::SetScreenMode()
{
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
    
	_options->getOption<int>("r_resolutionX") = windowWidth;
	_options->getOption<int>("r_resolutionY") = windowHeight;

    // Refresh frambefuffers to new size
	_gBuffer.Resize(windowWidth, windowHeight);

    GLuint old_finalfbo = final_fbo;
    GLuint old_shfbo = shadow_fbo;
    GLuint old_shtex = shadow_texture;
    
    SetupRenderBuffers();

    glDeleteFramebuffers(1, &old_finalfbo);
    glDeleteFramebuffers(1, &old_shfbo);
    glDeleteRenderbuffers(1, &old_shtex);
}

// Matrix functionality
void RendererGLProg::GetUIMatrix( glm::mat4& target )
{
	// pixel-perfect 2D projection
    target = glm::ortho<GLfloat>(0, windowWidth,
                                 0, windowHeight,
                                 ORTHO_NEARDEPTH, ORTHO_FARDEPTH);
	// move origin (0,0) to center of window
	GLfloat hw = (windowWidth / 2);
	GLfloat hh = (windowHeight / 2);
    target = glm::translate(target, glm::vec3(hw, hh, 0));
}

void RendererGLProg::GetGameMatrix( glm::mat4 &target )
{
    GLfloat aspectRatio = (windowWidth > windowHeight) ?
    GLfloat(windowWidth)/GLfloat(windowHeight) : GLfloat(windowHeight)/GLfloat(windowWidth);
    target = glm::perspective(_camera->fieldOfView, aspectRatio, _camera->nearDepth, _camera->farDepth);
    target = glm::rotate(target, -_camera->rotation.x, glm::vec3(1.0, 0.0, 0.0));
    target = glm::rotate(target, -_camera->rotation.y, glm::vec3(0.0, 1.0, 0.0));
    target = glm::rotate(target, -_camera->rotation.z, glm::vec3(0.0, 0.0, 1.0));
    target = glm::translate(target, glm::vec3(-_camera->position.x, -_camera->position.y, -_camera->position.z));
}

/*  --------------------    *
 *      SHADER STUFF        *
 *  --------------------    */
void RendererGLProg::SetupShaders()
{
    // Used to draw lines and debugging
    f_shaderDefault_vColor = _shaders->load("d_default_vColor.vsh", "d_default.fsh");
    d_shaderDefault_uColor = _shaders->load("d_default_uColor.vsh", "d_default.fsh");
    // 2D rendering shaders
    f_shaderUI_tex = _shaders->load("f_ui_tex.vsh", "f_ui_tex.fsh");
    f_shaderUI_color = _shaders->load("f_ui_color.vsh", "f_ui_color.fsh");
    f_shaderUI_vColor = _shaders->load("f_ui_vcolor.vsh", "f_ui_vcolor.fsh");
    f_shaderUI_cubeMap = _shaders->load("f_ui_cubeMap.vsh", "f_ui_cubeMap.fsh");
    // Sunshine lens flares and halo
    d_shaderSunPP = _shaders->load("d_sunPostProcess.vsh", "d_sunPostProcess.fsh");
    d_shaderSunPP->begin();
    d_shaderSunPP->setUniform1iv("LowBlurredSunTexture", 0);
    d_shaderSunPP->setUniform1iv("HighBlurredSunTexture", 1);
    d_shaderSunPP->setUniform1iv("DirtTexture", 2);
    d_shaderSunPP->setUniform1fv("Dispersal", 0.1875f);
    d_shaderSunPP->setUniform1fv("HaloWidth", 0.45f);
    d_shaderSunPP->setUniform1fv("Intensity", 2.25f);
    d_shaderSunPP->setUniform3fv("Distortion", 0.94f, 0.97f, 1.00f);
    d_shaderSunPP->end();
    // Vertical and horizontal blurs
	d_shaderBlurH = _shaders->load("d_blur.vsh", "d_blur_horizontal.fsh");
	d_shaderBlurV = _shaders->load("d_blur.vsh", "d_blur_vertical.fsh");
    // Deferred light pass, includes fog and noise
    d_shaderLightShadow = _shaders->load("d_light_pass_shadow.vsh", "d_light_pass_shadow.fsh");
    // Renders the cubemesh
    d_shaderMesh = _shaders->load("d_mesh_color.vsh", "d_mesh_color.fsh");
    // Renders dynamic objects in batched instances
    d_shaderInstance = _shaders->load("d_object_instance.vsh", "d_object_instance.fsh");
    // Renders single colored cubes in batched instances
    d_shaderCube = _shaders->load("d_cube_instance.vsh", "d_cube_instance.fsh");
	// Renders triangle meshes
	_shaderDeferredMesh = _shaders->load("d_mesh.vsh", "d_mesh.fsh");
    // Renders instanced spheres
    _shaderDeferredSphere = _shaders->load( "d_impostor_sphere.gsh", "d_impostor_sphere.vsh", "d_impostor_sphere.fsh");
	// Renders instanced sprites
    _shaderDeferredSprite = _shaders->load( "d_impostor_billboard.gsh", "d_impostor_billboard.vsh", "d_impostor_billboard.fsh");
    d_shaderCloud = _shaders->load( "d_impostor_cloud.gsh", "d_impostor_cloud.vsh", "d_impostor_cloud.fsh");
    f_shaderSprite = _shaders->load( "f_impostor_billboard.gsh", "f_impostor_billboard.vsh", "f_impostor_billboard.fsh");
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
    d_shaderSky = _shaders->load("d_sky_dome.vsh", "d_sky_dome.fsh");
    d_shaderSky->begin();
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
    d_shaderSky->end();
    
    // Post-processing shaders
    d_shaderPost = _shaders->load("d_post_process.vsh", "d_post_process.fsh");
    d_shaderDepth = _shaders->load("d_depth.vsh", "d_depth.fsh");
    d_shaderLensFlare = _shaders->load("d_lensFlare.vsh", "d_lensFlare.fsh");
    d_shaderLightRays = _shaders->load("d_light_rays.vsh", "d_light_rays.fsh");
    if ( ssao_depthOnly ) {
        d_shaderSSAO = _shaders->load("d_ssao_depth.vsh", "d_ssao_depth.fsh");
    } else {
        d_shaderSSAO = _shaders->load("d_ssao_normal.vsh", "d_ssao_normal.fsh");
    }
    d_shaderEdgeSobel = _shaders->load("d_edge_sobel.vsh", "d_edge_sobel.fsh");
    d_shaderEdgeFreiChen = _shaders->load("d_edge_frei_chen.vsh", "d_edge_frei_chen.fsh");
    
    d_shaderShadowMapMesh = _shaders->load("d_shadowMap_mesh.vsh", "d_shadowMap.fsh");
    d_shaderShadowMapObject = _shaders->load("d_shadowMap_object.vsh", "d_shadowMap.fsh");
    d_shaderShadowMapSphere = _shaders->load("d_shadowMap_sphere.gsh", "d_shadowMap_sphere.vsh", "d_shadowMap_sphere.fsh");
    d_shaderShadowMapCube = _shaders->load("d_shadowMap_cube.vsh", "d_shadowMap.fsh");

//    d_shaderFXAA = _shaders->load("d_FXAA.vsh", "d_FXAA.fsh");
    
    // Load lens dirt texture
    tex_lens_dirt = TextureManager::Inst()->LoadTexture(FileUtil::GetPath().append("DATA/GFX/"),
                                                        "lens_dirt.png",
                                                        GL_CLAMP_TO_EDGE,
                                                        GL_NEAREST);
    // Load SSAO noise texture
    tex_ssao_noise = TextureManager::Inst()->LoadTexture(FileUtil::GetPath().append("DATA/GFX/"),
                                                         "noise.png",
                                                         GL_REPEAT,
                                                         GL_LINEAR);

	CHECK_GL_ERROR();
}

void RendererGLProg::SetupRenderBuffers()
{
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
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Generate Light frame buffer
    glGenFramebuffers(1, &light_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, light_fbo); // Bind our frame buffer
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, light_textures[0], 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, light_textures[4], 0);
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
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, _gBuffer.GetDepth(), 0);  // Attach previous depth/stencil to it
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

	CHECK_GL_ERROR();
}

void RendererGLProg::CleanupRenderBuffers()
{
    glDeleteTextures( 1, &final_texture );
    glDeleteFramebuffers(1, &final_fbo);
    glDeleteTextures( 1, &ao_texture );
    glDeleteFramebuffers(1, &ao_fbo);
    glDeleteTextures( 5, light_textures );
    glDeleteFramebuffers(1, &light_fbo);
    TextureManager::Inst()->UnloadTexture( tex_lens_dirt );
    TextureManager::Inst()->UnloadTexture( tex_ssao_noise );
}
/*  --------------------    *
 *      GEOMETRY STUFF      *
 *  --------------------    */
void RendererGLProg::SetupGeometry()
{
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
    // --------- COLOR VERTEX BUFFERS  ----------- //
	_colorVert_vao = addVertexArray();
	_colorVertVB = addVertBuffer(ColorVerts);
    //glGenVertexArrays(1, &colorVerts_vao);
    //glBindVertexArray(colorVerts_vao);
    //glGenBuffers(1, &colorVerts_vbo);
    //glBindBuffer(GL_ARRAY_BUFFER, colorVerts_vbo);
    //glBufferData(GL_ARRAY_BUFFER, MAX_BUFFERED_VERTS * sizeof(ColorVertexData), NULL, GL_DYNAMIC_DRAW);
    //glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), 0);
    //glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (GLvoid*)(4*sizeof(GLfloat)));
    //glEnableVertexAttribArray(0);
    //glEnableVertexAttribArray(1);
    //glBindVertexArray(0);
	// ----------   CUBE BUFFER   --------- //
	glGenVertexArrays(1, &_cube_vao);
	glBindVertexArray(_cube_vao);
	glGenBuffers(1, &_cube_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _cube_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices) + sizeof(cube_normals), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cube_vertices), cube_vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(cube_vertices), sizeof(cube_normals), cube_normals);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)sizeof(cube_vertices));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribDivisorARB(1, 0);
	_cubeVB = addVertBuffer(InstancedCubeVerts);
	glBindVertexArray(0);
    // ----------   SPHERE BUFFER   --------- //
	_sphere_vao = addVertexArray();
	_sphereVB = addVertBuffer(SphereVerts);
    glBindVertexArray(0);
	// ----------   SPRITE BUFFER   --------- //
	_sprite_vao = addVertexArray();
	// ----------   SPHERE BUFFER   --------- //
	_mesh_vao = addVertexArray();
	_meshVB = addVertBuffer(MeshVerts);
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
    // -------- INSTANCING BUFFERS ----------
	instancing_vao = addVertexArray();

	CHECK_GL_ERROR();
}

void RendererGLProg::CleanupGeometry()
{
    glDeleteVertexArrays(1, &_sphere_vao);
    glDeleteBuffers(1, &vertex_vbo);
    glDeleteBuffers(1, &vertex_ibo);
    glDeleteVertexArrays(1, &vertex_vao);
	glDeleteBuffers(1, &ui_vbo);
	glDeleteVertexArrays(1, &ui_vao);
    glDeleteBuffers(1, &square2D_vbo);
    glDeleteVertexArrays(1, &square2D_vao);
    glDeleteBuffers(1, &lines_vbo);
    glDeleteVertexArrays(1, &lines_vao);
    glDeleteBuffers(1, &_cube_vbo);
    glDeleteVertexArrays(1, &_cube_vao);
    glDeleteBuffers(1, & instancing_vbo);
    glDeleteVertexArrays(1, &instancing_vao);

	CHECK_GL_ERROR();
}

// Setup rendering for a new frame
void RendererGLProg::BeginDraw()
{
    // Save timestamp for beginning of this render cycle
    r_frameStartTime = Timer::Seconds();
    // Reset buffers
    numBuffered2DLines = 0;
    numBuffered3DLines = 0;
	_lightsQueue.clear();
	_sphereData.clear();
	_cubeData.clear();
	_colorVertData.clear();
	_deferredQueue.clear();
	_deferredInstanceQueue.clear();
	_forwardQueue.clear();

	// Reset stats
    renderedSegs = 0;
    renderedTris = 0;
    renderedSprites = 0;
}

void RendererGLProg::EndDraw()
{
    UpdateStats();
}

void RendererGLProg::UpdateStats()
{
    //double frameEndTime = Timer::Seconds();
    //_stats->SetRTime(frameEndTime-r_frameStartTime);
    //_stats->SetRNumSegs(renderedSegs);
    //_stats->SetRNumTris(renderedTris);
    //_stats->SetRNumSpheres(renderedSprites);
}

void RendererGLProg::flush()
{
	CHECK_GL_ERROR();

	if (_cubeData.getCount())	// upload cube data
	{
		_cubeVB->bind();
		_cubeVB->upload(_cubeData.getData(), _cubeData.getCount() * sizeof(CubeInstance), true);
	}
	if (_sphereData.getCount())	// upload sphere data
	{
		_sphereVB->bind();
		_sphereVB->upload(
			(void*)_sphereData.getData(), _sphereData.getCount() * sizeof(SphereVertexData), true);
	}

	// Clear color and Z-buffer
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0);
	glClearStencil(Stencil_None);

	glm::vec4 viewPort = glm::vec4(0, 0, windowWidth, windowHeight);
	updateReflections();

	_gBuffer.Bind();
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	_gBuffer.Clear();

	glViewport(0, 0, windowWidth, windowHeight);

	// Build matrices for billboards/impostors
	glm::mat4 rotMatrix = glm::mat4();
	rotMatrix = glm::rotate(rotMatrix, -_camera->rotation.x, glm::vec3(1.0, 0.0, 0.0));
	rotMatrix = glm::rotate(rotMatrix, -_camera->rotation.y, glm::vec3(0.0, 1.0, 0.0));
	rotMatrix = glm::rotate(rotMatrix, -_camera->rotation.z, glm::vec3(0.0, 0.0, 1.0));

	glm::mat4 view = glm::mat4();
	view = glm::translate(rotMatrix, glm::vec3(-_camera->position.x, -_camera->position.y, -_camera->position.z));

	glm::mat4 projection;
	GLfloat aspectRatio = (windowWidth > windowHeight) ?
		GLfloat(windowWidth) / GLfloat(windowHeight) : GLfloat(windowHeight) / GLfloat(windowWidth);
	projection = glm::perspective(_camera->fieldOfView, aspectRatio, _camera->nearDepth, _camera->farDepth);
	glm::mat3 normalMatrix = glm::inverse(glm::mat3(rotMatrix));
	CHECK_GL_ERROR();

	// Render everything
	if (_options->getOption<bool>("r_wireFrame"))
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_CULL_FACE);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, Stencil_Solid, 0xFF);
	renderCubes(projection*view);
	renderSpheres(view, projection, normalMatrix, _camera->position);
	RenderVerts();
	flushDeferredQueue(view, projection, _camera->position);
	glDisable(GL_STENCIL_TEST);
	CHECK_GL_ERROR();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	bool ssao = _options->getOption<bool>("r_renderSSAO");
	// Get Ambient Occlusion if needed
	if (ssao)
		PassSSAO(ao_fbo);
	
	glm::vec2 screenRatio = glm::vec2(1.0f, 1.0f);
	//RenderLighting(
	//	rotMatrix,
	//	projection,
	//	viewPort,
	//	_camera->position,
	//	screenRatio,
	//	_camera->nearDepth,
	//	_camera->farDepth);

	prepareFinalFBO(windowWidth, windowHeight);

	_lighting->RenderLighting(_lightsQueue,
		rotMatrix,
		projection,
		viewPort,
		_camera->position,
		screenRatio,
		_camera->nearDepth,
		_camera->farDepth,
		final_fbo,
		_gBuffer,
		_reflectionProbe.getPosition(),
		_reflectionProbe.getSize(),
		_reflectionProbe.getCubeMap(),
		_options->getOption<bool>("r_renderSSAO"),
		ao_texture,
		tex_ssao_noise->GetID());

	RenderLightingFX(
		rotMatrix,
		projection,
		viewPort,
		_camera->position,
		screenRatio,
		_camera->nearDepth,
		_camera->farDepth);


	PostProcess();
	// Copy depth data from gbuffer to screen buffer
	glBindFramebuffer(GL_READ_FRAMEBUFFER, _gBuffer.GetFBO());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, windowWidth, windowHeight, 0, 0, windowWidth, windowHeight,
		GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	flushForwardQueue(view, projection, _camera->position);
	render3DLines(projection*view);

	GetUIMatrix(mvp2D);
	bool renderDebug = _options->getOption<bool>("r_debug");
	if (renderDebug) { // Render buffers to screen for debugging
		float ar = (float)windowHeight / windowWidth;
		int ww2 = windowWidth / 2;
		int wh2 = windowHeight / 2;
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glDepthMask(GL_FALSE);
		Rect2D tRect = Rect2D(0.0, 0.0, 1.0, 1.0);
		// Render G-Buffer to screen
		DrawTexture(Rect2D(-640, wh2 - (256 * ar), 256, 256 * ar), tRect, _gBuffer.GetAlbedo(), mvp2D);
		DrawTexture(Rect2D(-384, wh2 - (256 * ar), 256, 256 * ar), tRect, _gBuffer.GetMaterial(), mvp2D);
		DrawTexture(Rect2D(-128, wh2 - (256 * ar), 256, 256 * ar), tRect, _gBuffer.GetNormal(), mvp2D);
		DrawTexture(Rect2D(128, wh2 - (256 * ar), 256, 256 * ar), tRect, _gBuffer.GetDepth(), mvp2D);
		DrawTexture(Rect2D(384, wh2 - (256 * ar), 256, 256 * ar), tRect, ao_texture, mvp2D);
		DrawTexture(Rect2D(384, wh2 - (768 * ar), 256, 256 * ar), tRect, final_texture, mvp2D);
		if (_options->getOption<bool>("r_debugLights")) { // Render light buffers to screen for debugging
			DrawTexture(Rect2D(-640, wh2 - (512 * ar), 256, 256 * ar), tRect, light_textures[0], mvp2D);
			DrawTexture(Rect2D(-384, wh2 - (512 * ar), 256, 256 * ar), tRect, light_textures[1], mvp2D);
			DrawTexture(Rect2D(-128, wh2 - (512 * ar), 256, 256 * ar), tRect, light_textures[2], mvp2D);
			DrawTexture(Rect2D(128, wh2 - (512 * ar), 256, 256 * ar), tRect, light_textures[3], mvp2D);
			DrawTexture(Rect2D(384, wh2 - (512 * ar), 256, 256 * ar), tRect, light_textures[4], mvp2D);
		}
		if (_options->getOption<bool>("r_debugShadows")) { // Render shadow buffers to screen for debugging
														   //DrawTexture(Rect2D(384, wh2 - (1024 * ar), 256, 256 * ar), tRect, shadow_texture);
			const int drawSize = 256;
			// Draw cubemaps
			//DrawCubeMap(Rect2D(-640, wh2 - (512), 256, 256), texCoordsXN, shadow_cubeMap);//-X
			//DrawCubeMap(Rect2D(-384, wh2 - (512), 256, 256), texCoordsZP, shadow_cubeMap);//+Z
			//DrawCubeMap(Rect2D(-128, wh2 - (512), 256, 256), texCoordsXP, shadow_cubeMap);//+X
			//DrawCubeMap(Rect2D(128, wh2 - (512), 256, 256), texCoordsZN, shadow_cubeMap);//-Z
			//DrawCubeMap(Rect2D(-384, wh2 - (256), 256, 256), texCoordsYN, shadow_cubeMap);//-Y
			//DrawCubeMap(Rect2D(-384, wh2 - (768), 256, 256), texCoordsYP, shadow_cubeMap);//+Y
			DrawCubeMap(Rect2D(-640, wh2 - (drawSize * 2), drawSize, drawSize), CubeMapTexCoords[Negative_X], _reflectionProbe.getCubeMap());//-X
			DrawCubeMap(Rect2D(-384, wh2 - (drawSize * 2), drawSize, drawSize), CubeMapTexCoords[Positive_Z], _reflectionProbe.getCubeMap());//+Z
			DrawCubeMap(Rect2D(-128, wh2 - (drawSize * 2), drawSize, drawSize), CubeMapTexCoords[Positive_X], _reflectionProbe.getCubeMap());//+X
			DrawCubeMap(Rect2D(128, wh2 - (drawSize * 2), drawSize, drawSize), CubeMapTexCoords[Negative_Z], _reflectionProbe.getCubeMap());//-Z
			DrawCubeMap(Rect2D(-384, wh2 - (drawSize * 3), drawSize, drawSize), CubeMapTexCoords[Negative_Y], _reflectionProbe.getCubeMap());//-Y
			DrawCubeMap(Rect2D(-384, wh2 - (drawSize * 1), drawSize, drawSize), CubeMapTexCoords[Positive_Y], _reflectionProbe.getCubeMap());//+Y
		}
		// debug materials data
		//DrawTexture(Rect2D(-128, -4, 512, 8), tRect, _materialTexture.getTexture(), mvp2D);
	}
	CHECK_GL_ERROR();
}

void RendererGLProg::updateReflections()
{
	glm::vec4 viewPort = glm::vec4(0, 0, windowWidth, windowHeight);

	const int drawSize = 256;
	const int cubeSize = _reflectionProbe.getTextureSize();
	const float cubeNearDepth = 0.001;
	const float cubeFarDepth = _reflectionProbe.getSize();

	for (int side = 0; side < 6; side++)
	{
		CHECK_GL_ERROR();

		CubeMapSide cubeSide = (CubeMapSide)side;
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_TRUE);
		_gBuffer.Bind();
		_gBuffer.Clear();
		glm::mat4 reflectionView = _reflectionProbe.getView(cubeSide);
		glm::mat4 reflectionProjection = _reflectionProbe.getProjection(cubeSide, cubeNearDepth, cubeFarDepth);
		glm::mat3 reflectionNormalMatrix = glm::inverse(glm::mat3(reflectionView));

		glViewport(0, 0, cubeSize, cubeSize);
		//glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilFunc(GL_ALWAYS, Stencil_Solid, 0xFF);
		renderCubes(_reflectionProbe.getMVP(cubeSide, cubeNearDepth, cubeFarDepth));
		renderSpheres(
			reflectionView,
			reflectionProjection,
			reflectionNormalMatrix,
			_reflectionProbe.getPosition());
		flushDeferredQueue(
			reflectionView,
			reflectionProjection,
			_reflectionProbe.getPosition());
		glDisable(GL_STENCIL_TEST);
		CHECK_GL_ERROR();

		float ratioX = (float)cubeSize / windowWidth;
		float ratioY = (float)cubeSize / windowHeight;
		// render lit reflection perspective into final_fbo
		//RenderLighting(
		//	reflectionView,
		//	reflectionProjection,
		//	viewPort,
		//	_reflectionProbe.getPosition(),
		//	glm::vec2(ratioX, ratioY),
		//	cubeNearDepth,
		//	cubeFarDepth);

		prepareFinalFBO(windowWidth, windowHeight);

		_lighting->RenderLighting(
			_lightsQueue,
			reflectionView,
			reflectionProjection,
			viewPort,
			_reflectionProbe.getPosition(),
			glm::vec2(ratioX, ratioY),
			cubeNearDepth,
			cubeFarDepth,
			final_fbo,
			_gBuffer,
			_reflectionProbe.getPosition(),
			_reflectionProbe.getSize(),
			_reflectionProbe.getCubeMap(),
			false,
			ao_texture,
			tex_ssao_noise->GetID());

		// render forward queue, should be mostly self-lit particles
		//flushForwardQueue(
		//	reflectionView,
		//	reflectionProjection,
		//	_reflectionProbe.getPosition());
		//RenderLightingFX(
		//	reflectionView,
		//	reflectionProjection,
		//	viewPort,
		//	_reflectionProbe.getPosition(),
		//	glm::vec2(ratioX, ratioY),
		//	cubeNearDepth,
		//	cubeFarDepth);

		_reflectionProbe.bind(cubeSide);
		//glClear(GL_COLOR_BUFFER_BIT);

		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glDepthMask(GL_FALSE);

		Rect2D tRect = Rect2D(0.0f, 0.0f, ratioX, ratioY);

		glm::mat4 mvp = glm::ortho<GLfloat>(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
		// copy lit image from final_fbo to cubemap side
		DrawTexture(Rect2D(0, 0, 1.0f, 1.0f), tRect, final_texture, mvp);

		if (side == 5)
		{
			glBindTexture(GL_TEXTURE_CUBE_MAP, _reflectionProbe.getCubeMap());
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);  // Generate mipmaps for rough surfaces
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		}
	}
}

void RendererGLProg::prepareFinalFBO(const int width, const int height)
{
	// Output to final image FBO
	glBindFramebuffer(GL_FRAMEBUFFER, final_fbo);
	//glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_BLEND);

	// Draw sky layer without lighting and opaque layer in black
	glEnable(GL_STENCIL_TEST);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilFunc(GL_EQUAL, Stencil_Sky, 0xFF);             // Only draw sky layer
	RenderFromTexture(_gBuffer.GetAlbedo());
	glStencilFunc(GL_EQUAL, Stencil_Solid, 0xFF);           // Only draw solid layer
	Draw2DRect(glm::vec2(), width, height, COLOR_NONE, COLOR_BLACK, 0.0f);
	glDisable(GL_STENCIL_TEST);
}


void RendererGLProg::PostProcess()
{
	CHECK_GL_ERROR();

	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind screen buffer and clear it
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	bool dof = _options->getOption<bool>("r_renderDOF");
	bool flare = _options->getOption<bool>("r_renderFlare");
	bool vignette = _options->getOption<bool>("r_renderVignette");
	bool correctGamma = _options->getOption<bool>("r_renderCorrectGamma");
	bool toneMap = _options->getOption<bool>("r_renderToneMap");

	//if (renderMode == RM_Final_Image &&
	//	!dof && !flare && !vignette && !correctGamma && !toneMap && !renderDebug)
	//{
	//	RenderFromTexture(final_texture);
	//	glDisable(GL_STENCIL_TEST);
	//	glDisable(GL_BLEND);
	//	glDepthMask(GL_TRUE);
	//	glEnable(GL_DEPTH_TEST);
	//	return;
	//}

	if (renderMode == RM_Final_Image)
	{
		CHECK_GL_ERROR();

		glm::mat4 mvp2d = glm::ortho<GLfloat>(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

		d_shaderPost->begin();
		d_shaderPost->setUniformM4fv("MVP", mvp2d);
		d_shaderPost->setUniform1iv("textureMap", 0);
		d_shaderPost->setUniform1iv("depthMap", 1);
		d_shaderPost->setUniform1iv("dustMap", 2);
		d_shaderPost->setUniform1fv("textureWidth", windowWidth);
		d_shaderPost->setUniform1fv("textureHeight", windowHeight);
		d_shaderPost->setUniform1fv("focalDepth", _camera->focalDepth);
		d_shaderPost->setUniform1fv("focalLength", _camera->focalLength);
		d_shaderPost->setUniform1fv("fstop", _camera->fStop);
		d_shaderPost->setUniform1fv("exposure", _camera->exposure);
		d_shaderPost->setUniform1iv("showFocus", _camera->debugLens);
		d_shaderPost->setUniform1iv("autofocus", _camera->autoFocus);
		d_shaderPost->setUniform1iv("renderDOF", dof);
		d_shaderPost->setUniform1iv("renderVignette", vignette);
		d_shaderPost->setUniform1iv("correctGamma", correctGamma);
		d_shaderPost->setUniform1iv("toneMap", toneMap);
		d_shaderPost->setUniform1fv("znear", _camera->nearDepth);
		d_shaderPost->setUniform1fv("zfar", _camera->farDepth);

		glDepthMask(GL_FALSE);

		glActiveTexture(GL_TEXTURE2);
		tex_lens_dirt->Bind();
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, _gBuffer.GetDepth());
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, final_texture);
		glBindVertexArray(square2D_vao);
		glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		d_shaderPost->end();
		glBindVertexArray(0);
		renderedTris += 2;
	}
	else
	{
		CHECK_GL_ERROR();

		// Render debug image
		glm::mat4 mvp2d = glm::ortho<GLfloat>(-0.5, 0.5, -0.5, 0.5, -1.0, 1.0);
		// Pick which image to show
		if (renderMode == RM_Diffuse) {
			// Nothing fancy here, just render to screen
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			RenderFromTexture(_gBuffer.GetAlbedo());
		}
		else if (renderMode == RM_Specular) {
			// Nothing fancy here, just render to screen
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			RenderFromTexture(_gBuffer.GetMaterial());
		}
		else if (renderMode == RM_Normal) {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			RenderFromTexture(_gBuffer.GetNormal());
		}
		else if (renderMode == RM_Depth) {
			d_shaderDepth->begin();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, _gBuffer.GetDepth());
			d_shaderDepth->setUniformM4fv("MVP", mvp2d);
			d_shaderDepth->setUniform4fv("u_color", COLOR_WHITE);
			d_shaderDepth->setUniform1fv("nearDepth", _camera->nearDepth);
			d_shaderDepth->setUniform1fv("farDepth", _camera->farDepth);
			glBindVertexArray(square2D_vao);
			glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			d_shaderDepth->end();
		}
		else if (renderMode == RM_SSAO) {
			PassSSAO(ao_fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			RenderFromTexture(ao_texture);
		}
		else if (renderMode == RM_Shadowmap) {
			d_shaderDepth->begin();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, shadow_texture);
			d_shaderDepth->setUniformM4fv("MVP", mvp2d);
			d_shaderDepth->setUniform4fv("u_color", COLOR_WHITE);
			d_shaderDepth->setUniform1fv("nearDepth", _camera->nearDepth);
			d_shaderDepth->setUniform1fv("farDepth", _camera->farDepth);
			glBindVertexArray(square2D_vao);
			glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			d_shaderDepth->end();
		}
		else if (renderMode == RM_Stencil) {
			PassStencil(0);
		}
	}
	CHECK_GL_ERROR();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RendererGLProg::RenderFromTexture( const GLuint tex )
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glm::mat4 mvp2d = glm::ortho<GLfloat>(-0.5, 0.5, -0.5, 0.5, -1.0, 1.0);
    f_shaderUI_tex->begin();
    f_shaderUI_tex->setUniformM4fv("MVP", mvp2d);
    f_shaderUI_tex->setUniform4fv("u_color", COLOR_WHITE);
    glBindVertexArray(square2D_vao);
    glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    f_shaderUI_tex->end();
    glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
    renderedTris += 2;
}

void RendererGLProg::PassSSAO( const GLuint fbo )
{
    int blur = _options->getOption<int>("r_SSAOblur");
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
    d_shaderSSAO->begin();
    d_shaderSSAO->setUniformM4fv( "MVP", mvp2d );
    d_shaderSSAO->setUniform1fv( "texScale", 1.0/SSAO_SCALE );
    d_shaderSSAO->setUniform1iv( "depthMap", 1 );
    d_shaderSSAO->setUniform1iv( "rnm", 0 );
    float total_strength = _options->getOption<float>("r_SSAOtotal_strength");
    float base = _options->getOption<float>("r_SSAObase");
    float area = _options->getOption<float>("r_SSAOarea");
    float falloff = _options->getOption<float>("r_SSAOfalloff");
    float radius = _options->getOption<float>("r_SSAOradius");
    d_shaderSSAO->setUniform1fv("total_strength", total_strength);
    d_shaderSSAO->setUniform1fv("base", base);
    d_shaderSSAO->setUniform1fv("area", area);
    d_shaderSSAO->setUniform1fv("falloff", falloff);
    d_shaderSSAO->setUniform1fv("radius", radius);

    if ( !ssao_depthOnly ) {
        d_shaderSSAO->setUniform1iv( "normalMap", 2 );
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, _gBuffer.GetNormal());
    } else {
        d_shaderSSAO->setUniform2fv( "pixelSize", glm::vec2(1.0f/(windowWidth*SSAO_SCALE), 1.0f/(windowHeight*SSAO_SCALE)) );
    }
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _gBuffer.GetDepth());
    glActiveTexture(GL_TEXTURE0);
    tex_ssao_noise->Bind();
    glBindVertexArray(square2D_vao);
    glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
    // Render SSAO to buffer
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    d_shaderSSAO->end();
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
        d_shaderBlurH->begin();
        d_shaderBlurH->setUniform1iv("Width", blur);
        d_shaderBlurH->setUniform1fv("odw", 1.0f/(windowWidth*LIGHT_SCALE));
        glBindVertexArray(square2D_vao);
        glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        d_shaderBlurH->end();
        glBindVertexArray(0);
        // Blur vertically - wide (tex 3 to buffer)
        // Render SSAO to buffer
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glActiveTexture(GL_TEXTURE0);
        glClear( GL_COLOR_BUFFER_BIT );
        glBindTexture(GL_TEXTURE_2D, light_textures[3]);
        d_shaderBlurV->begin();
        d_shaderBlurV->setUniform1iv("Width", blur);
        d_shaderBlurV->setUniform1fv("odh", 1.0f/(windowHeight*LIGHT_SCALE));
        glBindVertexArray(square2D_vao);
        glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        d_shaderBlurV->end();
        glBindVertexArray(0);
    }
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
}

void RendererGLProg::PassStencil( const GLuint fbo )
{
    if ( fbo != _gBuffer.GetFBO() )
	{
        glBindFramebuffer(GL_READ_FRAMEBUFFER, _gBuffer.GetFBO());
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

void RendererGLProg::PassLightRays( glm::vec3 lightWorldPos, const GLuint fbo )
{
    // Render regular image to screen first
//    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
//    RenderFromTexture(_albedoTexture);

    // Project sun coordinate to screen space
    glm::mat4 model = glm::mat4();
    model = glm::rotate(model, -_camera->rotation.x, glm::vec3(1.0, 0.0, 0.0));
    model = glm::rotate(model, -_camera->rotation.y, glm::vec3(0.0, 1.0, 0.0));
    model = glm::rotate(model, -_camera->rotation.z, glm::vec3(0.0, 0.0, 1.0));
    glm::mat4 proj = glm::mat4();
    GLfloat aspectRatio = (windowWidth > windowHeight) ?
    GLfloat(windowWidth)/GLfloat(windowHeight) : GLfloat(windowHeight)/GLfloat(windowWidth);
    proj = glm::perspective(_camera->fieldOfView, aspectRatio, _camera->nearDepth, _camera->farDepth);
    glm::vec4 viewport = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    
    glm::mat4 mvp2d = glm::ortho<GLfloat>(-0.5, 0.5, -0.5, 0.5, -1.0, 1.0);

    for (int i=0; i < _lightsQueue.size(); i++) {
        LightInstance& light = _lightsQueue[i];
        if ( !light.active ) continue;
            glm::vec3 lightPos = glm::vec3(light.position.x, light.position.y, light.position.z);
            glm::vec3 lightScrnPos = glm::project(lightPos, model, proj, viewport);

            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            d_shaderLightRays->begin();
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
            d_shaderLightRays->end();
        

//        // Get light and occluders from stencil buffer and render texture
//        glBindFramebuffer(GL_READ_FRAMEBUFFER, _renderFBO);
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
//        glBindTexture(GL_TEXTURE_2D, _albedoTexture);
//        f_shaderUI_tex->begin();
//        f_shaderUI_tex->setUniformM4fv("MVP", mvp2d);
//        f_shaderUI_tex->setUniform4fv("u_color", COLOR_WHITE);
//        f_shaderUI_tex->setUniform1fv("texScale", 2.0f);
//        glBindVertexArray(square2D_vao);
//        glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
//        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//        f_shaderUI_tex->setUniform1fv("texScale", 1.0f);
//        f_shaderUI_tex->end();
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
glm::mat4 RendererGLProg::GetLightMVP(LightInstance& light)
{
    if (light.type == Light_Type_Directional)
	{
        // Camera projection matrices
        glm::mat4 model = glm::mat4();
        glm::mat4 proj = glm::mat4();
        model = glm::rotate(model, -_camera->rotation.x, glm::vec3(1.0, 0.0, 0.0));
        model = glm::rotate(model, -_camera->rotation.y, glm::vec3(0.0, 1.0, 0.0));
        model = glm::rotate(model, -_camera->rotation.z, glm::vec3(0.0, 0.0, 1.0));
        model = glm::translate(model, glm::vec3(-_camera->position.x, -_camera->position.y, -_camera->position.z));
        glm::vec4 viewport = glm::vec4(0, 0, windowWidth, windowHeight);
        GLfloat aspectRatio = (windowWidth > windowHeight) ?
        GLfloat(windowWidth)/GLfloat(windowHeight) : GLfloat(windowHeight)/GLfloat(windowWidth);
        proj = glm::perspective(_camera->fieldOfView, aspectRatio, _camera->nearDepth, _camera->farDepth);
        
        
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
        max += glm::vec3(_camera->farDepth*0.2f);
        min -= glm::vec3(_camera->farDepth*0.2f);
        
        // Compute the MVP matrix from the light's point of view
        glm::mat4 lightProjectionMatrix = glm::ortho<float>(min.x,max.x,min.y,max.y,-max.z,-min.z);
        glm::mat4 lightMVP = lightProjectionMatrix * lightViewMatrix * lightModelMatrix;
        return lightMVP;
    }
	else if (light.type == Light_Type_Spot &&
		light.position.w > 0.0f ) 
	{
        glm::vec3 lightPos = glm::vec3(light.position.x,light.position.y,light.position.z);
        glm::mat4 lightViewMatrix = glm::lookAt(glm::normalize(-light.direction), glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0,1,0));
        glm::mat4 lightModelMatrix = glm::translate(glm::mat4(1.0f), -lightPos);
        glm::mat4 lightProjectionMatrix = glm::perspective(light.spotCutoff*2.0f, 1.0f, 0.05f, light.position.w);
        glm::mat4 lightMVP = lightProjectionMatrix * lightViewMatrix * lightModelMatrix;
        return lightMVP;
    }
    return glm::mat4(1.0f);
}

glm::mat4 RendererGLProg::GetLightModel(LightInstance& light) {
    glm::vec3 lightPos = glm::vec3(light.position.x,light.position.y,light.position.z);
    return glm::translate(glm::mat4(1.0f), -lightPos);
}
glm::mat4 RendererGLProg::GetLightView(LightInstance& light) {
    if (light.type == Light_Type_Directional)
	{
        glm::vec3 lightInvDir = glm::vec3(light.position.x,light.position.y,light.position.z);
        glm::mat4 lightViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,1.0f,0.0f));
        return lightViewMatrix;
    } else if ( light.type == Light_Type_Spot ) {
        glm::mat4 lightViewMatrix = glm::lookAt(glm::normalize(-light.direction), glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0,1,0));
        return lightViewMatrix;
    }
    return glm::mat4(1.0f);
}

glm::mat4 RendererGLProg::GetLightProjection(LightInstance& light)
{
    if ( light.type == Light_Type_Directional) 
	{
        // Camera projection matrices
        glm::mat4 model = glm::mat4();
        glm::mat4 proj = glm::mat4();
        model = glm::rotate(model, -_camera->rotation.x, glm::vec3(1.0, 0.0, 0.0));
        model = glm::rotate(model, -_camera->rotation.y, glm::vec3(0.0, 1.0, 0.0));
        model = glm::rotate(model, -_camera->rotation.z, glm::vec3(0.0, 0.0, 1.0));
        model = glm::translate(model, glm::vec3(-_camera->position.x, -_camera->position.y, -_camera->position.z));
        glm::vec4 viewport = glm::vec4(0, 0, windowWidth, windowHeight);
        GLfloat aspectRatio = (windowWidth > windowHeight) ?
        GLfloat(windowWidth)/GLfloat(windowHeight) : GLfloat(windowHeight)/GLfloat(windowWidth);
        proj = glm::perspective(_camera->fieldOfView, aspectRatio, _camera->nearDepth, _camera->farDepth);
        
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
        max += glm::vec3(_camera->farDepth*0.1f);
        min -= glm::vec3(_camera->farDepth*0.1f);
        
        // Compute the MVP matrix from the light's point of view
        glm::mat4 lightProjectionMatrix = glm::ortho<float>(min.x,max.x,min.y,max.y,-max.z,-min.z);
        return lightProjectionMatrix;
    } else if ( light.type == Light_Type_Spot )
	{
        glm::mat4 lightProjectionMatrix = glm::perspective(light.spotCutoff*2.0f, 1.0f, 0.05f, light.position.w);
        return lightProjectionMatrix;
    }
}
//
//// Get lights from LightSystem3D
//void RendererGLProg::RenderLighting(
//	const glm::mat4& model,
//	const glm::mat4& projection,
//	const glm::vec4& viewPort,
//	const glm::vec3& position,
//	const glm::vec2& ratio,
//	const float nearDepth,
//	const float farDepth)
//{
//	const int width = viewPort.z;
//	const int height = viewPort.w;
//	// Parameters for linearizing depth value
//	glm::vec2 depthParameter = glm::vec2(
//		farDepth / (farDepth - nearDepth),
//		farDepth * nearDepth / (nearDepth - farDepth));
//
//    bool renderFog = _options->getOption<bool>("r_renderFog");
//    bool shadowMultitap = _options->getOption<bool>("r_shadowMultitap");
//    bool shadowNoise = _options->getOption<bool>("r_shadowNoise");
//    bool shadows = _options->getOption<bool>("r_shadows");
//    float fogDensity = _options->getOption<float>("r_fogDensity");
//    float fogHeightFalloff = _options->getOption<float>("r_fogHeightFalloff");
//    float fogExtinctionFalloff = _options->getOption<float>("r_fogExtinctionFalloff");
//    float fogInscatteringFalloff = _options->getOption<float>("r_fogInscatteringFalloff");
//
//    // Output to final image FBO
//    glBindFramebuffer(GL_FRAMEBUFFER, final_fbo);
//    glClear(GL_COLOR_BUFFER_BIT);
//    glDisable(GL_DEPTH_TEST);
//    glDepthMask(GL_FALSE);
//	glDisable(GL_BLEND);
//
//    // Draw sky layer without lighting and opaque layer in black
//    glEnable(GL_STENCIL_TEST);
//    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
//    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
//    glStencilFunc(GL_EQUAL, Stencil_Sky, 0xFF);             // Only draw sky layer
//    RenderFromTexture(_gBuffer.GetAlbedo());
//    glStencilFunc(GL_EQUAL, Stencil_Solid, 0xFF);           // Only draw solid layer
//    Draw2DRect(glm::vec2(), width, height, COLOR_NONE, COLOR_BLACK, 0.0f);
//    glDisable(GL_STENCIL_TEST);
//
//
//	std::shared_ptr<Shader> lightShader = _lighting->getShader();
//    lightShader->begin();
//    lightShader->setUniform1iv("albedoMap", 0);
//    lightShader->setUniform1iv("materialMap", 1);
//    lightShader->setUniform1iv("normalMap", 2);
//    lightShader->setUniform1iv("depthMap", 3);
//	lightShader->setUniform1iv("aoMap", 4);
//	lightShader->setUniform1iv("noiseMap", 5);
//	lightShader->setUniform1iv("cubeMap", 6);
//	lightShader->setUniform1fv("reflectionSize", _reflectionProbe.getSize());
//	lightShader->setUniform3fv("reflectionPos", _reflectionProbe.getPosition());
//    lightShader->setUniform2fv("depthParameter", depthParameter);
//    lightShader->setUniform1fv("farDepth", farDepth);
//    lightShader->setUniform1fv("nearDepth", nearDepth);
//    lightShader->setUniform3fv("camPos", position);
//	lightShader->setUniform1iv("renderSSAO", _options->getOption<bool>("r_renderSSAO"));
//	lightShader->setUniform1iv("renderFog", renderFog);
//    lightShader->setUniform1fv("fogDensity", fogDensity);
//    lightShader->setUniform1fv("fogHeightFalloff", fogHeightFalloff);
//    lightShader->setUniform1fv("fogExtinctionFalloff", fogExtinctionFalloff);
//    lightShader->setUniform1fv("fogInscatteringFalloff", fogInscatteringFalloff);
//    //lightShader->setUniform3fv("fogColor", _fogColor.r,_fogColor.g,_fogColor.b);
//
//    // Frustum far plane corner coordinates
//    glm::vec3 viewVerts[4];
//    const float cz = 1.0f;
//    viewVerts[0] = glm::unProject(glm::vec3(0.0f, 0.0f, cz), model, projection, viewPort);
//    viewVerts[1] = glm::unProject(glm::vec3(width, 0.0f, cz), model, projection, viewPort);
//    viewVerts[2] = glm::unProject(glm::vec3(width, height, cz), model, projection, viewPort);
//    viewVerts[3] = glm::unProject(glm::vec3(0.0f, height, cz), model, projection, viewPort);
//
//	const GLfloat texCoords[] = {
//		0.0, 0.0,
//		ratio.x, 0.0,
//		ratio.x, ratio.y,
//		0.0, ratio.y,
//	};
//
//    // Prepare VAO for light render
//    glBindVertexArray(vertex_vao);
//    glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(square2D_coords) + sizeof(texCoords) + (sizeof(GLfloat) * 3 * 4), NULL, GL_DYNAMIC_DRAW);
//    // Vertices & texcoords
//	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(square2D_coords), square2D_coords);
//    glBufferSubData(GL_ARRAY_BUFFER, sizeof(square2D_coords), sizeof(texCoords), texCoords);
//    glBufferSubData(GL_ARRAY_BUFFER, sizeof(square2D_coords)+sizeof(texCoords), sizeof(GLfloat)*3*4, viewVerts);
//    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
//    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(sizeof(square2D_coords)));
//    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(sizeof(square2D_coords)+sizeof(texCoords)));
//    glEnableVertexAttribArray(0);
//    glEnableVertexAttribArray(1);
//    glEnableVertexAttribArray(2);
//    glActiveTexture(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_2D, _gBuffer.GetAlbedo());
//    glActiveTexture(GL_TEXTURE1);
//    glBindTexture(GL_TEXTURE_2D, _gBuffer.GetMaterial());
//    glActiveTexture(GL_TEXTURE2);
//    glBindTexture(GL_TEXTURE_2D, _gBuffer.GetNormal());
//    glActiveTexture(GL_TEXTURE3);
//    glBindTexture(GL_TEXTURE_2D, _gBuffer.GetDepth());
//    glActiveTexture(GL_TEXTURE4);
//    glBindTexture(GL_TEXTURE_2D, ao_texture);
//	glActiveTexture(GL_TEXTURE5);
//	tex_ssao_noise->Bind();
//	glActiveTexture(GL_TEXTURE6);
//	glBindTexture(GL_TEXTURE_CUBE_MAP, _reflectionProbe.getCubeMap());
//
//    // Ready stencil to draw lighting only over solid geometry
//    glEnable(GL_STENCIL_TEST);
//    glStencilFunc(GL_LEQUAL, Stencil_Solid, 0xFF);        // Only draw on solid layer
//    glEnable(GL_BLEND);
//    glBlendFunc(GL_ONE, GL_ONE);
//    
//	const float globalTime = Timer::RunTimeSeconds();
//	// Render all lights
//	for (int i = 0; i < _lightsQueue.size(); i++) {
//		LightInstance& light = _lightsQueue[i];
//		if (!light.active) continue;
//		lightShader->setUniform4fv("lightPosition", light.position);
//		lightShader->setUniform4fv("lightColor", light.color);
//		lightShader->setUniform3fv("lightAttenuation", light.attenuation);
//		lightShader->setUniform3fv("lightSpotDirection", light.direction);
//		lightShader->setUniform1fv("lightSpotCutoff", light.spotCutoff);
//		lightShader->setUniform1fv("lightSpotExponent", light.spotExponent);
//		lightShader->setUniform1fv("globalTime", globalTime);
//		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//	}
//	glBindVertexArray(0);
//
//	lightShader->end();
//    glActiveTexture(GL_TEXTURE6); glBindTexture(GL_TEXTURE_2D, 0);
//    glActiveTexture(GL_TEXTURE5); glBindTexture(GL_TEXTURE_2D, 0);
//    glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, 0);
//    glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, 0);
//    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, 0);
//    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);
//    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
//    glDisable(GL_STENCIL_TEST);
//}

void RendererGLProg::RenderLightingFX(
	const glm::mat4& model,
	const glm::mat4& projection,
	const glm::vec4& viewPort,
	const glm::vec3& position,
	const glm::vec2& ratio,
	const float nearDepth,
	const float farDepth)
{
	const int width = viewPort.z;
	const int height = viewPort.w;
	glm::vec2 depthParameter = glm::vec2(
		farDepth / (farDepth - nearDepth),
		farDepth * nearDepth / (nearDepth - farDepth));

	glm::mat4 localModel = glm::translate(model, glm::vec3(-position.x, -position.y, -position.z));
	bool renderLightDots = _options->getOption<bool>("r_lightDots");
	bool renderLightBlur = _options->getOption<bool>("r_lightBlur");
	bool renderLightRays = _options->getOption<bool>("r_lightRays");
	bool renderLightFlare = _options->getOption<bool>("r_lightFlare");

	if (!renderLightDots) { return; }
	// Copy depth+stencil buffer at lower res to light FBO
	glDepthMask(GL_TRUE);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, final_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, light_fbo);
	glBlitFramebuffer(0, 0, width, height,
		0, 0, width*LIGHT_SCALE, height*LIGHT_SCALE,
		GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
	glDepthMask(GL_FALSE);
	return;


	//glBindFramebuffer(GL_FRAMEBUFFER, light_fbo);
	//glClear(GL_STENCIL_BUFFER_BIT);   // Clear light stencil
	// ------------ LIGHTING STAGE 2 ------------------------
	// 2D Radial beam lights in 3D / add lens flare
	for (int i = 0; i<_lightsQueue.size(); i++)
	{
		LightInstance& light = _lightsQueue[i];
		if (!light.active) continue;
		glm::vec3 lightPosition = glm::vec3(light.position.x, light.position.y, light.position.z);
		float radius = 8.0f; // 8 pixel radius minimum?
		if (light.type == Light_Type_Directional)
		{
			lightPosition = position + (lightPosition*100.0f);
		}
		glm::vec3 lightScrnPos = glm::project(lightPosition, localModel, projection, viewPort);
		glm::vec2 lightScrn2D = glm::vec2(lightScrnPos.x, lightScrnPos.y);
		if (lightScrnPos.z < 1.0005f)
		{
			// Fix light screen position relative to light texture
			lightScrn2D *= (float)LIGHT_SCALE;
			lightScrn2D -= glm::vec2(width*0.5f, height*0.5f);
			// Render simple sun circle into light buffer 0
			glBindFramebuffer(GL_FRAMEBUFFER, light_fbo);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, light_textures[0], 0);
			glClear(GL_COLOR_BUFFER_BIT);
			const float lightDrawScale = 8.0f;

			if (light.position.w == 0.0f) {   // Light has no radius, we give it an arbitrary one
				radius = 32.0f; // Default sun radius
			}
			else {
				float Z = depthParameter.y / (depthParameter.x - lightScrnPos.z);
				// Scale light radius by radius/1.0-depth to keep constant relative size
				radius = light.position.w*lightDrawScale / (1.0 - Z);
			}
			//if ( radius < 4.0f ) continue;
			radius = fmaxf(radius, 40.0f);
			radius = fminf(radius, 1024.0f);
			glEnable(GL_DEPTH_TEST);
			float lightZ = ORTHO_FARDEPTH + (ORTHO_NEARDEPTH - ORTHO_FARDEPTH)*fminf(lightScrnPos.z, 1.0f);

			//Log::Debug("Light screen position: %f,%f,%f, ortho Z: %f", lightScrnPos.x, lightScrnPos.y, lightScrnPos.z, lightZ);
			//Log::Debug("Light radius: %f, scrn radius: %f", light.position.w, radius);

			Color lightDotColor = RGBAColor(light.color.r, light.color.g, light.color.b, 1.0);
			// Draw a simple 2D circle at light position
			DrawCircle(lightScrn2D, 0.0f, radius*LIGHT_SCALE, COLOR_NONE, lightDotColor, lightZ, 1);
			glDisable(GL_DEPTH_TEST);

			//if (renderLightBlur)
			//{
			//	// Blur horizontally - narrow (tex 0 to 3)
			//	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, light_textures[3], 0);
			//	glClear(GL_COLOR_BUFFER_BIT);
			//	glActiveTexture(GL_TEXTURE0);
			//	glBindTexture(GL_TEXTURE_2D, light_textures[0]);
			//	d_shaderBlurH->begin();
			//	d_shaderBlurH->setUniform1iv("Width", 1);
			//	d_shaderBlurH->setUniform1fv("odw", 1.0f / (windowWidth*LIGHT_SCALE));
			//	glBindVertexArray(square2D_vao);
			//	glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
			//	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			//	d_shaderBlurH->end();
			//	glBindVertexArray(0);
			//	// Blur horizontally - narrow (tex 3 to 1)
			//	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, light_textures[1], 0);
			//	glClear(GL_COLOR_BUFFER_BIT);
			//	glActiveTexture(GL_TEXTURE0);
			//	glBindTexture(GL_TEXTURE_2D, light_textures[3]);
			//	d_shaderBlurV->begin();
			//	d_shaderBlurV->setUniform1iv("Width", 1);
			//	d_shaderBlurV->setUniform1fv("odh", 1.0f / (windowHeight*LIGHT_SCALE));
			//	glBindVertexArray(square2D_vao);
			//	glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
			//	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			//	d_shaderBlurV->end();
			//	glBindVertexArray(0);
			//	if (!renderLightRays || !light.rayCaster)
			//	{
			//		// Blur horizontally - wide (tex 0 to 3)
			//		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, light_textures[3], 0);
			//		glClear(GL_COLOR_BUFFER_BIT);
			//		glActiveTexture(GL_TEXTURE0);
			//		glBindTexture(GL_TEXTURE_2D, light_textures[0]);
			//		d_shaderBlurH->begin();
			//		d_shaderBlurH->setUniform1iv("Width", 10);
			//		d_shaderBlurH->setUniform1fv("odw", 1.0f / (width*LIGHT_SCALE));
			//		glBindVertexArray(square2D_vao);
			//		glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
			//		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			//		d_shaderBlurH->end();
			//		glBindVertexArray(0);
			//		// Blur vertically - wide (tex 3 to 2)
			//		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, light_textures[2], 0);
			//		glActiveTexture(GL_TEXTURE0);
			//		glClear(GL_COLOR_BUFFER_BIT);
			//		glBindTexture(GL_TEXTURE_2D, light_textures[3]);
			//		d_shaderBlurV->begin();
			//		d_shaderBlurV->setUniform1iv("Width", 10);
			//		d_shaderBlurV->setUniform1fv("odh", 1.0f / (height*LIGHT_SCALE));
			//		glBindVertexArray(square2D_vao);
			//		glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
			//		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			//		d_shaderBlurV->end();
			//		glBindVertexArray(0);
			//	}
			//}
			//if (light.rayCaster && renderLightRays)
			//{
			//	// Radial light beams
			//	// Update sun into light buffer tex 2
			//	glBindFramebuffer(GL_FRAMEBUFFER, light_fbo);
			//	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, light_textures[2], 0);
			//	glViewport(0, 0, width*LIGHT_SCALE, height*LIGHT_SCALE);
			//	glClear(GL_COLOR_BUFFER_BIT);
			//	glm::mat4 mvp2d = glm::ortho<GLfloat>(-0.5, 0.5, -0.5, 0.5, -1.0, 1.0);
			//	glm::vec2 lightScrnUnit = glm::vec2(lightScrnPos.x, lightScrnPos.y) / glm::vec2(width, height);
			//	d_shaderLightRays->begin();
			//	d_shaderLightRays->setUniformM4fv("MVP", mvp2d);
			//	//d_shaderLightRays->setUniform4fv( "u_color" , COLOR_WHITE);
			//	d_shaderLightRays->setUniform2fv("lightScrnPos", lightScrnUnit);
			//	d_shaderLightRays->setUniform1fv("exposure", lr_exposure);
			//	d_shaderLightRays->setUniform1fv("decay", lr_decay);
			//	d_shaderLightRays->setUniform1fv("density", lr_density);
			//	d_shaderLightRays->setUniform1fv("weight", lr_weight);
			//	glActiveTexture(GL_TEXTURE0);
			//	if (renderLightBlur) {
			//		glBindTexture(GL_TEXTURE_2D, light_textures[1]);    // Read from blurred light colors
			//	}
			//	else {
			//		glBindTexture(GL_TEXTURE_2D, light_textures[0]);    // Read from light colors
			//	}
			//	glBindVertexArray(square2D_vao);
			//	glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
			//	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);    // Blend light in with regular picture
			//	d_shaderLightRays->end();
			//	glViewport(0, 0, width, height);    // reset viewport
			//}
			//if (renderLightFlare)
			//{
			//	// blur sun sphere radially and calculate lens flare and halo and apply dirt texture (to tex 3)
			//	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, light_textures[3], 0);
			//	glClear(GL_COLOR_BUFFER_BIT);
			//	if (renderLightBlur) {
			//		glActiveTexture(GL_TEXTURE0);
			//		glBindTexture(GL_TEXTURE_2D, light_textures[1]);
			//		glActiveTexture(GL_TEXTURE1);
			//		glBindTexture(GL_TEXTURE_2D, light_textures[2]);
			//	}
			//	else {
			//		glActiveTexture(GL_TEXTURE0);
			//		glBindTexture(GL_TEXTURE_2D, light_textures[0]);
			//		glActiveTexture(GL_TEXTURE1);
			//		if (renderLightRays) {
			//			glBindTexture(GL_TEXTURE_2D, light_textures[2]);
			//		}
			//		else {
			//			glBindTexture(GL_TEXTURE_2D, light_textures[0]);
			//		}
			//	}
			//	glActiveTexture(GL_TEXTURE2);
			//	tex_lens_dirt->Bind();
			//	d_shaderSunPP->begin();
			//	glm::vec2 lightScrnUnit = glm::vec2(lightScrnPos.x, lightScrnPos.y) / glm::vec2(width, height);
			//	d_shaderSunPP->setUniform2fv("SunPosProj", lightScrnUnit);
			//	glBindVertexArray(square2D_vao);
			//	glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
			//	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			//	d_shaderSunPP->end();
			//	glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, 0);
			//	glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);
			//	glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
			//}
			// Render final light to FBO
			glBindFramebuffer(GL_FRAMEBUFFER, final_fbo);
			//glViewport(0, 0, width, height);
			//glEnable(GL_BLEND);
			//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
			//const glm::vec2 lightRes = glm::vec2(windowWidth*LIGHT_SCALE, windowHeight*LIGHT_SCALE);
			//Rect2D tRect = Rect2D(0.0f, 0.0f, 1.0, 1.0);

			//glm::mat4 mvp = glm::ortho<GLfloat>(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
			//if (renderLightFlare)
			//{
			//	DrawTexture(Rect2D(0, 0, 1.0f, 1.0f), tRect, light_textures[3], mvp);
			//}
			//else
			//{
			//	if (renderLightRays || renderLightBlur)
			//	{
			//		DrawTexture(Rect2D(0, 0, 1.0f, 1.0f), tRect, light_textures[2], mvp);
			//	}
			//	else
			//	{
			//		DrawTexture(Rect2D(0, 0, 1.0f, 1.0f), tRect, light_textures[0], mvp);
			//	}
			//}
		}
	}	// for all lights
	glBindVertexArray(0);
}

GLuint RendererGLProg::addVertexArray()
{
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	_vertArrays.push_back(vao);

	return vao;
}

std::shared_ptr<VertBuffer> RendererGLProg::addVertBuffer(const VertexDataType type)
{
	auto buffer = std::make_shared<VertBuffer>(type);
	buffer->bind();

	 if (type == MeshVerts)
	{
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), 0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (GLvoid*)(4 * sizeof(GLfloat)));
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (GLvoid*)(8 * sizeof(GLfloat)));
	}
	else if (type == TexturedVerts)
	{
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(TexturedVertexData), 0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertexData), (GLvoid*)(4 * sizeof(GLfloat)));
	}
	else if (type == SphereVerts)
	{
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(SphereVertexData), 0);
		glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(SphereVertexData), (GLvoid*)(4 * sizeof(GLfloat)));
	}
	else if (
		type == ColorVerts ||
		type == SpriteVerts)
	{
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(4 * sizeof(GLfloat)));
	}
	else if (type == InstanceVerts)
	{
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), 0);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (GLvoid*)(3 * sizeof(GLfloat)));
		glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (GLvoid*)(7 * sizeof(GLfloat)));
		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
	}
	else if (type == InstancedCubeVerts)
	{
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(CubeInstance), 0);	// X, Y, Z, size
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(CubeInstance), (GLvoid*)(4 * sizeof(GLfloat)));	// Rotation Quat
		glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(CubeInstance), (GLvoid*)(8 * sizeof(GLfloat)));	// Material
		glVertexAttribDivisorARB(2, 1);
		glVertexAttribDivisorARB(3, 1);
		glVertexAttribDivisorARB(4, 1);
	}
	_vertBuffers[buffer->getVBO()] = buffer;
	return buffer;
}

void RendererGLProg::queueMesh(std::shared_ptr<Mesh> mesh)
{
	_renderQueueDeferred.push_back(mesh);
}

void RendererGLProg::queueDeferredBuffer(
	const VertexDataType type,
	const GLuint buffer,
	const unsigned int rangeEnd,
	const unsigned int rangeStart,
	const GLuint tex,
	const BlendMode blendMode,
	const DepthMode depthMode)
{
	DrawPackage package = {
		type,
		buffer,
		rangeEnd,
		rangeStart,
		tex,
		blendMode,
		depthMode,
		true
	};
	_deferredQueue.push_back(package);
}

void RendererGLProg::queueForwardBuffer(
	const VertexDataType type,
	const GLuint buffer,
	const unsigned int rangeEnd,
	const unsigned int rangeStart,
	const GLuint tex,
	const BlendMode blendMode,
	const DepthMode depthMode,
	const bool render3D)
{
	DrawPackage package = {
		type,
		buffer,
		rangeEnd,
		rangeStart,
		tex,
		blendMode,
		depthMode,
		render3D
	};
	_forwardQueue.push_back(package);
}

void RendererGLProg::queueDeferredInstances(
	const GLuint instanceBuffer,
	const unsigned int instanceCount, 
	const VertexDataType type,
	const GLuint buffer,
	const unsigned int rangeEnd,
	const unsigned int rangeStart,
	const GLuint tex,
	const BlendMode blendMode,
	const DepthMode depthMode)
{
	InstancedDrawPackage package = {
		instanceBuffer,
		instanceCount,
		type,
		buffer,
		rangeEnd,
		rangeStart,
		tex,
		blendMode,
		depthMode
	};
	_deferredInstanceQueue.push_back(package);
}

void RendererGLProg::queueLights(
	const LightInstance * lights,
	const unsigned int lightCount)
{
	for (size_t i = 0; i < lightCount; i++)
	{
		_lightsQueue.push_back(lights[i]);
	}
}

void RendererGLProg::renderVertBuffer(
	std::shared_ptr<VertBuffer> buffer,
	const unsigned int rangeEnd,
	const unsigned int rangeStart,
	const Texture* tex,
	const bool render3D)
{
	if (buffer->getType() == MeshVerts)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _materialTexture.getTexture());

		glBindVertexArray(_mesh_vao);
		buffer->bind();
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), 0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (GLvoid*)(4 * sizeof(GLfloat)));
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (GLvoid*)(8 * sizeof(GLfloat)));
		_shaderDeferredMesh->begin();
		_shaderDeferredMesh->setUniformM4fv("MVP", mvp3D);
		_shaderDeferredMesh->setUniform1iv("materialTexture", 0);
		glDrawArrays(GL_TRIANGLES, rangeStart, rangeEnd);
		_shaderDeferredMesh->end();
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		renderedSprites = rangeEnd;
	}
	else if (buffer->getType() == SpriteVerts)
	{
		if (tex != NULL)
		{
			glActiveTexture(GL_TEXTURE0);
			tex->Bind();
		}

		glBindVertexArray(_sprite_vao);
		buffer->bind();
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(4 * sizeof(GLfloat)));
		if (render3D)
		{
			_shaderDeferredSprite->begin();
		}
		else
		{
			f_shaderSprite->begin();
			f_shaderSprite->setUniformM4fv("View", glm::mat4());
			f_shaderSprite->setUniformM4fv("Projection", mvp2D);
			f_shaderSprite->setUniform3fv("CameraPosition", glm::vec3());
		}
		glDrawArrays(GL_POINTS, rangeStart, rangeEnd);
		_shaderDeferredSprite->end();
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		renderedSprites = rangeEnd;
	}

	if (tex != NULL)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void RendererGLProg::BufferSpheres(
	const SphereVertexData *spheres,
	const int numSpheres)
{
	_sphereData.buffer(spheres, numSpheres);
}

void RendererGLProg::RenderVerts()
{
    //if ( numBufferedColorVerts ) {
    //    f_shaderDefault_vColor->begin();
    //    f_shaderDefault_vColor->setUniformM4fv("MVP", mvp3D);
    //    glBindVertexArray(colorVerts_vao);
    //    glBindBuffer(GL_ARRAY_BUFFER, colorVerts_vbo);
    //    glBufferData(GL_ARRAY_BUFFER, numBufferedColorVerts * sizeof(ColorVertexData), vertBufferColor, GL_DYNAMIC_DRAW);
    //    glDrawArrays(GL_TRIANGLES, 0, numBufferedColorVerts);
    //    glBindVertexArray(0);
    //    f_shaderDefault_vColor->end();
    //    renderedTris += numBufferedColorVerts/3;
    //    numBufferedColorVerts = 0;
    //}
}

void RendererGLProg::renderSpheres(
	const glm::mat4& view,
	const glm::mat4& projection,
	const glm::mat3& normalMatrix,
	const glm::vec3& position)
{

    if (_sphereData.getCount() > 0)
	{
		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _materialTexture.getTexture());
		glBindVertexArray(_sphere_vao);

		_shaderDeferredSphere->begin();
		_shaderDeferredSphere->setUniformM4fv("View", view);
		_shaderDeferredSphere->setUniformM4fv("Projection", projection);
		_shaderDeferredSphere->setUniformM3fv("NormalMatrix", normalMatrix);
		_shaderDeferredSphere->setUniform3fv("CameraPosition", position);
		glDrawArrays(GL_POINTS, 0, _sphereData.getCount());
        _shaderDeferredSphere->end();
        glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);

        renderedSprites += _sphereData.getCount();
    }
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

void RendererGLProg::Render2DLines()
{
    if ( numBuffered2DLines == 0 ) return;
    
    f_shaderDefault_vColor->begin();
    f_shaderDefault_vColor->setUniformM4fv("MVP", mvp2D);

    glBindVertexArray(lines_vao);
    glBindBuffer(GL_ARRAY_BUFFER, lines_vbo);
    // Buffer instance data to GPU
    glBufferData(GL_ARRAY_BUFFER, sizeof(ColorVertexData)*numBuffered2DLines, lineBuffer2D, GL_STATIC_DRAW);
    glDrawArrays(GL_LINES, 0, numBuffered2DLines);
    f_shaderDefault_vColor->end();
    glBindVertexArray(0);
    renderedSegs += numBuffered2DLines/2;
    numBuffered2DLines = 0;
}

void RendererGLProg::DrawPolygon(const int count,
                                 const GLfloat *verts,
                                 const Color lineColor,
                                 const Color fillColor)
{
    f_shaderUI_color->begin();
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
    f_shaderUI_color->end();
    glBindVertexArray(0);
}

void RendererGLProg::DrawPolygon(const int count,
                                 const glm::vec2 *verts,
                                 const Color lineColor,
                                 const Color fillColor,
                                 const float z)
{
    f_shaderUI_color->begin();
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
    f_shaderUI_color->end();
    glBindVertexArray(0);
    delete [] vertices;
}

void RendererGLProg::Draw2DRect(Rect2D rect,
                                Color lineColor,
                                Color fillColor,
                                float z)
{
    GLfloat verts[] = {
        rect.x           , rect.y,              z, 1,
        rect.x+(rect.w-1), rect.y,              z, 1,
        rect.x+(rect.w-1), rect.y+(rect.h-1),   z, 1,
        rect.x           , rect.y+(rect.h-1),   z, 1,
    };
    DrawPolygon(4, verts, lineColor, fillColor);
}

void RendererGLProg::Draw2DRect(glm::vec2 center,
                                float width,
                                float height,
                                Color lineColor,
                                Color fillColor,
                                float z)
{
    GLfloat hw = (GLfloat)(width*0.5f);
    GLfloat hh = (GLfloat)(height*0.5f);
    GLfloat verts[] = {
        -hw + center.x, -hh + center.y, z, 1,
        +hw + center.x, -hh + center.y, z, 1,
        +hw + center.x, +hh + center.y, z, 1,
        -hw + center.x, +hh + center.y, z, 1,
    };
    DrawPolygon(4, verts, lineColor, fillColor);
}

void RendererGLProg::Draw2DRect3D(glm::vec3 center,
                                  float width,
                                  float height,
                                  Color lineColor,
                                  Color fillColor,
                                  float z)
{
    
    glm::mat4 model = glm::mat4();
    model = glm::rotate(model, -_camera->rotation.x, glm::vec3(1.0, 0.0, 0.0));
    model = glm::rotate(model, -_camera->rotation.y, glm::vec3(0.0, 1.0, 0.0));
    model = glm::rotate(model, -_camera->rotation.z, glm::vec3(0.0, 0.0, 1.0));
    glm::mat4 proj = glm::mat4();
    GLfloat aspectRatio = (windowWidth > windowHeight) ?
    GLfloat(windowWidth)/GLfloat(windowHeight) : GLfloat(windowHeight)/GLfloat(windowWidth);
    proj = glm::perspective(_camera->fieldOfView, aspectRatio, _camera->nearDepth, _camera->farDepth);
    int ww2 = windowWidth/2;
    int wh2 = windowHeight/2;
    glm::vec4 viewport = glm::vec4(0, 0, windowWidth, windowHeight);
    //  Calculate screen position of rectangle
    glm::vec3 scrnPos = glm::project(center, model, proj, viewport);
    scrnPos += glm::vec3(-ww2,-wh2,0.0f);
    Draw2DRect(glm::vec2(scrnPos.x,scrnPos.y), width, height, lineColor, fillColor, scrnPos.z);
}

void RendererGLProg::Draw2DProgressBar(glm::vec3 center,
                                       float width,
                                       float height,
                                       float amount,
                                       Color lineColor,
                                       Color fillColor,
                                       float z)
{
    
    glm::mat4 model = glm::mat4();
    model = glm::rotate(model, -_camera->rotation.x, glm::vec3(1.0, 0.0, 0.0));
    model = glm::rotate(model, -_camera->rotation.y, glm::vec3(0.0, 1.0, 0.0));
    model = glm::rotate(model, -_camera->rotation.z, glm::vec3(0.0, 0.0, 1.0));
    model = glm::translate(model, glm::vec3(-_camera->position.x, -_camera->position.y, -_camera->position.z));
    glm::mat4 proj = glm::mat4();
    GLfloat aspectRatio = (windowWidth > windowHeight) ?
    GLfloat(windowWidth)/GLfloat(windowHeight) : GLfloat(windowHeight)/GLfloat(windowWidth);
    proj = glm::perspective(_camera->fieldOfView, aspectRatio, _camera->nearDepth, _camera->farDepth);
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
    f_shaderUI_color->begin();
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
    f_shaderUI_color->end();
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
    f_shaderUI_color->begin();
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
    f_shaderUI_color->end();
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
	d_shaderDefault_uColor->begin();
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
    d_shaderDefault_uColor->end();
    
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
    
    f_shaderUI_tex->begin();
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
    f_shaderUI_tex->end();
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

    f_shaderUI_tex->begin();
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
    f_shaderUI_tex->end();
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
        f_shaderUI_tex->begin();
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
        f_shaderUI_tex->end();
        glBindVertexArray(0);
        renderedTris += 2;
		glBindTexture(GL_TEXTURE_2D, 0);
    }
}
void RendererGLProg::DrawTexture(
	const Rect2D rect,
	const Rect2D texRect,
	const GLuint tex,
	const glm::mat4& mvp,
	const float z,
	const Color color)
{
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
    f_shaderUI_tex->begin();
    f_shaderUI_tex->setUniformM4fv("MVP", mvp);
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
    f_shaderUI_tex->end();
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

    f_shaderUI_tex->begin();
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
    f_shaderUI_tex->end();
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    renderedTris += 2;
    glDeleteTextures(1, &tempTexture);
}
void RendererGLProg::DrawCubeMap( 
	const Rect2D rect,
	const CubeTexVerts& texCoords,
    const GLuint tex,
	const float z,
	const Color color)
{
    GLfloat vertices[] = {
        rect.x          , rect.y,           z, 1,
        rect.x + rect.w , rect.y,           z, 1,
        rect.x + rect.w , rect.y + rect.h,  z, 1,
        rect.x          , rect.y + rect.h,  z, 1,
    };

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
    f_shaderUI_cubeMap->begin();
    f_shaderUI_cubeMap->setUniformM4fv("MVP", mvp2D);
    f_shaderUI_cubeMap->setUniform4fv("u_color", color);
    f_shaderUI_cubeMap->setUniform1iv("textureMap", 0);
    glBindVertexArray(ui_vao);
    glBindBuffer(GL_ARRAY_BUFFER,ui_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(GLfloat)*12, NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(CubeTexVerts), &texCoords);
    glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 0, 0 );
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)sizeof(vertices) );
    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    f_shaderUI_cubeMap->end();
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
}

void RendererGLProg::render3DLines(const glm::mat4& mvp)
{
    if ( numBuffered3DLines == 0 ) return;
	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	f_shaderUI_vColor->begin();
	f_shaderUI_vColor->setUniformM4fv("MVP", mvp);
    glBindVertexArray(lines_vao);
    glBindBuffer(GL_ARRAY_BUFFER, lines_vbo);
    // Buffer instance data to GPU
    glBufferData(GL_ARRAY_BUFFER, sizeof(ColorVertexData)*numBuffered3DLines, lineBuffer3D, GL_STATIC_DRAW);
    glDrawArrays(GL_LINES, 0, numBuffered3DLines);
    glBindVertexArray(0);
    renderedSegs += numBuffered3DLines/2;
    numBuffered3DLines = 0;
	f_shaderUI_vColor->end();
}

void RendererGLProg::bufferCubes(
	const CubeInstance* cubes,
	const size_t count)
{
	_cubeData.buffer(cubes, count);
}

void RendererGLProg::renderCubes(const glm::mat4& mvp) 
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glDisable(GL_BLEND);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _materialTexture.getTexture());

    d_shaderCube->begin();
    d_shaderCube->setUniformM4fv("MVP", mvp);
	d_shaderCube->setUniform1iv("materialTexture", 0);

	glBindVertexArray(_cube_vao);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, _cubeData.getCount());
    glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
    d_shaderCube->end();
    renderedTris += _cubeData.getCount()*12;
}

void RendererGLProg::DrawBoxOutline(
	glm::vec3 center,
	glm::vec3 boxSize,
	Color color )
{
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
}




const glm::vec3 RendererGLProg::GetCursor3DPos( glm::vec2 cursorPos ) const {
    const int hw = windowWidth/2;
    const int hh = windowHeight/2;

    GLfloat cursorDepth=0;
    // Obtain the Z position (not world coordinates but in range 0 ~ 1)
    glReadPixels(cursorPos.x+hw, cursorPos.y+hh, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &cursorDepth);
    // Grab pixel under cursor color
//    GLfloat col[4];
//    glReadPixels(cursorPos.x, cursorPos.y, 1, 1, GL_RGBA, GL_FLOAT, &col);
//    printf("cursor color:%.1f, %.1f %.1f\n", col[0],col[1],col[2]);
    
    // Prepare matrices to unproject cursor coordinates
    glm::mat4 model = glm::mat4();
    model = glm::rotate(model, -_camera->rotation.x, glm::vec3(1.0, 0.0, 0.0));
    model = glm::rotate(model, -_camera->rotation.y, glm::vec3(0.0, 1.0, 0.0));
    model = glm::rotate(model, -_camera->rotation.z, glm::vec3(0.0, 0.0, 1.0));
    model = glm::translate(model, glm::vec3(-_camera->position.x, -_camera->position.y, -_camera->position.z));
    
    glm::mat4 proj = glm::mat4();
    GLfloat aspectRatio = (windowWidth > windowHeight) ?
    GLfloat(windowWidth)/GLfloat(windowHeight) : GLfloat(windowHeight)/GLfloat(windowWidth);
    proj = glm::perspective(_camera->fieldOfView, aspectRatio, _camera->nearDepth, _camera->farDepth);
    glm::vec4 viewport = glm::vec4(0, 0, windowWidth, windowHeight);
    glm::vec3 crosshairPos  = glm::unProject(glm::vec3(cursorPos.x+hw, cursorPos.y+hh, cursorDepth), model, proj, viewport);
    // Linearize depth
//    glm::vec2 depthParameter = glm::vec2( _camera->farDepth / ( _camera->farDepth - _camera->nearDepth ),
//                                         _camera->farDepth * _camera->nearDepth / ( _camera->nearDepth - _camera->farDepth ) );
//    float Z = depthParameter.y/(depthParameter.x - cursorDepth);

//    printf("Cursor depth: %f, pos: %f, %f, %f\n", Z, crosshairPos.x, crosshairPos.y, crosshairPos.z);
    return crosshairPos;
}

const std::string RendererGLProg::GetInfo() const {
    std::string info = "Renderer: OpenGL Programmable pipeline\n";
    if ( _options->getOption<bool>("r_fullScreen") ) {
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

void RendererGLProg::refreshMaterials()
{
	_materialTexture.setData(_materials);
}

void RendererGLProg::flushDeferredQueue(
	const glm::mat4& view,
	const glm::mat4& projection,
	const glm::vec3& position)
{
	for (DrawPackage& package : _deferredQueue)
	{
		setBlendMode(package.blendMode);
		setDepthMode(package.depthMode);

		if (package.type == MeshVerts)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, _materialTexture.getTexture());

			glBindVertexArray(_mesh_vao);
			glBindBuffer(GL_ARRAY_BUFFER, package.buffer);
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), 0);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (GLvoid*)(4 * sizeof(GLfloat)));
			glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (GLvoid*)(8 * sizeof(GLfloat)));
			_shaderDeferredMesh->begin();
			_shaderDeferredMesh->setUniformM4fv("MVP", projection*view);
			_shaderDeferredMesh->setUniform1iv("materialTexture", 0);
			glDrawArrays(GL_TRIANGLES, package.rangeStart, package.rangeEnd);
			_shaderDeferredMesh->end();
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			renderedTris = (package.rangeStart - package.rangeEnd) / 3;
		}
		else if (package.type == SpriteVerts)
		{
			if (package.texture != 0)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, package.texture);
			}
			glBindVertexArray(_sprite_vao);
			glBindBuffer(GL_ARRAY_BUFFER, package.buffer);
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(4 * sizeof(GLfloat)));
			_shaderDeferredSprite->begin();
			_shaderDeferredSprite->setUniformM4fv("View", view);
			_shaderDeferredSprite->setUniformM4fv("Projection", projection);
			_shaderDeferredSprite->setUniform3fv("CameraPosition", position);
			glDrawArrays(GL_POINTS, package.rangeStart, package.rangeEnd);
			_shaderDeferredSprite->end();
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			renderedSprites = package.rangeEnd;
		}
	}

	for (InstancedDrawPackage& package : _deferredInstanceQueue)
	{
		if (package.type != MeshVerts)
		{
			Log::Error("[RendererGLProg] unsupported instanced mesh vertex data type!");
			continue;
		}
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _materialTexture.getTexture());

		setBlendMode(package.blendMode);
		setDepthMode(package.depthMode);

		glBindVertexArray(instancing_vao);
		// Vertex data binding
		glBindBuffer(GL_ARRAY_BUFFER, package.buffer);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), 0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (GLvoid*)(4 * sizeof(GLfloat)));
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (GLvoid*)(8 * sizeof(GLfloat)));
		// Instance data binding
		glBindBuffer(GL_ARRAY_BUFFER, package.instanceBuffer);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), 0);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (GLvoid*)(3 * sizeof(GLfloat)));
		glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (GLvoid*)(7 * sizeof(GLfloat)));
		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		
		CHECK_GL_ERROR();

		d_shaderInstance->begin();
		d_shaderInstance->setUniformM4fv("MVP", projection*view);
		d_shaderInstance->setUniform1iv("materialTexture", 0);

		CHECK_GL_ERROR();
		glDrawArraysInstanced(GL_TRIANGLES, package.rangeStart, package.rangeEnd, package.instanceCount);
		CHECK_GL_ERROR();
		d_shaderInstance->end();
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		renderedTris += package.instanceCount*((package.rangeEnd - package.rangeStart) / 3);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void RendererGLProg::flushForwardQueue(
	const glm::mat4& view,
	const glm::mat4& projection,
	const glm::vec3& position)
{
	for (DrawPackage& package : _forwardQueue)
	{
		if (package.texture != 0)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, package.texture);
		}
		setBlendMode(package.blendMode);
		setDepthMode(package.depthMode);

		if (package.type == MeshVerts)	// TODO: Create forward rendering shader for mesh data!!!
		{
			glBindVertexArray(_mesh_vao);
			glBindBuffer(GL_ARRAY_BUFFER, package.buffer);
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), 0);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (GLvoid*)(4 * sizeof(GLfloat)));
			glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (GLvoid*)(8 * sizeof(GLfloat)));
			_shaderDeferredMesh->begin();
			glDrawArrays(GL_TRIANGLES, package.rangeStart, package.rangeEnd);
			_shaderDeferredMesh->end();
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			renderedSprites = package.rangeEnd;
		}
		else if (package.type == SpriteVerts)
		{
			glBindVertexArray(_sprite_vao);
			glBindBuffer(GL_ARRAY_BUFFER, package.buffer);
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(4 * sizeof(GLfloat)));
			f_shaderSprite->begin();
			if (package.render3D) {
				f_shaderSprite->setUniformM4fv("View", view);
				f_shaderSprite->setUniformM4fv("Projection", projection);
				f_shaderSprite->setUniform3fv("CameraPosition", position);
			} else {
				f_shaderSprite->setUniformM4fv("View", glm::mat4());
				f_shaderSprite->setUniformM4fv("Projection", mvp2D);
				f_shaderSprite->setUniform3fv("CameraPosition", glm::vec3());
			}

			glDrawArrays(GL_POINTS, package.rangeStart, package.rangeEnd);
			f_shaderSprite->end();
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			renderedSprites = package.rangeEnd;
		}
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void RendererGLProg::setBlendMode(BlendMode mode)
{
	if (mode.enable)
	{
		glEnable(GL_BLEND);
		glBlendFunc(mode.srcFunc, mode.dstFunc);
	}
	else
	{
		glDisable(GL_BLEND);
	}
}

void RendererGLProg::setDepthMode(DepthMode mode)
{
	if (mode.enable)
	{
		glEnable(GL_DEPTH_TEST);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}
	glDepthMask(mode.mask ? GL_TRUE : GL_FALSE);
}
