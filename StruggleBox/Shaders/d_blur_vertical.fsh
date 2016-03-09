#version 400

layout(location = 0) out vec4 color;

uniform sampler2D Texture;
uniform int Width;
uniform float odh;

in vec2 texCoord;

void main()
{
	vec3 Color = vec3(0.0);
	int wp1 = Width + 1;
	float Sum = 0.0;
	
	for(int y = -Width; y <= Width; y++)
	{
		float width = (wp1 - abs(float(y)));
		Color += texture(Texture, texCoord.st + vec2(0.0, odh * y)).rgb * width;
		Sum += width;
	}
	
	color = vec4(Color / Sum, 1.0);
}
