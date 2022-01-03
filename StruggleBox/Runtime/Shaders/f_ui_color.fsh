#version 400

// Ouput data
out vec4 color;

// Interpolated values from the vertex shaders
in vec4 fragmentColor;

void main(){
	// Output color = color specified in the vertex shader, 
	// interpolated between all 3 surrounding vertices
	color = fragmentColor;
}