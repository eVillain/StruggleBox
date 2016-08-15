#version 400

out vec4 color;

in Fragment {
    vec2 texCoord;
} fragment;

uniform sampler2D textureMap;
uniform float texScale = 1.0;
uniform vec4 u_color;

void main(){
	// Output color = color specified in the vertex shader, 
	// interpolated between all 3 surrounding vertices
	color = texture(textureMap, fragment.texCoord*texScale);
    if ( color.a < 0.0001 ) discard;
    color *= u_color;
}
