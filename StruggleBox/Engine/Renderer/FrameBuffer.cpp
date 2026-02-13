#include "FrameBuffer.h"

#include "ArenaOperators.h"
#include "CoreIncludes.h"
#include "RenderCore.h"
#include "GLUtils.h"
#include "Texture2D.h"

FrameBuffer::FrameBuffer(RenderCore& renderCore, const std::string& name)
	: m_renderCore(renderCore)
    , m_fboHandle(0)
	, m_textureHandle(0)
    , m_width(0)
    , m_height(0)
    , m_textureName(name)
    , m_textureID(0)
{
}

FrameBuffer::~FrameBuffer()
{
}

void FrameBuffer::initialize(const uint32_t width, const uint32_t height)
{
    setupTexture(width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::initialize(const uint32_t width, const uint32_t height, const uint32_t depthTexture)
{
    setupTexture(width, height);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depthTexture, 0);  // Attach previous depth/stencil to it
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::terminate()
{
    glDeleteTextures(1, &m_textureHandle);
    glDeleteFramebuffers(1, &m_fboHandle);

    //m_renderCore.removeTexture(m_textureID);
    Texture2D* textureAlbedo = m_renderCore.getTextureCache().getTextureByID(m_textureID);
    m_renderCore.getTextureCache().removeTexture(m_textureID);
    CUSTOM_DELETE(textureAlbedo, m_renderCore.getAllocator());
}

void FrameBuffer::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboHandle);
}

void FrameBuffer::bindAndClear(const Color& clearColor) const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboHandle);
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void FrameBuffer::setupTexture(const uint32_t width, const uint32_t height)
{
    m_width = width;
    m_height = height;
    m_textureHandle = GLUtils::createTexture(width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
  
    Texture2D* textureAlbedo = CUSTOM_NEW(Texture2D, m_renderCore.getAllocator())(m_textureHandle, m_width, m_height, GL_RGBA, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR, 0);
    m_textureID = m_renderCore.getTextureCache().addTexture(textureAlbedo, m_textureName);

    glGenFramebuffers(1, &m_fboHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboHandle);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_textureHandle, 0);
}
