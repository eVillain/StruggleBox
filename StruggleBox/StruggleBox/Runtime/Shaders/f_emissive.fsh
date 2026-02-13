#version 400

out vec4 fragColor;

// Interpolated values from the vertex shaders
in vec4 fragmentColor;

void main()
{
		fragColor = fragmentColor;
}
