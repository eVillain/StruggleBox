#ifndef CUBE_CONSTANTS_H
#define CUBE_CONSTANTS_H

#include "CoreIncludes.h"

// Sides convention is always:
// left, right, bottom, top, back, front (-x, +x, -y, +y, -z, +z)
namespace CubeConstants {
    static const glm::vec4 cube_verts[36] = {
        // left
        glm::vec4(-0.5,-0.5,-0.5, 1.0),
        glm::vec4(-0.5,-0.5, 0.5, 1.0),
        glm::vec4(-0.5, 0.5, 0.5, 1.0),
        glm::vec4(-0.5, 0.5, 0.5, 1.0),
        glm::vec4(-0.5, 0.5,-0.5, 1.0),
        glm::vec4(-0.5,-0.5,-0.5, 1.0),
        // right
        glm::vec4(0.5,-0.5, 0.5, 1.0),
        glm::vec4(0.5,-0.5,-0.5, 1.0),
        glm::vec4(0.5, 0.5,-0.5, 1.0),
        glm::vec4(0.5, 0.5,-0.5, 1.0),
        glm::vec4(0.5, 0.5, 0.5, 1.0),
        glm::vec4(0.5,-0.5, 0.5, 1.0),
        // bottom
        glm::vec4(-0.5,-0.5,-0.5, 1.0),
        glm::vec4(0.5,-0.5,-0.5, 1.0),
        glm::vec4(0.5,-0.5, 0.5, 1.0),
        glm::vec4(0.5,-0.5, 0.5, 1.0),
        glm::vec4(-0.5,-0.5, 0.5, 1.0),
        glm::vec4(-0.5,-0.5,-0.5, 1.0),
        // top
        glm::vec4(-0.5, 0.5, 0.5, 1.0),
        glm::vec4(0.5, 0.5, 0.5, 1.0),
        glm::vec4(0.5, 0.5,-0.5, 1.0),
        glm::vec4(0.5, 0.5,-0.5, 1.0),
        glm::vec4(-0.5, 0.5,-0.5, 1.0),
        glm::vec4(-0.5, 0.5, 0.5, 1.0),
        // back
        glm::vec4(-0.5, 0.5,-0.5, 1.0),
        glm::vec4(0.5, 0.5,-0.5, 1.0),
        glm::vec4(0.5,-0.5,-0.5, 1.0),
        glm::vec4(0.5,-0.5,-0.5, 1.0),
        glm::vec4(-0.5,-0.5,-0.5, 1.0),
        glm::vec4(-0.5, 0.5,-0.5, 1.0),
        // front
        glm::vec4(-0.5,-0.5, 0.5, 1.0),
        glm::vec4(0.5,-0.5, 0.5, 1.0),
        glm::vec4(0.5, 0.5, 0.5, 1.0),
        glm::vec4(0.5, 0.5, 0.5, 1.0),
        glm::vec4(-0.5, 0.5, 0.5, 1.0),
        glm::vec4(-0.5,-0.5, 0.5, 1.0),
    };

    static const glm::vec3 cube_normals[6] = {
        glm::vec3(-1.f, 0.f, 0.f), // left
        glm::vec3( 1.f, 0.f, 0.f), // right
        glm::vec3( 0.f,-1.f, 0.f), // bottom
        glm::vec3( 0.f, 1.f, 0.f), // top
        glm::vec3( 0.f, 0.f,-1.f), // back
        glm::vec3 (0.f, 0.f, 1.f), // front
    };

    static const glm::vec3 cube_tangents[6] = {
        glm::vec3( 0.f, 0.f,-1.f), // left v
        glm::vec3( 0.f, 0.f, 1.f), // right v
        glm::vec3(-1.f, 0.f, 0.f), // bottom v
        glm::vec3( 1.f, 0.f, 0.f), // top v
        glm::vec3( 1.f, 0.f, 0.f), // back 
        glm::vec3(-1.f, 0.f, 0.f), // front
    };

    static const glm::vec2 cube_uvs[36] = {
        // left
        glm::vec2(1.0f, 1.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec2(0.0f, 0.0f),
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
        // right
        glm::vec2(1.0f, 1.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec2(0.0f, 0.0f),
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
        // bottom
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        // top
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        // back
        glm::vec2(1.0f, 1.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec2(0.0f, 0.0f),
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
        // front
        glm::vec2(1.0f, 1.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec2(0.0f, 0.0f),
        glm::vec2(0.0f, 0.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(1.0f, 1.0f),
    };

    static const int cube_indices[] = {
    6, 2, 0, 0, 4, 6, // left
    3, 7, 5, 5, 1, 3, // right
    1, 5, 4, 4, 0, 1, // bottom
    2, 6, 7, 7, 3, 2, // top
    2, 3, 1, 1, 0, 2, // rear
    7, 6, 4, 4, 5, 7, // front
    };

    static const GLfloat raw_cube_vertices[] = {
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

    static const GLfloat raw_cube_normals[] = {
        // front
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        // right
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        // back
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        // left
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        // bottom
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        // top
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
    };

    static const GLfloat raw_cube_tangents[] = {
        // front
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        // right
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        // back
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        // left
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        // bottom
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        // top
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
    };

    static const GLfloat raw_cube_texcoords[] = {
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
};

#endif
