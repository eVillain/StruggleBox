#include "RendererGLProg.h"

#include "TextureAtlasLoader.h"
#include "ShaderLoader.h"
#include "TextAtlasLoader.h"
#include "GLUtils.h"
#include "GLErrorUtil.h"
#include "RenderUtils.h"
#include "FileUtil.h"
#include "PathUtil.h"
#include "Timer.h"

#include "ArenaOperators.h"
#include "Allocator.h"
#include "Shader.h"
#include "VertBuffer.h"
#include "Mesh.h"
#include "Texture2D.h"
#include "Options.h"
#include "GFXDefines.h"
#include "Light3D.h"
#include "Camera.h"
#include "Frustum.h"
#include "Console.h"
#include "Log.h"

#include <SDL2/SDL.h>
#include <glm/gtx/rotate_vector.hpp>


#define SHADOW_MAP_SIZE 4096
#define SHADOW_CUBE_SIZE 512
const float LIGHT_SCALE = 0.5f;
//const float BLOOM_SCALE = 0.125f;

const int CUBE_BUFFER_MAX = 32 * 32 * 32;
const int SPHERE_BUFFER_MAX = 32 * 32 * 32;
const int SPRITE_BUFFER_MAX = 1024;
const int COLORVERT_BUFFER_MAX = 2048;

const size_t FRAME_ALLOCATOR_SIZE = 64 * 1024 * 1024;

RendererGLProg::RendererGLProg(Allocator& allocator, Options& options, Camera& camera, ThreadPool& threadPool)
    : m_allocator(allocator)
    , m_options(options)
    , m_camera(camera)
    , m_threadPool(threadPool)
    , m_textureLoader(allocator, threadPool)
    , m_frameAllocator(FRAME_ALLOCATOR_SIZE, allocator.allocate(FRAME_ALLOCATOR_SIZE))
    , m_lightSystem3D()
    , m_cubeBuffer(CUBE_BUFFER_MAX, VertexDataType::StaticCubeVertsM, VertexDataType::InstancedCubeVerts, allocator)
    , m_colorCubeBuffer(512, VertexDataType::StaticCubeVertsC,  VertexDataType::InstancedColorCubeVerts, allocator)
    , m_sphereBuffer(SPHERE_BUFFER_MAX, VertexDataType::SphereVerts, allocator)
    , m_fireBallBuffer(SPHERE_BUFFER_MAX, VertexDataType::FireballVerts, allocator)
    , renderMode(RM_Final_Image)
    , vertex_vbo(0)
    , vertex_vao(0)
    , lines_vbo(0)
    , lines_vao(0)
    , numBuffered2DLines(0)
    , numBuffered3DLines(0)
{
	Log::Info("[Renderer] constructor, instance at %p", this);
}

RendererGLProg::~RendererGLProg()
{
	Log::Info("[Renderer] destructor, instance at %p", this);
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
    
	m_gBuffer.Initialize(windowWidth, windowHeight);
	SetupShaders();     // Setup all our shaders and render buffers
    SetupRenderBuffers();
    SetupGeometry();
	_materials.load(PathUtil::MaterialsPath() + "default.plist");
	refreshMaterials();

    m_lightSystem3D.initialize();
    m_reflectionProbe.setup(SHADOW_CUBE_SIZE);

    const bool useMultithreading = m_options.getOption<bool>("h_multiThreading");
    _material.initialize(*this, useMultithreading);

    m_cubeBuffer.initialize();
    m_colorCubeBuffer.initialize();
    m_sphereBuffer.initialize();
    m_fireBallBuffer.initialize();
    //m_debugCube.initialize();

    Console::AddVar((int&)renderMode, "renderMode");
}

void RendererGLProg::Terminate()
{
	for (GLuint vao : _vertArrays)
	{
		glDeleteVertexArrays(1, &vao);
	}
    
    m_cubeBuffer.terminate();
    m_colorCubeBuffer.terminate();
    m_sphereBuffer.terminate();
    m_fireBallBuffer.terminate();
    //m_debugCube.terminate();
    m_lightSystem3D.terminate();
	m_gBuffer.Terminate();

	CleanupRenderBuffers();
    // Clean up geometry buffers
    CleanupGeometry();
    
    Log::Debug("[RendererGLProg] Clean shutdown complete");
}

TextureID RendererGLProg::getTextureID(const std::string& textureName, bool load)
{
    TextureID cachedID = m_textureCache.getTextureID(textureName);
    if (cachedID == TextureCache::NO_TEXTURE_ID && load)
    {
        Texture2D* texture = m_textureLoader.loadFromFile(textureName);
        if (texture)
        {
            cachedID = m_textureCache.addTexture(texture, textureName);
        }
    }
    return cachedID;
}

void RendererGLProg::getTextureIDAsync(const std::string& textureName, const std::function<void(TextureID)>& callback)
{
    TextureID cachedID = m_textureCache.getTextureID(textureName);
    if (cachedID != TextureCache::NO_TEXTURE_ID)
    {
        callback(cachedID);
        return;
    }

    m_textureLoader.asyncLoadFromFile(textureName, [this, textureName, callback](Texture2D* texture) {
        if (texture)
        {
            TextureID cachedID = m_textureCache.addTexture(texture, textureName);
            callback(cachedID);
            Log::Debug("RendererGLProg::getTextureIDAsync loaded %s", textureName.c_str());
        }
        else
        {
            Log::Error("RendererGLProg::getTextureIDAsync failed to load %s", textureName.c_str());
        }
    });
}

const Texture2D* RendererGLProg::getTextureByID(const TextureID textureID)
{
    return m_textureCache.getTextureByID(textureID);
}

TextureAtlasID RendererGLProg::getTextureAtlasID(const std::string& textureAtlasName)
{
    TextureAtlasID cachedID = m_atlasCache.getTextureAtlasID(textureAtlasName);
    if (cachedID == TextureAtlasCache::NO_ATLAS_ID)
    {
        TextureAtlas* textureAtlas = AtlasLoader::load(textureAtlasName, m_allocator, *this);
        if (textureAtlas)
        {
            cachedID = m_atlasCache.addTextureAtlas(textureAtlas, textureAtlasName);
            m_frameCache.addAtlas(textureAtlas, cachedID);
        }
    }
    return cachedID;
}

const TextureAtlas* RendererGLProg::getTextureAtlasByID(const TextureAtlasID textureAtlasID)
{
    return m_atlasCache.getTextureAtlasByID(textureAtlasID);
}

TextureAtlasID RendererGLProg::getTextureAtlasIDForFrame(const std::string& frameName)
{
    return m_frameCache.getAtlasForFrame(frameName);
}

ShaderID RendererGLProg::getShaderID(const std::string& shaderVertexName, const std::string& shaderFragName)
{
    ShaderID cachedID = m_shaderCache.getShaderID(shaderVertexName, shaderFragName);
    if (cachedID == ShaderCache::NO_SHADER_ID)
    {
        Shader* shader = ShaderLoader::load(shaderVertexName, shaderFragName, m_allocator);
        if (shader)
        {
            cachedID = m_shaderCache.addShader(shader, shaderVertexName, shaderFragName);
        }
    }
    return cachedID;
}

ShaderID RendererGLProg::getShaderID(const std::string& shaderGeometryName, const std::string& shaderVertexName, const std::string& shaderFragName)
{
    ShaderID cachedID = m_shaderCache.getShaderID(shaderGeometryName, shaderVertexName, shaderFragName);
    if (cachedID == ShaderCache::NO_SHADER_ID)
    {
        Shader* shader = ShaderLoader::load(shaderGeometryName, shaderVertexName, shaderFragName, m_allocator);
        if (shader)
        {
            cachedID = m_shaderCache.addShader(shader, shaderGeometryName, shaderVertexName, shaderFragName);
        }
    }
    return cachedID;
}

const Shader* RendererGLProg::getShaderByID(const ShaderID shaderID)
{
    return m_shaderCache.getShaderByID(shaderID);
}

TextAtlasID RendererGLProg::getTextAtlasID(const std::string& textAtlasName, const uint8_t fontHeight)
{
    char buf[256];
#ifdef _WIN32   /* Windows... ಠ_ಠ I don't even */
    sprintf_s(buf, "%s%i", textAtlasName.c_str(), fontHeight);
#else
    sprintf(buf, "%s%i", textAtlasName.c_str(), fontHeight);
#endif
    // Check if we already have an atlas for that font at that size
    const std::string fontNameAndSize = std::string(buf);

    TextAtlasID cachedID = m_textAtlasCache.getTextAtlasID(fontNameAndSize);
    if (cachedID == TextAtlasCache::NO_ATLAS_ID)
    {
        TextAtlas* atlas = TextAtlasLoader::load(textAtlasName, fontHeight, m_allocator, m_textureCache);
        if (atlas)
        {
            cachedID = m_textAtlasCache.addTextAtlas(atlas, fontNameAndSize);
        }
    }
    return cachedID;
}

TextAtlas* RendererGLProg::getTextAtlasByID(const TextAtlasID textAtlasID)
{
    return m_textAtlasCache.getTextAtlasByID(textAtlasID);
}

