#version 400

layout(location = 0) out vec4 color;

uniform sampler2D Texture;
uniform int Width;
uniform float odw;

in vec2 texCoord;

void main()
{
	vec3 Color = vec3(0.0);
	int wp1 = Width + 1;
	float Sum = 0.0;
	
	for(int x = -Width; x <= Width; x++)
	{
		float width = (wp1 - abs(float(x)));
		Color += texture(Texture, texCoord.st + vec2(odw * x, 0.0)).rgb * width;
		Sum += width;
	}
	
	color = vec4(Color / Sum, 1.0);
}
