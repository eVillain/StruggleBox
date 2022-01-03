#version 400

layout (location = 0) in vec4 v_vertex;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec3 v_tangent;
layout (location = 3) in vec2 v_uv;
layout (location = 4) in vec2 v_materialOffset;

uniform mat4 MVP;
uniform vec3 instance_position = vec3(0);

out Fragment {
  smooth vec2 uv;
  smooth float depth;
  vec2 material;
  smooth vec3 fragPos;
  mat3 tbn;
} fragment;

void main(void)
 {
    vec4 vertex = vec4(v_vertex.xyz + instance_position, 1.0);
    gl_Position = MVP*(vertex);

    // v_vertex.w contains vertex light intensity
    // (for minecraft-style ambient occlusion)
    // fragment.albedo = vec4(albedo.rgb*(1.0-v_vertex.w), albedo.a);
    
    fragment.uv = v_uv;
    fragment.depth = gl_Position.z;
    fragment.material = v_materialOffset;
    fragment.fragPos = vertex.xyz;

    mat3 modelMatrix = mat3(1.0);
    vec3 bitangent = cross(v_normal, v_tangent);
    fragment.tbn = mat3(normalize(vec3(modelMatrix * v_tangent)),
                        normalize(vec3(modelMatrix * bitangent)),
                        normalize(vec3(modelMatrix * v_normal)));
}
