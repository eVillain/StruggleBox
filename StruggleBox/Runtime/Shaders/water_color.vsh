#version 400

layout (location = 0) in vec4 v_vertex;
layout (location = 1) in vec4 v_color;
layout (location = 2) in vec3 v_normal;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform vec3 sunPosition;
uniform float waveHeight;
// Water stuff
const float pi = 3.14159;
const float waveScale = 1.0;
//const float waveHeight = 1.0;

//uniform float waterHeight;
uniform float time;

out Fragment {
    smooth vec4 color;
    smooth vec3 normal;
    smooth float depth;
} fragment;
out vec3 sunDirection;


void main(void) {
    vec4 vertex;
    vertex.xyz = v_vertex.xyz;
    vertex.w = 1.0;
    gl_Position = MVP*(vertex);
    gl_Position.y += sin(vertex.x*waveScale+time)*waveHeight;  // wave x
    gl_Position.y += sin(vertex.z*waveScale+time)*waveHeight;  // wave z

    fragment.color = vec4(v_color.rgb, v_color.a);
    fragment.normal = normalize( v_normal);
    fragment.depth = gl_Position.z;
    
    // Calculate sun direction from position
    sunDirection = normalize((sunPosition*149600000.0)-gl_Position.xyz);
}
