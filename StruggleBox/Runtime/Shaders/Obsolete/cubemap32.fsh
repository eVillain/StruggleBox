#version 330 core

// Interpolated values from the vertex shaders
in vec3 Position;

// Ouput data
out vec4 color;

uniform samplerCube CubeMap;

void main() {
    color = texture(CubeMap, Position);
}
