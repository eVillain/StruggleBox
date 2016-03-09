#version 400

layout (location = 0) in vec4 v_vertex;
layout (location = 1) in vec4 v_diffColor;
layout (location = 2) in float v_specColor;
layout (location = 3) in vec3 v_normal;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform vec3 instance_position;

out Fragment {  // Output to fragment shader
    vec4 diffColor;
    float specColor;
    smooth vec3 normal;
    smooth vec3 wPos;
    float depth;
} fragment;


void main(void) {
    vec4 vertex;
    vertex.xyz = v_vertex.xyz + instance_position;
    vertex.w = 1.0;
    fragment.wPos = vertex.xyz;  // Save world position
    gl_Position = MVP*(vertex);
    
    // v_vertex.w contains vertex light intensity
    // (for minecraft-style ambient occlusion)
    fragment.diffColor = vec4(v_diffColor.rgb*v_vertex.w, v_diffColor.a);
    fragment.specColor = v_specColor * v_vertex.w;

    fragment.normal = v_normal;
    fragment.depth = gl_Position.z;
}