TexturedVertexData* RendererGLProg::queueTexturedVerts(const uint32_t numVerts, const TextureID textureID)
{
    auto it = m_texturedVertexPackages.find(textureID);
    if (it != m_texturedVertexPackages.end())
    {
        TexturedVertsDataPackage& package = it->second;
        if (package.count + numVerts >= package.capacity)
        {
            Log::Error("[RendererGLProg::queueTexturedVerts] out of capacity for textured verts");
            assert(false);
            return nullptr;
        }
        TexturedVertexData* dataPtr = &package.data[package.count];
        package.count += numVerts;
        return dataPtr;
    }

    TexturedVertexData* dataPtr = (TexturedVertexData*)m_frameAllocator.allocate(sizeof(TexturedVertexData) * MAX_UI_VERTS_PER_TEXTURE);
    if (!dataPtr)
    {
        Log::Error("[RendererGLProg::queueTexturedVerts] out of frame memory for textured verts");
        assert(false);
        return nullptr;
    }
    m_texturedVertexPackages[textureID] = { dataPtr, numVerts, MAX_UI_VERTS_PER_TEXTURE };
    return dataPtr;
}

TexturedVertexData* RendererGLProg::queueTextVerts(const uint32_t numVerts, const TextureID textureID)
{
    auto it = m_textVertexPackages.find(textureID);
    if (it != m_textVertexPackages.end())
    {
        TexturedVertsDataPackage& package = it->second;
        if (package.count + numVerts >= package.capacity)
        {
            Log::Error("[RendererGLProg::queueTexturedVerts] out of capacity for textured verts");
            assert(false);
            return nullptr;
        }
        TexturedVertexData* dataPtr = &package.data[package.count];
        package.count += numVerts;
        return dataPtr;
    }

    TexturedVertexData* dataPtr = (TexturedVertexData*)m_frameAllocator.allocate(sizeof(TexturedVertexData) * MAX_TEXT_VERTS_PER_TEXTURE);
    if (!dataPtr)
    {
        Log::Error("[RendererGLProg::queueTexturedVerts] out of frame memory for textured verts");
        assert(false);
        return nullptr;
    }
    m_textVertexPackages[textureID] = { dataPtr, numVerts, MAX_TEXT_VERTS_PER_TEXTURE };
    return dataPtr;
}

glm::ivec2 RendererGLProg::getWindowSize() const
{
    return glm::ivec2(windowWidth, windowHeight);
}

void RendererGLProg::SetDefaults()
{
    windowWidth = m_options.getOption<int>("r_resolutionX");
    windowHeight = m_options.getOption<int>("r_resolutionY");
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
}

void RendererGLProg::SetScreenMode()
{
    const GLubyte* version = glGetString(GL_VERSION);
    Log::Info("[RendererGLProg] OpenGL Version:%s", version);
    
    if(!GLEW_EXT_geometry_shader4) { Log::Warn("[RendererGLProg] No support for geometry shaders found"); }
    else { Log::Info("[RendererGLProg] Geometry shaders are supported"); }

    //Use Vsync
    if(SDL_GL_SetSwapInterval(1) < 0)
    {
        Log::Warn("[RendererGLProg] Unable to set VSync! SDL Error: %s", SDL_GetError());
    }
    
    Log::Info("[RendererGLProg] started with programmable pipeline");
    
    GLint dims[2];
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, &dims[0]);
    Log::Info("[RendererGLProg] Maximum render dimensions: %i, %i", dims[0],dims[1]);
}

//========================================================================
// ResizeWindow() - GLFW window resize callback function
//========================================================================

void RendererGLProg::Resize(const int width, const int height)
{
    int newY = width > 0 ? height : 1;   // Prevent division by zero in aspect calc.
    windowWidth = width;
    windowHeight = newY;
    
	m_options.getOption<int>("r_resolutionX") = windowWidth;
	m_options.getOption<int>("r_resolutionY") = windowHeight;

    // Refresh frambefuffers to new size
	m_gBuffer.Resize(windowWidth, windowHeight);

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
	//GLfloat hw = (windowWidth / 2);
	//GLfloat hh = (windowHeight / 2);
 //   target = glm::translate(target, glm::vec3(hw, hh, 0));
}

void RendererGLProg::GetGameMatrix( glm::mat4 &target )
{
    GLfloat aspectRatio = (windowWidth > windowHeight) ?
    GLfloat(windowWidth)/GLfloat(windowHeight) : GLfloat(windowHeight)/GLfloat(windowWidth);
    target = glm::perspective(m_camera.fieldOfView, aspectRatio, m_camera.nearDepth, m_camera.farDepth);
    target = glm::rotate(target, -m_camera.rotation.x, glm::vec3(1.0, 0.0, 0.0));
    target = glm::rotate(target, -m_camera.rotation.y, glm::vec3(0.0, 1.0, 0.0));
    target = glm::rotate(target, -m_camera.rotation.z, glm::vec3(0.0, 0.0, 1.0));
    target = glm::translate(target, glm::vec3(-m_camera.position.x, -m_camera.position.y, -m_camera.position.z));
}

/*  --------------------    *
 *      SHADER STUFF        *
 *  --------------------    */
void RendererGLProg::SetupShaders()
{
    // Used to draw lines and debugging
    f_shaderDefault_vColor = ShaderLoader::load("d_default_vColor.vsh", "d_default.fsh", m_allocator);
    d_shaderDefault_uColor = ShaderLoader::load("d_default_uColor.vsh", "d_default.fsh", m_allocator);
    // 2D rendering shaders
    f_shaderUI_tex = ShaderLoader::load("f_ui_tex.vsh", "f_ui_tex.fsh", m_allocator);
    f_shaderUI_text = ShaderLoader::load("f_ui_tex.vsh", "f_ui_text.fsh", m_allocator);
    f_shaderUI_color = ShaderLoader::load("f_ui_color.vsh", "f_ui_color.fsh", m_allocator);
    f_shaderUI_vColor = ShaderLoader::load("f_ui_vcolor.vsh", "f_ui_vcolor.fsh", m_allocator);
    f_shaderUI_cubeMap = ShaderLoader::load("f_ui_cubeMap.vsh", "f_ui_cubeMap.fsh", m_allocator);
    // Sunshine lens flares and halo
    f_shaderLensFlare = ShaderLoader::load("d_sunPostProcess.vsh", "d_sunPostProcess.fsh", m_allocator);
    f_shaderLensFlare->begin();
    f_shaderLensFlare->setUniform1iv("LowBlurredSunTexture", 0);
    f_shaderLensFlare->setUniform1iv("HighBlurredSunTexture", 1);
    f_shaderLensFlare->setUniform1iv("DirtTexture", 2);
    f_shaderLensFlare->setUniform1fv("Dispersal", 0.1875f);
    f_shaderLensFlare->setUniform1fv("HaloWidth", 0.45f);
    f_shaderLensFlare->setUniform1fv("Intensity", 2.25f);
    f_shaderLensFlare->setUniform3fv("Distortion", 0.94f, 0.97f, 1.00f);
    f_shaderLensFlare->end();
    // Vertical and horizontal blurs
	d_shaderBlurH = ShaderLoader::load("d_blur.vsh", "d_blur_horizontal.fsh", m_allocator);
	d_shaderBlurV = ShaderLoader::load("d_blur.vsh", "d_blur_vertical.fsh", m_allocator);
    // Deferred light pass, includes fog and noise
    d_shaderLightShadow = ShaderLoader::load("d_light_pass_shadow.vsh", "d_light_pass_shadow.fsh", m_allocator);
    // Renders the cubemesh
    d_shaderMesh = ShaderLoader::load("d_mesh_color.vsh", "d_mesh_color.fsh", m_allocator);
    // Renders dynamic objects in batched instances
    d_shaderInstance = ShaderLoader::load("d_object_instance.vsh", "d_object_instance.fsh", m_allocator);
    // Renders single colored cubes in batched instances
    d_shaderCubeSimple = ShaderLoader::load("d_cube_instance.vsh", "d_cube_instance.fsh", m_allocator);
    d_shaderCubeFancy = ShaderLoader::load("d_cube_instance_fancy.vsh", "d_cube_instance_fancy.fsh", m_allocator);
    d_shaderCubeColor = ShaderLoader::load("d_cube_instance_color.vsh", "d_cube_instance_color.fsh", m_allocator);;

	// Renders triangle meshes
	_shaderDeferredMesh = ShaderLoader::load("d_mesh.vsh", "d_mesh.fsh", m_allocator);

    // Renders instanced spheres
    _shaderDeferredSphere = ShaderLoader::load( "d_impostor_sphere.gsh", "d_impostor_sphere.vsh", "d_impostor_sphere.fsh", m_allocator);
    _shaderForwardFireball = ShaderLoader::load( "f_impostor_fireball.gsh", "f_impostor_fireball.vsh", "f_impostor_fireball.fsh", m_allocator);
	// Renders instanced sprites
    _shaderDeferredSprite = ShaderLoader::load( "d_impostor_billboard.gsh", "d_impostor_billboard.vsh", "d_impostor_billboard.fsh", m_allocator);
    d_shaderCloud = ShaderLoader::load( "d_impostor_cloud.gsh", "d_impostor_cloud.vsh", "d_impostor_cloud.fsh", m_allocator);
    f_shaderSprite = ShaderLoader::load( "f_impostor_billboard.gsh", "f_impostor_billboard.vsh", "f_impostor_billboard.fsh", m_allocator);
    m_lightShaderEmissive = ShaderLoader::load("d_light_pass_emissive.vsh", "d_light_pass_emissive.fsh", m_allocator);
    m_lightShaderBasic = ShaderLoader::load("d_light_pass_basic.vsh", "d_light_pass_basic.fsh", m_allocator);
    m_lightShaderBadass = ShaderLoader::load("d_light_pass_badass.vsh", "d_light_pass_badass.fsh", m_allocator);
    m_lightShaderDisney = ShaderLoader::load("d_light_pass_disney.vsh", "d_light_pass_disney.fsh", m_allocator);
    f_shaderBloomBrightPass = ShaderLoader::load("f_light_pass_bright.vsh", "f_light_pass_bright.fsh", m_allocator);
    f_shaderBlurGaussian = ShaderLoader::load("f_blur.vsh", "f_blur.fsh", m_allocator);

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
    d_shaderSky = ShaderLoader::load("d_sky_dome.vsh", "d_sky_dome.fsh", m_allocator);
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
    d_shaderPost = ShaderLoader::load("d_post_process.vsh", "d_post_process.fsh", m_allocator);
    d_shaderDepth = ShaderLoader::load("d_depth.vsh", "d_depth.fsh", m_allocator);
    d_shaderLensFlare = ShaderLoader::load("d_lensFlare.vsh", "d_lensFlare.fsh", m_allocator);
    d_shaderLightRays = ShaderLoader::load("d_light_rays.vsh", "d_light_rays.fsh", m_allocator);
    d_shaderEdgeSobel = ShaderLoader::load("d_edge_sobel.vsh", "d_edge_sobel.fsh", m_allocator);
    d_shaderEdgeFreiChen = ShaderLoader::load("d_edge_frei_chen.vsh", "d_edge_frei_chen.fsh", m_allocator);
    
    d_shaderShadowMapMesh = ShaderLoader::load("d_shadowMap_mesh.vsh", "d_shadowMap.fsh", m_allocator);
    d_shaderShadowMapObject = ShaderLoader::load("d_shadowMap_object.vsh", "d_shadowMap.fsh", m_allocator);
    d_shaderShadowMapSphere = ShaderLoader::load("d_shadowMap_sphere.gsh", "d_shadowMap_sphere.vsh", "d_shadowMap_sphere.fsh", m_allocator);
    d_shaderShadowMapCube = ShaderLoader::load("d_shadowMap_cube.vsh", "d_shadowMap.fsh", m_allocator);
    
    // Load lens dirt texture
    const std::string lensDirtFile = FileUtil::GetPath().append("DATA/GFX/lens_dirt.png");
    Texture2D* lens_dirt = m_textureLoader.loadFromFile(lensDirtFile, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
    tex_lens_dirt = m_textureCache.addTexture(lens_dirt, lensDirtFile);
    // noise texture
    const std::string noiseFile = FileUtil::GetPath().append("DATA/GFX/rgba-noise-medium.png");
    Texture2D* noise = m_textureLoader.loadFromFile(noiseFile, GL_REPEAT, GL_LINEAR, GL_LINEAR);
    tex_noise = m_textureCache.addTexture(noise, noiseFile);

    f_shaderDebugCube = ShaderLoader::load("f_debug_cube.vsh", "f_debug_cube.fsh", m_allocator);

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
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, m_gBuffer.GetDepth(), 0);  // Attach previous depth/stencil to it
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

	CHECK_GL_ERROR();
}

