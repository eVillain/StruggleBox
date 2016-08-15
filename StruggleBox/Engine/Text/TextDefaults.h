#ifndef TEXT_DEFAULTS_H
#define TEXT_DEFAULTS_H

#include <string>

/**********************************************************************
 * Default text rendering shader programs
 *********************************************************************/
static const std::string text_vertex_shader_forward = {
    "#version 400\n"
    "layout(location = 0) in vec4 vCoord;"
    "layout(location = 1) in vec2 tCoord;"
    "uniform mat4 MVP;"
    "out vec2 texCoord;"
    "void main(void) {"
    "gl_Position = MVP * vec4(vCoord.xyz,1);"
    "texCoord = tCoord;"
    "}"
};

static const std::string text_frag_shader_forward = {
    "#version 400\n"
    "in vec2 texCoord;"
    "out vec4 fragColor;"
    "uniform sampler2D tex;"
    "uniform vec4 color;"
    "void main(void) {"
    "float intensity = texture(tex, texCoord).r;"
    "if (intensity <= 0.001) discard;"
    "fragColor = vec4(1, 1, 1, intensity) * color;"
    "}"
};

static const std::string text_vertex_shader_deferred = {
    "#version 400\n"
    "layout(location = 0) in vec4 vCoord;"
    "layout(location = 1) in vec2 tCoord;"
    "uniform mat4 MVP;"
    "out vec2 texCoord;"
    "out vec3 normal;"
    "out float depth;"
    "void main(void) {"
    "gl_Position = MVP * vec4(vCoord.xyz,1);"
    "texCoord = tCoord;"
    "normal = (MVP*vec4(0,0,1,1)).xyz;"
    "depth = gl_Position.z;"
    "}"
};

static const std::string text_frag_shader_deferred = {
    "#version 400\n"
    "in vec2 texCoord;"
    "in vec3 normal;"
    "in float depth;"
    "layout(location = 0) out vec4 fragColor;"
    "layout(location = 1) out vec4 specularColor;"
    "layout(location = 2) out vec3 fragNormal;"
    "layout(location = 3) out float fragDepth;"
    "uniform sampler2D tex;"
    "uniform vec4 color;"
    "void main(void) {"
    "float intensity = texture(tex, texCoord).r;"
    "if (intensity <= 0.001) discard;"
    "fragColor = vec4(1, 1, 1, intensity) * color;"
    "specularColor = vec4(0, 0, 0, 0);"
    "fragNormal = normal;"
    "fragDepth = depth;"
    "}"
};

#endif
