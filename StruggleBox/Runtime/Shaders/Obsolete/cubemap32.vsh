#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vCoord;

// Output data ; will be interpolated for each fragment.
out vec3 Position;
// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform vec3 CameraPosition;

void main() {
    Position = vec3(-vCoord.x, -vCoord.y, vCoord.z);
    gl_Position = MVP * vec4(vCoord.xyz + CameraPosition, 1.0);
    gl_Position.z = gl_Position.w -0.00001; //fix to far plane.
}