void RendererGLProg::CleanupRenderBuffers()
{
    glDeleteTextures( 1, &final_texture );
    glDeleteFramebuffers(1, &final_fbo);
    glDeleteTextures( 5, light_textures );
    glDeleteFramebuffers(1, &light_fbo);
}
/*  --------------------    *
 *      GEOMETRY STUFF      *
 *  --------------------    */
void RendererGLProg::SetupGeometry()
{
    CHECK_GL_ERROR();

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
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*3, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*2, (GLvoid*)(sizeof(GLfloat)*3));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
	// ----------   SPRITE BUFFER   --------- //
	m_sprite_vao = addVertexArray();
    glGenBuffers(1, &m_sprite_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_sprite_vbo);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(ColorVertexData), 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ColorVertexData), (GLvoid*)(4 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
  	CHECK_GL_ERROR();

	// ----------   SPHERE BUFFER   --------- //
	_mesh_vao = addVertexArray();
	_meshVB = addVertBuffer(VertexDataType::MeshVerts);
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
    glDeleteBuffers(1, &vertex_vbo);
    glDeleteBuffers(1, &vertex_ibo);
    glDeleteVertexArrays(1, &vertex_vao);
	glDeleteBuffers(1, &ui_vbo);
	glDeleteVertexArrays(1, &ui_vao);
    glDeleteBuffers(1, &square2D_vbo);
    glDeleteVertexArrays(1, &square2D_vao);
    glDeleteBuffers(1, &lines_vbo);
    glDeleteVertexArrays(1, &lines_vao);
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
	_deferredQueue.clear();
	_deferredInstanceQueue.clear();
	_forwardQueue.clear();
    m_cubeBuffer.clear();
    m_colorCubeBuffer.clear();
    m_sphereBuffer.clear();
    m_fireBallBuffer.clear();

    // Reset stats
    renderedSegs = 0;
    renderedTris = 0;
    renderedSprites = 0;

    m_textureLoader.processQueue();
}

void RendererGLProg::EndDraw()
{
}

