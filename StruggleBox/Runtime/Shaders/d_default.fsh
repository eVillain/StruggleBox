#version 400

layout(location = 0) out vec4 diffuseColor;
layout(location = 1) out vec4 specularColor;
layout(location = 2) out vec3 normal;
layout(location = 3) out float depth;

// Interpolated values from the vertex shaders
in Fragment {
    vec4 color;
    in float depth;
} fragment;

void main(){
	// Output color = color specified in the vertex shader, 
	// interpolated between all 3 surrounding vertices
	diffuseColor = fragment.color;
    specularColor = fragment.color;
    normal = vec3(0.0,1.0,0.0);
    depth = fragment.depth;
}
