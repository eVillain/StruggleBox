#version 400

// Interpolated values from the vertex shaders
in vec4 fragmentColor;
in vec2 texCoord;

uniform sampler2D textureMap;
uniform float nearDepth;    // Camera z near
uniform float farDepth;     // Camera z far

// Ouput data
out vec4 color;

float LinearizeDepth(vec2 uv) {
    float z = texture(textureMap, uv).x;
    return (2.0 * nearDepth) / (farDepth + nearDepth - z * (farDepth - nearDepth));
}

void main() {
	// Output color = color specified in the vertex shader, 
	color = LinearizeDepth(texCoord) * fragmentColor;
}