void RendererGLProg::flush()
{
	CHECK_GL_ERROR();

    // Build matrices for billboards/impostors
    glm::mat4 rotMatrix = glm::mat4();
    rotMatrix = glm::rotate(rotMatrix, -m_camera.rotation.x, glm::vec3(1.0, 0.0, 0.0));
    rotMatrix = glm::rotate(rotMatrix, -m_camera.rotation.y, glm::vec3(0.0, 1.0, 0.0));
    rotMatrix = glm::rotate(rotMatrix, -m_camera.rotation.z, glm::vec3(0.0, 0.0, 1.0));

    glm::mat4 view = glm::mat4();
    view = glm::translate(rotMatrix, glm::vec3(-m_camera.position.x, -m_camera.position.y, -m_camera.position.z));

    glm::mat4 projection;
    GLfloat aspectRatio = (windowWidth > windowHeight) ?
        GLfloat(windowWidth) / GLfloat(windowHeight) : GLfloat(windowHeight) / GLfloat(windowWidth);
    projection = glm::perspective(m_camera.fieldOfView, aspectRatio, m_camera.nearDepth, m_camera.farDepth);
    glm::mat3 normalMatrix = glm::inverse(glm::mat3(rotMatrix));

    if (m_cubeBuffer.getCount())
    {
        m_cubeBuffer.upload();
    }
    if (m_colorCubeBuffer.getCount())
    {
        m_colorCubeBuffer.upload();
    }
    if (m_sphereBuffer.getCount())
    {
        m_sphereBuffer.upload();
    }
    if (m_fireBallBuffer.getCount())
    {
        m_fireBallBuffer.upload();
    }
    glm::vec4 viewPort = glm::vec4(0, 0, windowWidth, windowHeight);
    updateReflections();

    CHECK_GL_ERROR();

    // Clear color and Z-buffer
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glClearStencil(Stencil_None);

    m_gBuffer.Bind();
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_gBuffer.Clear();
    glViewport(0, 0, windowWidth, windowHeight);

    // Render 3D everything
    if (m_options.getOption<bool>("r_wireFrame"))
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
    if (_material.allLoaded())
    {
        renderCubes(view, projection, *d_shaderCubeFancy);
        renderSpheres(view, projection, normalMatrix, m_camera.position);
    }
    renderColorCubes(view, projection, *d_shaderCubeColor);
    RenderVerts();
    flushDeferredQueue(view, projection, m_camera.position);
    glDisable(GL_STENCIL_TEST);
    CHECK_GL_ERROR();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glm::vec2 screenRatio = glm::vec2(1.0f, 1.0f);

    prepareFinalFBO(windowWidth, windowHeight);

    m_lightSystem3D.RenderLighting(
        m_lightShaderDisney,
        m_lightShaderEmissive,
        _lightsQueue,
        rotMatrix,
        projection,
        viewPort,
        m_camera.position,
        screenRatio,
        m_camera.nearDepth,
        m_camera.farDepth,
        final_fbo,
        m_gBuffer,
        m_reflectionProbe.getPosition(),
        m_reflectionProbe.getSize(),
        m_reflectionProbe.getCubeMap());

    RenderLightingFX(
        rotMatrix,
        projection,
        viewPort,
        m_camera.position,
        screenRatio,
        m_camera.nearDepth,
        m_camera.farDepth);


    PostProcess();

    // Copy depth data from gbuffer to screen buffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_gBuffer.GetFBO());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, windowWidth, windowHeight, 0, 0, windowWidth, windowHeight,
        GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    CHECK_GL_ERROR();

	flushForwardQueue(view, projection, m_camera.position);
    CHECK_GL_ERROR();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    const Texture2D* noiseTex = m_textureCache.getTextureByID(tex_noise);
    m_fireBallBuffer.render(view, projection, normalMatrix, m_camera.position, noiseTex->getGLTextureID(), 0, 0, 0, 0, 0, *_shaderForwardFireball);
	render3DLines(projection*view);

    // Prepare for 2D rendering of GUI
    glDepthMask(GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClear(GL_DEPTH_BUFFER_BIT);

	GetUIMatrix(mvp2D);

    flushTexturedVertsQueue();
    flushTextVertsQueue();

	if (m_options.getOption<bool>("r_debug"))
    {
        renderDebug(); // Render buffers to screen for debugging
	}
	CHECK_GL_ERROR();

    m_texturedVertexPackages.clear();
    m_textVertexPackages.clear();
    m_frameAllocator.clear();
}

void RendererGLProg::updateReflections()
{
	glm::vec4 viewPort = glm::vec4(0, 0, windowWidth, windowHeight);

	const int drawSize = 256;
	const int cubeSize = m_reflectionProbe.getTextureSize();

	for (int side = 0; side < 6; side++)
	{
		CHECK_GL_ERROR();

		CubeMapSide cubeSide = (CubeMapSide)side;
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_TRUE);
		m_gBuffer.Bind();
		m_gBuffer.Clear();
		glm::mat4 reflectionView = m_reflectionProbe.getView(cubeSide);
		glm::mat4 reflectionProjection = m_reflectionProbe.getProjection(cubeSide);
		glm::mat3 reflectionNormalMatrix = glm::inverse(glm::mat3(reflectionView));

		glViewport(0, 0, cubeSize, cubeSize);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilFunc(GL_ALWAYS, Stencil_Solid, 0xFF);
        if (_material.allLoaded())
        {
            renderCubes(m_reflectionProbe.getView(cubeSide), m_reflectionProbe.getProjection(cubeSide), *d_shaderCubeSimple);
            renderSpheres(
                reflectionView,
                reflectionProjection,
                reflectionNormalMatrix,
                m_reflectionProbe.getPosition());
        }
		flushDeferredQueue(
			reflectionView,
			reflectionProjection,
			m_reflectionProbe.getPosition());
		glDisable(GL_STENCIL_TEST);
		CHECK_GL_ERROR();

		float ratioX = (float)cubeSize / windowWidth;
		float ratioY = (float)cubeSize / windowHeight;
		prepareFinalFBO(windowWidth, windowHeight);

		m_lightSystem3D.RenderLighting(
            m_lightShaderDisney,
            nullptr,
			_lightsQueue,
			reflectionView,
			reflectionProjection,
			viewPort,
			m_reflectionProbe.getPosition(),
			glm::vec2(ratioX, ratioY),
			1.0f,
			cubeSize+1.0f,
			final_fbo,
			m_gBuffer,
			m_reflectionProbe.getPosition(),
			m_reflectionProbe.getSize(),
			m_reflectionProbe.getCubeMap());

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

		m_reflectionProbe.bind(cubeSide);
		//glClear(GL_COLOR_BUFFER_BIT);

		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glDepthMask(GL_FALSE);

		Rect2D tRect = Rect2D(0.0f, 0.0f, ratioX, ratioY);

		glm::mat4 mvp = glm::ortho<GLfloat>(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
		// copy lit image from final_fbo to cubemap side
		DrawTexture(Rect2D(0, 0, 1.0f, 1.0f), tRect, final_texture, mvp);
	}

    // Generate mipmaps for rough surfaces
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_reflectionProbe.getCubeMap());
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);  
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void RendererGLProg::prepareFinalFBO(const int width, const int height)
{
	// Output to final image FBO
	glBindFramebuffer(GL_FRAMEBUFFER, final_fbo);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_BLEND);

	// Draw sky layer without lighting and opaque layer in black
	glEnable(GL_STENCIL_TEST);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilFunc(GL_EQUAL, Stencil_Sky, 0xFF);             // Only draw sky layer
	RenderFromTexture(m_gBuffer.GetAlbedo());
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
	bool dof = m_options.getOption<bool>("r_renderDOF");
	bool flare = m_options.getOption<bool>("r_renderFlare");
	bool vignette = m_options.getOption<bool>("r_renderVignette");
	bool correctGamma = m_options.getOption<bool>("r_renderCorrectGamma");
	bool toneMap = m_options.getOption<bool>("r_renderToneMap");

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
		d_shaderPost->setUniform1fv("focalDepth", m_camera.focalDepth);
		d_shaderPost->setUniform1fv("focalLength", m_camera.focalLength);
		d_shaderPost->setUniform1fv("fstop", m_camera.fStop);
		d_shaderPost->setUniform1fv("exposure", m_camera.exposure);
		d_shaderPost->setUniform1iv("showFocus", m_camera.debugLens);
		d_shaderPost->setUniform1iv("autofocus", m_camera.autoFocus);
		d_shaderPost->setUniform1iv("renderDOF", dof);
		d_shaderPost->setUniform1iv("renderVignette", vignette);
		d_shaderPost->setUniform1iv("correctGamma", correctGamma);
		d_shaderPost->setUniform1iv("toneMap", toneMap);
		d_shaderPost->setUniform1fv("znear", m_camera.nearDepth);
		d_shaderPost->setUniform1fv("zfar", m_camera.farDepth);

		glDepthMask(GL_FALSE);

		glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, m_textureCache.getTextureByID(tex_lens_dirt)->getGLTextureID());
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_gBuffer.GetDepth());
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
			RenderFromTexture(m_gBuffer.GetAlbedo());
		}
		else if (renderMode == RM_Specular) {
			// Nothing fancy here, just render to screen
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			RenderFromTexture(m_gBuffer.GetMaterial());
		}
		else if (renderMode == RM_Normal) {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			RenderFromTexture(m_gBuffer.GetNormal());
		}
		else if (renderMode == RM_Depth) {
			d_shaderDepth->begin();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_gBuffer.GetDepth());
			d_shaderDepth->setUniformM4fv("MVP", mvp2d);
			d_shaderDepth->setUniform4fv("u_color", COLOR_WHITE);
			d_shaderDepth->setUniform1fv("nearDepth", m_camera.nearDepth);
			d_shaderDepth->setUniform1fv("farDepth", m_camera.farDepth);
			glBindVertexArray(square2D_vao);
			glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			d_shaderDepth->end();
		}
		else if (renderMode == RM_Shadowmap) {
			d_shaderDepth->begin();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, shadow_texture);
			d_shaderDepth->setUniformM4fv("MVP", mvp2d);
			d_shaderDepth->setUniform4fv("u_color", COLOR_WHITE);
			d_shaderDepth->setUniform1fv("nearDepth", m_camera.nearDepth);
			d_shaderDepth->setUniform1fv("farDepth", m_camera.farDepth);
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

void RendererGLProg::PassStencil( const GLuint fbo )
{
    if ( fbo != m_gBuffer.GetFBO() )
	{
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_gBuffer.GetFBO());
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

glm::mat4 RendererGLProg::GetLightMVP(LightInstance& light)
{
    if (light.type == Light_Type_Directional)
	{
        // Camera projection matrices
        glm::mat4 model = glm::mat4();
        glm::mat4 proj = glm::mat4();
        model = glm::rotate(model, -m_camera.rotation.x, glm::vec3(1.0, 0.0, 0.0));
        model = glm::rotate(model, -m_camera.rotation.y, glm::vec3(0.0, 1.0, 0.0));
        model = glm::rotate(model, -m_camera.rotation.z, glm::vec3(0.0, 0.0, 1.0));
        model = glm::translate(model, glm::vec3(-m_camera.position.x, -m_camera.position.y, -m_camera.position.z));
        glm::vec4 viewport = glm::vec4(0, 0, windowWidth, windowHeight);
        GLfloat aspectRatio = (windowWidth > windowHeight) ?
        GLfloat(windowWidth)/GLfloat(windowHeight) : GLfloat(windowHeight)/GLfloat(windowWidth);
        proj = glm::perspective(m_camera.fieldOfView, aspectRatio, m_camera.nearDepth, m_camera.farDepth);
        
        
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
        max += glm::vec3(m_camera.farDepth*0.2f);
        min -= glm::vec3(m_camera.farDepth*0.2f);
        
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
        model = glm::rotate(model, -m_camera.rotation.x, glm::vec3(1.0, 0.0, 0.0));
        model = glm::rotate(model, -m_camera.rotation.y, glm::vec3(0.0, 1.0, 0.0));
        model = glm::rotate(model, -m_camera.rotation.z, glm::vec3(0.0, 0.0, 1.0));
        model = glm::translate(model, glm::vec3(-m_camera.position.x, -m_camera.position.y, -m_camera.position.z));
        glm::vec4 viewport = glm::vec4(0, 0, windowWidth, windowHeight);
        GLfloat aspectRatio = (windowWidth > windowHeight) ?
        GLfloat(windowWidth)/GLfloat(windowHeight) : GLfloat(windowHeight)/GLfloat(windowWidth);
        proj = glm::perspective(m_camera.fieldOfView, aspectRatio, m_camera.nearDepth, m_camera.farDepth);
        
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
        max += glm::vec3(m_camera.farDepth*0.1f);
        min -= glm::vec3(m_camera.farDepth*0.1f);
        
        // Compute the MVP matrix from the light's point of view
        glm::mat4 lightProjectionMatrix = glm::ortho<float>(min.x,max.x,min.y,max.y,-max.z,-min.z);
        return lightProjectionMatrix;
    }
    //else if ( light.type == Light_Type_Spot )
	//{
        glm::mat4 lightProjectionMatrix = glm::perspective(light.spotCutoff*2.0f, 1.0f, 0.05f, light.position.w);
        return lightProjectionMatrix;
    //}
}

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
	bool renderLightDots = m_options.getOption<bool>("r_lightDots");
	bool renderLightBlur = m_options.getOption<bool>("r_lightBlur");
	bool renderLightRays = m_options.getOption<bool>("r_lightRays");
	bool renderLightFlare = m_options.getOption<bool>("r_lightFlare");

	// Copy depth+stencil buffer at lower res to light FBO
	glDepthMask(GL_TRUE);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, final_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, light_fbo);
	glBlitFramebuffer(0, 0, width, height,
		0, 0, width*LIGHT_SCALE, height*LIGHT_SCALE,
		GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
	glDepthMask(GL_FALSE);

	// ------------ LIGHTING STAGE 2 ------------------------
	// 2D Radial beam lights in 3D / add lens flare
	for (int i = 0; i < _lightsQueue.size(); i++)
	{
		LightInstance& light = _lightsQueue[i];
		if (!light.active || light.raySize <= 0.f) continue;
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
			const float lightDrawScale = 16.f;

			if (light.position.w == 0.0f)
            {   
				radius = 16.0f; // Light has no radius, we give it an arbitrary one
			}
			else 
            {
				float Z = depthParameter.y / (depthParameter.x - lightScrnPos.z);
				// Scale light radius by radius/1.0-depth to keep constant relative size (light.position.w for radius)
                //const float brightness = (light.color.r + light.color.g + light.color.b + light.color.a) * 0.25f * light.position.w;
				radius = light.raySize * lightDrawScale / (1.f - Z);
			}
			if ( radius < 4.0f ) continue;
			//radius = fmaxf(radius, 40.0f);
			//radius = fminf(radius, 1024.0f);
            float lightZ = ORTHO_FARDEPTH + (ORTHO_NEARDEPTH - ORTHO_FARDEPTH) * fminf(lightScrnPos.z, 1.0f);
            Color lightDotColor = RGBAColor(light.color.r, light.color.g, light.color.b, 1.0);

            glViewport(0, 0, windowWidth * LIGHT_SCALE, windowHeight * LIGHT_SCALE);
            // Render simple sun circle into light buffer 0
            glBindFramebuffer(GL_FRAMEBUFFER, light_fbo);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, light_textures[0], 0);
            glClear(GL_COLOR_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			// Draw a simple 2D circle at light position
			DrawCircle(lightScrn2D, 0.0f, radius, COLOR_NONE, lightDotColor, lightZ, 1);
			glDisable(GL_DEPTH_TEST);

            GLuint currentLightBuffer = light_textures[0];

			if (light.raySize > 0.f)
			{
				// Radial light beams
				// Update sun into light buffer tex 2
				glBindFramebuffer(GL_FRAMEBUFFER, light_fbo);
				glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, light_textures[2], 0);
				//glViewport(0, 0, width*LIGHT_SCALE, height*LIGHT_SCALE);
				glClear(GL_COLOR_BUFFER_BIT);
				glm::mat4 mvp2d = glm::ortho<GLfloat>(-0.5, 0.5, -0.5, 0.5, -1.0, 1.0);
				glm::vec2 lightScrnUnit = glm::vec2(lightScrnPos.x, lightScrnPos.y) / glm::vec2(width, height);
				d_shaderLightRays->begin();
				d_shaderLightRays->setUniformM4fv("MVP", mvp2d);
				d_shaderLightRays->setUniform2fv("lightScrnPos", lightScrnUnit);
				d_shaderLightRays->setUniform1fv("exposure", lr_exposure);
				d_shaderLightRays->setUniform1fv("decay", lr_decay);
				d_shaderLightRays->setUniform1fv("density", lr_density);
				d_shaderLightRays->setUniform1fv("weight", lr_weight);
				glActiveTexture(GL_TEXTURE0);
 				glBindTexture(GL_TEXTURE_2D, currentLightBuffer);    // Read from light colors
				glBindVertexArray(square2D_vao);
				glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
				glDrawArrays(GL_TRIANGLE_FAN, 0, 4);    // Render light rays
				d_shaderLightRays->end();
				//glViewport(0, 0, width, height);    // reset viewport
                currentLightBuffer = light_textures[2];
			}
			if (renderLightFlare)
			{
                glViewport(0, 0, windowWidth, windowHeight);
				// calculate lens flare and halo and apply dirt texture (to tex 3)
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, light_textures[3], 0);
				glClear(GL_COLOR_BUFFER_BIT);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, light_textures[0]);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, currentLightBuffer);
				glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, m_textureCache.getTextureByID(tex_lens_dirt)->getGLTextureID());
				f_shaderLensFlare->begin();
				glm::vec2 lightScrnUnit = glm::vec2(lightScrnPos.x, lightScrnPos.y) / glm::vec2(width, height);
				f_shaderLensFlare->setUniform2fv("SunPosProj", lightScrnUnit);
				glBindVertexArray(square2D_vao);
				glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
				glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
				f_shaderLensFlare->end();
				glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, 0);
				glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);
				glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
                currentLightBuffer = light_textures[3];
			}
			// Render final light to FBO
			glBindFramebuffer(GL_FRAMEBUFFER, final_fbo);
            glViewport(0, 0, windowWidth, windowHeight);
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
			Rect2D tRect = Rect2D(0.0f, 0.0f, 1.0, 1.0);
			glm::mat4 mvp = glm::ortho<GLfloat>(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
			DrawTexture(Rect2D(0, 0, 1.0f, 1.0f), tRect, currentLightBuffer, mvp);
		}
	}	// for all lights
	glBindVertexArray(0);
    
    bool bloom = true;
    if (bloom)
    {
        renderBloom();
    }

    glViewport(0, 0, windowWidth, windowHeight);
}

GLuint RendererGLProg::addVertexArray()
{
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	_vertArrays.push_back(vao);

	return vao;
}

VertBuffer* RendererGLProg::addVertBuffer(const VertexDataType type)
{
	VertBuffer* buffer = CUSTOM_NEW(VertBuffer, m_allocator)(type);
	buffer->initialize();
	_vertBuffers[buffer->getVBO()] = buffer;
	return buffer;
}

void RendererGLProg::destroyVertBuffer(VertBuffer* buffer)
{
    auto it = _vertBuffers.find(buffer->getVBO());
    if (it != _vertBuffers.end())
    {
        _vertBuffers.erase(it);
    }
    buffer->destroy();
    CUSTOM_DELETE(buffer, m_allocator);
}

BaseVertexData* RendererGLProg::createVertexData(const size_t size, const VertexDataType type)
{
    if (type == VertexDataType::MeshVerts)
    {
        return allocVertexData<MeshVertexData>(size, type);
    }
    else if (type == VertexDataType::TexturedVerts)
    {
        return allocVertexData<TexturedVertexData>(size, type);
    }
    else if (type == VertexDataType::SphereVerts)
    {
        return allocVertexData<SphereVertexData>(size, type);
    }
    else if (
        type == VertexDataType::ColorVerts ||
        type == VertexDataType::SpriteVerts)
    {
        return allocVertexData<ColorVertexData>(size, type);
    }
    else if (type == VertexDataType::InstanceVerts)
    {
        return allocVertexData<InstanceTransformData>(size, type);
    }
    else if (type == VertexDataType::InstancedCubeVerts)
    {
        return allocVertexData<CubeInstance>(size, type);
    }
    Log::Error("RendererGLProg::createVertexData unsupported vertex data type %i", type);
    return nullptr;
}

void RendererGLProg::destroyVertexData(BaseVertexData* vertexData)
{
    if (vertexData->getType() == VertexDataType::MeshVerts)
    {
        VertexData<MeshVertexData>* data = dynamic_cast<VertexData<MeshVertexData>*>(vertexData);
        //data->destroy();
        CUSTOM_DELETE(data, m_allocator);
    }
    else if (vertexData->getType() == VertexDataType::TexturedVerts)
    {
        VertexData<TexturedVertexData>* data = dynamic_cast<VertexData<TexturedVertexData>*>(vertexData);
        CUSTOM_DELETE(data, m_allocator);
    }
    else if (vertexData->getType() == VertexDataType::SphereVerts)
    {
        VertexData<SphereVertexData>* data = dynamic_cast<VertexData<SphereVertexData>*>(vertexData);
        CUSTOM_DELETE(data, m_allocator);
    }
    else if (
        vertexData->getType() == VertexDataType::ColorVerts ||
        vertexData->getType() == VertexDataType::SpriteVerts)
    {
        VertexData<ColorVertexData>* data = dynamic_cast<VertexData<ColorVertexData>*>(vertexData);
        CUSTOM_DELETE(data, m_allocator);
    }
    else if (vertexData->getType() == VertexDataType::InstanceVerts)
    {
        VertexData<InstanceTransformData>* data = dynamic_cast<VertexData<InstanceTransformData>*>(vertexData);
        CUSTOM_DELETE(data, m_allocator);
    }
    else if (vertexData->getType() == VertexDataType::InstancedCubeVerts)
    {
        VertexData<CubeInstance>* data = dynamic_cast<VertexData<CubeInstance>*>(vertexData);
        CUSTOM_DELETE(data, m_allocator);
    }
    Log::Error("RendererGLProg::destroyVertexData unsupported vertex data type %i", vertexData->getType());
}

template <typename T> VertexData<T>* RendererGLProg::allocVertexData(const size_t size, const VertexDataType type)
{
    VertexData<T>* vertexData = CUSTOM_NEW(VertexData<T>, m_allocator)(size, type, m_allocator);

    return vertexData;
}

void RendererGLProg::queueDeferredBuffer(
    const VertBuffer* buffer,
    const void* data,
    const unsigned int rangeStart,
    const unsigned int rangeEnd,
    const TextureID texture,
    const BlendMode blendMode,
    const DepthMode depthMode)
{
    DrawPackage package = { buffer, data, rangeEnd, rangeStart, texture, blendMode, depthMode };
	_deferredQueue.push_back(package);
}

void RendererGLProg::queueForwardBuffer(
    const VertBuffer* buffer,
    const void* data,
    const unsigned int rangeStart,
    const unsigned int rangeEnd,
    const TextureID texture,
    const BlendMode blendMode,
    const DepthMode depthMode)
{
	DrawPackage package = { buffer, data, rangeEnd, rangeStart, texture, blendMode, depthMode };
	_forwardQueue.push_back(package);
}

void RendererGLProg::queueDeferredInstances(
	const GLuint instanceBuffer,
	const unsigned int instanceCount, 
	const VertexDataType type,
	const GLuint buffer,
	const unsigned int rangeStart,
	const unsigned int rangeEnd,
	const TextureID tex,
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
	VertBuffer* buffer,
	const unsigned int rangeEnd,
	const unsigned int rangeStart,
	const Texture2D* tex,
	const bool render3D)
{
	//if (buffer->getType() == MeshVerts)
	//{
 //       glActiveTexture(GL_TEXTURE4);
 //       glBindTexture(GL_TEXTURE_2D, _material.getDisplacement()->getGLTextureID());
 //       glActiveTexture(GL_TEXTURE3);
 //       glBindTexture(GL_TEXTURE_2D, _material.getRoughness()->getGLTextureID());
 //       glActiveTexture(GL_TEXTURE2);
 //       glBindTexture(GL_TEXTURE_2D, _material.getMetalness()->getGLTextureID());
 //       glActiveTexture(GL_TEXTURE1);
 //       glBindTexture(GL_TEXTURE_2D, _material.getNormal()->getGLTextureID());
 //       glActiveTexture(GL_TEXTURE0);
 //       glBindTexture(GL_TEXTURE_2D, _material.getAlbedo()->getGLTextureID());

	//	glBindVertexArray(_mesh_vao);
	//	buffer->bind();
 //       glEnableVertexAttribArray(0);
 //       glEnableVertexAttribArray(1);
 //       glEnableVertexAttribArray(2);
 //       glEnableVertexAttribArray(3);
 //       glEnableVertexAttribArray(4);
 //       glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), 0);
 //       glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (GLvoid*)(4 * sizeof(GLfloat)));
 //       glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (GLvoid*)(7 * sizeof(GLfloat)));
 //       glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (GLvoid*)(10 * sizeof(GLfloat)));
 //       glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (GLvoid*)(12 * sizeof(GLfloat)));

	//	_shaderDeferredMesh->begin();
	//	_shaderDeferredMesh->setUniformM4fv("MVP", mvp3D);
 //       _shaderDeferredMesh->setUniform1iv("albedoTexture", 0);
 //       _shaderDeferredMesh->setUniform1iv("normalTexture", 1);
 //       _shaderDeferredMesh->setUniform1iv("metalnessTexture", 2);
 //       _shaderDeferredMesh->setUniform1iv("roughnessTexture", 3);
 //       _shaderDeferredMesh->setUniform1iv("displacementTexture", 4);
 //       _shaderDeferredMesh->setUniform3fv("camPos", m_camera.position);
	//	glDrawArrays(GL_TRIANGLES, rangeStart, rangeEnd);
	//	_shaderDeferredMesh->end();
	//	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//	renderedSprites = rangeEnd;
	//}
	//else if (buffer->getType() == SpriteVerts)
	//{
	//	if (tex != NULL)
	//	{
	//		glActiveTexture(GL_TEXTURE0);
	//		tex->Bind();
	//	}

	//	glBindVertexArray(_sprite_vao);
	//	buffer->bind();
	//	glEnableVertexAttribArray(0);
	//	glEnableVertexAttribArray(1);
	//	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
	//	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(4 * sizeof(GLfloat)));
	//	if (render3D)
	//	{
	//		_shaderDeferredSprite->begin();
	//	}
	//	else
	//	{
	//		f_shaderSprite->begin();
	//		f_shaderSprite->setUniformM4fv("View", glm::mat4());
	//		f_shaderSprite->setUniformM4fv("Projection", mvp2D);
	//		f_shaderSprite->setUniform3fv("CameraPosition", glm::vec3());
	//	}
	//	glDrawArrays(GL_POINTS, rangeStart, rangeEnd);
	//	_shaderDeferredSprite->end();
	//	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//	renderedSprites = rangeEnd;
	//}

	//if (tex != NULL)
	//{
	//	glBindTexture(GL_TEXTURE_2D, 0);
	//}
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindVertexArray(0);
}

void RendererGLProg::BufferSpheres(const SphereVertexData* spheres, const int numSpheres)
{
	m_sphereBuffer.bufferData(spheres, numSpheres);
}
void RendererGLProg::BufferFireballs(const SphereVertexData* spheres, const int numFireballs)
{
    m_fireBallBuffer.bufferData(spheres, numFireballs);
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
    m_sphereBuffer.render(
        view,
        projection,
        normalMatrix,
        position,
        _material.getAlbedo()->getGLTextureID(),
        _material.getNormal()->getGLTextureID(),
        _material.getMetalness()->getGLTextureID(),
        _material.getRoughness()->getGLTextureID(),
        _material.getDisplacement()->getGLTextureID(),
        _material.getEmissive()->getGLTextureID(),
        *_shaderDeferredSphere);
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
        Log::Error("RendererGLProg::DrawPolygon out of memory");
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
    model = glm::rotate(model, -m_camera.rotation.x, glm::vec3(1.0, 0.0, 0.0));
    model = glm::rotate(model, -m_camera.rotation.y, glm::vec3(0.0, 1.0, 0.0));
    model = glm::rotate(model, -m_camera.rotation.z, glm::vec3(0.0, 0.0, 1.0));
    glm::mat4 proj = glm::mat4();
    GLfloat aspectRatio = (windowWidth > windowHeight) ?
    GLfloat(windowWidth)/GLfloat(windowHeight) : GLfloat(windowHeight)/GLfloat(windowWidth);
    proj = glm::perspective(m_camera.fieldOfView, aspectRatio, m_camera.nearDepth, m_camera.farDepth);
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
    model = glm::rotate(model, -m_camera.rotation.x, glm::vec3(1.0, 0.0, 0.0));
    model = glm::rotate(model, -m_camera.rotation.y, glm::vec3(0.0, 1.0, 0.0));
    model = glm::rotate(model, -m_camera.rotation.z, glm::vec3(0.0, 0.0, 1.0));
    model = glm::translate(model, glm::vec3(-m_camera.position.x, -m_camera.position.y, -m_camera.position.z));
    glm::mat4 proj = glm::mat4();
    GLfloat aspectRatio = (windowWidth > windowHeight) ?
    GLfloat(windowWidth)/GLfloat(windowHeight) : GLfloat(windowHeight)/GLfloat(windowWidth);
    proj = glm::perspective(m_camera.fieldOfView, aspectRatio, m_camera.nearDepth, m_camera.farDepth);
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

void RendererGLProg::DrawImage( const glm::vec2 center, const float width, const float height,
                                const std::string texName, const float z, const Color color )
{
    const TextureID texID = m_textureCache.getTextureID(texName);
    if (texID == TextureCache::NO_TEXTURE_ID)
    {
        return;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textureCache.getTextureByID(texID)->getGLTextureID());
    GLfloat hw = (GLfloat)(width * 0.5f);
    GLfloat hh = (GLfloat)(height * 0.5f);
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(texCoords), NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(texCoords), texCoords);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)sizeof(vertices));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    f_shaderUI_tex->end();
    glBindVertexArray(0);
    renderedTris += 2;
    glBindTexture(GL_TEXTURE_2D, 0);
}
void RendererGLProg::DrawTexture(
	const Rect2D rect,
	const Rect2D texRect,
	const GLuint tex,
	const glm::mat4& mvp,
	const float z,
	const Color color)
{
    GLfloat vertices[16] = {
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
    m_cubeBuffer.bufferData(cubes, count);
}

void RendererGLProg::renderCubes(const glm::mat4& view, const glm::mat4& projection, Shader& shader)
{
    m_cubeBuffer.render(view, projection, _material, shader, m_camera.position);
}

void RendererGLProg::bufferColorCubes(const CubeInstanceColor* cubes, const size_t count)
{
    m_colorCubeBuffer.bufferData(cubes, count);
}

void RendererGLProg::renderColorCubes(const glm::mat4& view, const glm::mat4& projection, Shader& shader)
{
    m_colorCubeBuffer.render(view, projection, _material, shader, m_camera.position);
}

const glm::vec3 RendererGLProg::GetCursor3DPos( glm::vec2 cursorPos ) const {
    const int hw = windowWidth/2;
    const int hh = windowHeight/2;

    GLfloat cursorDepth=0;
    // Obtain the Z position (not world coordinates but in range 0 ~ 1)
    glReadPixels(cursorPos.x, cursorPos.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &cursorDepth);
    // Grab pixel under cursor color
//    GLfloat col[4];
//    glReadPixels(cursorPos.x, cursorPos.y, 1, 1, GL_RGBA, GL_FLOAT, &col);
//    printf("cursor color:%.1f, %.1f %.1f\n", col[0],col[1],col[2]);
    
    // Prepare matrices to unproject cursor coordinates
    glm::mat4 model = glm::mat4();
    model = glm::rotate(model, -m_camera.rotation.x, glm::vec3(1.0, 0.0, 0.0));
    model = glm::rotate(model, -m_camera.rotation.y, glm::vec3(0.0, 1.0, 0.0));
    model = glm::rotate(model, -m_camera.rotation.z, glm::vec3(0.0, 0.0, 1.0));
    model = glm::translate(model, glm::vec3(-m_camera.position.x, -m_camera.position.y, -m_camera.position.z));
    
    glm::mat4 proj = glm::mat4();
    GLfloat aspectRatio = (windowWidth > windowHeight) ?
    GLfloat(windowWidth)/GLfloat(windowHeight) : GLfloat(windowHeight)/GLfloat(windowWidth);
    proj = glm::perspective(m_camera.fieldOfView, aspectRatio, m_camera.nearDepth, m_camera.farDepth);
    glm::vec4 viewport = glm::vec4(0, 0, windowWidth, windowHeight);
    glm::vec3 crosshairPos  = glm::unProject(glm::vec3(cursorPos.x, cursorPos.y, cursorDepth), model, proj, viewport);
    // Linearize depth
//    glm::vec2 depthParameter = glm::vec2( m_camera.farDepth / ( m_camera.farDepth - m_camera.nearDepth ),
//                                         m_camera.farDepth * m_camera.nearDepth / ( m_camera.nearDepth - m_camera.farDepth ) );
//    float Z = depthParameter.y/(depthParameter.x - cursorDepth);

//    printf("Cursor depth: %f, pos: %f, %f, %f\n", Z, crosshairPos.x, crosshairPos.y, crosshairPos.z);
    return crosshairPos;
}

const std::string RendererGLProg::GetInfo() const
{
    std::string info = "Renderer: OpenGL Programmable pipeline\n";
    if (m_options.getOption<bool>("r_fullScreen"))
    {
        info.append("Full screen, resolution: ");
    }
    else
    {
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
		GLUtils::setBlendMode(package.blendMode);
		GLUtils::setDepthMode(package.depthMode);

        const VertexDataType packageType = package.buffer->getType();
		if (packageType == VertexDataType::MeshVerts)
		{
            if (!_material.allLoaded())
            {
                continue;
            }
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, _material.getEmissive()->getGLTextureID());
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, _material.getDisplacement()->getGLTextureID());
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, _material.getRoughness()->getGLTextureID());
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, _material.getMetalness()->getGLTextureID());
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, _material.getNormal()->getGLTextureID());
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, _material.getAlbedo()->getGLTextureID());
            _shaderDeferredMesh->begin();
            _shaderDeferredMesh->setUniformM4fv("MVP", mvp3D);
            _shaderDeferredMesh->setUniform1iv("albedoTexture", 0);
            _shaderDeferredMesh->setUniform1iv("normalTexture", 1);
            _shaderDeferredMesh->setUniform1iv("metalnessTexture", 2);
            _shaderDeferredMesh->setUniform1iv("roughnessTexture", 3);
            _shaderDeferredMesh->setUniform1iv("displacementTexture", 4);
            _shaderDeferredMesh->setUniform1iv("emissiveTexture", 5);
            _shaderDeferredMesh->setUniform3fv("camPos", position);
			glBindVertexArray(_mesh_vao);
			glBindBuffer(GL_ARRAY_BUFFER, package.buffer->getVBO());
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glEnableVertexAttribArray(2);
            glEnableVertexAttribArray(3);
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), 0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (GLvoid*)(4 * sizeof(GLfloat)));
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (GLvoid*)(7 * sizeof(GLfloat)));
            glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (GLvoid*)(10 * sizeof(GLfloat)));
            glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (GLvoid*)(12 * sizeof(GLfloat)));
            if (package.data)
            {
                glBufferData(GL_ARRAY_BUFFER, package.rangeEnd * sizeof(MeshVertexData), package.data, GL_DYNAMIC_DRAW);
            }
            glDrawArrays(GL_TRIANGLES, package.rangeStart, package.rangeEnd);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			_shaderDeferredMesh->end();
			renderedTris = (package.rangeStart - package.rangeEnd) / 3;
		}
		else if (packageType == VertexDataType::SpriteVerts)
		{
			if (package.textureID != 0)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, package.textureID);
			}
			glBindVertexArray(m_sprite_vao);
			glBindBuffer(GL_ARRAY_BUFFER, package.buffer->getVBO());
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(4 * sizeof(GLfloat)));
            if (package.data)
            {
                glBufferData(GL_ARRAY_BUFFER, package.rangeEnd * sizeof(ColorVertexData), package.data, GL_DYNAMIC_DRAW);
            }
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
		if (package.type != VertexDataType::MeshVerts)
		{
			Log::Error("[RendererGLProg] unsupported instanced mesh vertex data type!");
			continue;
		}
        if (!_material.allLoaded())
        {
            continue;
        }
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, _material.getEmissive()->getGLTextureID());
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, _material.getDisplacement()->getGLTextureID());
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, _material.getRoughness()->getGLTextureID());
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, _material.getMetalness()->getGLTextureID());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, _material.getNormal()->getGLTextureID());
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _material.getAlbedo()->getGLTextureID());

		GLUtils::setBlendMode(package.blendMode);
		GLUtils::setDepthMode(package.depthMode);

		glBindVertexArray(instancing_vao);
		// Vertex data binding
		glBindBuffer(GL_ARRAY_BUFFER, package.bufferID);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), 0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (GLvoid*)(4 * sizeof(GLfloat)));
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (GLvoid*)(7 * sizeof(GLfloat)));
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (GLvoid*)(10 * sizeof(GLfloat)));
        glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertexData), (GLvoid*)(12 * sizeof(GLfloat)));
		// Instance data binding
		glBindBuffer(GL_ARRAY_BUFFER, package.instanceBufferID);
		glEnableVertexAttribArray(5);
		glEnableVertexAttribArray(6);
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceTransformData), 0);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceTransformData), (GLvoid*)(3 * sizeof(GLfloat)));
		glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceTransformData), (GLvoid*)(7 * sizeof(GLfloat)));
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
		glVertexAttribDivisor(7, 1);
		
		CHECK_GL_ERROR();

		d_shaderInstance->begin();
		d_shaderInstance->setUniformM4fv("MVP", projection*view);
        d_shaderInstance->setUniform1iv("albedoTexture", 0);
        d_shaderInstance->setUniform1iv("normalTexture", 1);
        d_shaderInstance->setUniform1iv("metalnessTexture", 2);
        d_shaderInstance->setUniform1iv("roughnessTexture", 3);
        d_shaderInstance->setUniform1iv("displacementTexture", 4);
        d_shaderInstance->setUniform1iv("emissiveTexture", 5);
        d_shaderInstance->setUniform3fv("camPos", m_camera.position);
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
	for (const DrawPackage& package : _forwardQueue)
	{
		if (Texture2D* tex = m_textureCache.getTextureByID(package.textureID))
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex->getGLTextureID());
		}
		GLUtils::setBlendMode(package.blendMode);
		GLUtils::setDepthMode(package.depthMode);

        const VertexDataType packageType = package.buffer->getType();
        if (packageType == VertexDataType::SpriteVerts)
		{
            f_shaderSprite->begin();
            f_shaderSprite->setUniformM4fv("View", view);
            f_shaderSprite->setUniformM4fv("Projection", projection);
            f_shaderSprite->setUniform3fv("CameraPosition", position);

			glBindVertexArray(m_sprite_vao);
			glBindBuffer(GL_ARRAY_BUFFER, package.buffer->getVBO());
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(4 * sizeof(GLfloat)));
            if (package.data)
            {
                glBufferData(GL_ARRAY_BUFFER, package.rangeEnd * sizeof(ColorVertexData), package.data, GL_DYNAMIC_DRAW);
            }
			glDrawArrays(GL_POINTS, package.rangeStart, package.rangeEnd);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

            f_shaderSprite->end();
			renderedSprites = package.rangeEnd;
		}
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void RendererGLProg::flushTexturedVertsQueue()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(ui_vao);
    glBindBuffer(GL_ARRAY_BUFFER, ui_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertexData), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertexData), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    f_shaderUI_tex->begin();
    f_shaderUI_tex->setUniformM4fv("MVP", mvp2D);
    f_shaderUI_tex->setUniform1iv("textureMap", 0);

    for (const auto& pair : m_texturedVertexPackages)
    {
        const TextureID textureID = pair.first;
        const TexturedVertsDataPackage& package = pair.second;
        const Texture2D* texture = m_textureCache.getTextureByID(textureID);
        if (!texture)
        {
            continue;
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture->getGLTextureID());

        glBufferData(GL_ARRAY_BUFFER, sizeof(TexturedVertexData) * package.count, package.data, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, package.count);
    }
    f_shaderUI_tex->end();
}

void RendererGLProg::flushTextVertsQueue()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    f_shaderUI_text->begin();
    f_shaderUI_text->setUniformM4fv("MVP", mvp2D);
    f_shaderUI_text->setUniform1iv("textureMap", 0);
    
    for (const auto& pair : m_textVertexPackages)
    {
        const TextureID textureID = pair.first;
        const TexturedVertsDataPackage& package = pair.second;
        const Texture2D* texture = m_textureCache.getTextureByID(textureID);
        if (!texture)
        {
            continue;
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture->getGLTextureID());
        glBindVertexArray(ui_vao);
        glBindBuffer(GL_ARRAY_BUFFER, ui_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(TexturedVertexData) * package.count, package.data, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertexData), 0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertexData), (GLvoid*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glDrawArrays(GL_TRIANGLES, 0, package.count);
    }
    f_shaderUI_text->end();
}

void RendererGLProg::renderBloom()
{
    glViewport(0, 0, windowWidth * LIGHT_SCALE, windowHeight * LIGHT_SCALE);

    glDisable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);
    // Bright-pass, writes to tex 1
    glBindFramebuffer(GL_FRAMEBUFFER, light_fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, light_textures[1], 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, final_texture);
    glBindVertexArray(square2D_vao);
    glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
    // Render bright pixels to light tex
    f_shaderBloomBrightPass->begin();
    f_shaderBloomBrightPass->setUniform1iv("textureMap", 0);
    f_shaderBloomBrightPass->setUniform1fv("texScale", 1.f);
    f_shaderBloomBrightPass->setUniform1fv("threshold", m_options.getOption<float>("r_bloomThreshold"));
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    f_shaderBloomBrightPass->end();
    // Blur (tex 1 to 2)
    glBindFramebuffer(GL_FRAMEBUFFER, light_fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, light_textures[2], 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, light_textures[1]);
    f_shaderBlurGaussian->begin();
    f_shaderBlurGaussian->setUniform2fv("resolution", glm::vec2(windowWidth, windowHeight) * LIGHT_SCALE);
    f_shaderBlurGaussian->setUniform2fv("direction", glm::vec2(1.f, 0.f));
    glBindVertexArray(square2D_vao);
    glBindBuffer(GL_ARRAY_BUFFER, square2D_vbo);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    // Blur vertically - wide (tex 2 to 3)
    glBindFramebuffer(GL_FRAMEBUFFER, light_fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, light_textures[3], 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, light_textures[2]);
    f_shaderBlurGaussian->setUniform2fv("direction", glm::vec2(0.f, 1.f));
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    d_shaderBlurV->end();
    // Add to final image (from light tex 3)
    glViewport(0, 0, windowWidth, windowHeight);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glBindFramebuffer(GL_FRAMEBUFFER, final_fbo);
    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, light_textures[3]);
    RenderFromTexture(light_textures[3]);
    glBindVertexArray(0);
}

void RendererGLProg::renderDebug()
{
    float ar = (float)windowHeight / windowWidth;
    //int ww2 = windowWidth / 2;
    int wh = windowHeight;
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDepthMask(GL_FALSE);
    Rect2D tRect = Rect2D(0.0, 0.0, 1.0, 1.0);
    // Render G-Buffer to screen
    DrawTexture(Rect2D(0, wh - (256 * ar), 256, 256 * ar), tRect, m_gBuffer.GetAlbedo(), mvp2D);
    DrawTexture(Rect2D(256, wh - (256 * ar), 256, 256 * ar), tRect, m_gBuffer.GetMaterial(), mvp2D);
    DrawTexture(Rect2D(512, wh - (256 * ar), 256, 256 * ar), tRect, m_gBuffer.GetNormal(), mvp2D);
    DrawTexture(Rect2D(768, wh - (256 * ar), 256, 256 * ar), tRect, m_gBuffer.GetDepth(), mvp2D);
    DrawTexture(Rect2D(1024, wh - (256 * ar), 256, 256 * ar), tRect, final_texture, mvp2D);

    if (m_options.getOption<bool>("r_debugLights")) { // Render light buffers to screen for debugging
        DrawTexture(Rect2D(0, wh - (512 * ar), 256, 256 * ar), tRect, light_textures[0], mvp2D);
        DrawTexture(Rect2D(256, wh - (512 * ar), 256, 256 * ar), tRect, light_textures[1], mvp2D);
        DrawTexture(Rect2D(512, wh - (512 * ar), 256, 256 * ar), tRect, light_textures[2], mvp2D);
        DrawTexture(Rect2D(768, wh - (512 * ar), 256, 256 * ar), tRect, light_textures[3], mvp2D);
        DrawTexture(Rect2D(1024, wh - (512 * ar), 256, 256 * ar), tRect, light_textures[4], mvp2D);
    }
    if (m_options.getOption<bool>("r_debugShadows")) { // Render shadow buffers to screen for debugging
        
        //DrawTexture(Rect2D(384, wh2 - (1024 * ar), 256, 256 * ar), tRect, shadow_texture);
        const int drawSize = 256;
        // Draw cubemaps
        //DrawCubeMap(Rect2D(-640, wh2 - (512), 256, 256), texCoordsXN, shadow_cubeMap);//-X
        //DrawCubeMap(Rect2D(-384, wh2 - (512), 256, 256), texCoordsZP, shadow_cubeMap);//+Z
        //DrawCubeMap(Rect2D(-128, wh2 - (512), 256, 256), texCoordsXP, shadow_cubeMap);//+X
        //DrawCubeMap(Rect2D(128, wh2 - (512), 256, 256), texCoordsZN, shadow_cubeMap);//-Z
        //DrawCubeMap(Rect2D(-384, wh2 - (256), 256, 256), texCoordsYN, shadow_cubeMap);//-Y
        //DrawCubeMap(Rect2D(-384, wh2 - (768), 256, 256), texCoordsYP, shadow_cubeMap);//+Y
        DrawCubeMap(Rect2D(0, wh - (drawSize * 2), drawSize, drawSize), CubeMapTexCoords[Negative_X], m_reflectionProbe.getCubeMap());//-X
        DrawCubeMap(Rect2D(256, wh - (drawSize * 2), drawSize, drawSize), CubeMapTexCoords[Positive_Z], m_reflectionProbe.getCubeMap());//+Z
        DrawCubeMap(Rect2D(512, wh - (drawSize * 2), drawSize, drawSize), CubeMapTexCoords[Positive_X], m_reflectionProbe.getCubeMap());//+X
        DrawCubeMap(Rect2D(768, wh - (drawSize * 2), drawSize, drawSize), CubeMapTexCoords[Negative_Z], m_reflectionProbe.getCubeMap());//-Z
        DrawCubeMap(Rect2D(1024, wh - (drawSize * 3), drawSize, drawSize), CubeMapTexCoords[Negative_Y], m_reflectionProbe.getCubeMap());//-Y
        DrawCubeMap(Rect2D(256, wh - (drawSize * 1), drawSize, drawSize), CubeMapTexCoords[Positive_Y], m_reflectionProbe.getCubeMap());//+Y
    }
    // debug materials data
    //DrawTexture(Rect2D(-128, -4, 512, 8), tRect, _materialTexture.getTexture(), mvp2D);
}
