#version 400

out vec4 color;

in Fragment {
    vec3 texCoord;
} fragment;

uniform samplerCube textureMap;
uniform float texScale = 1.0;
uniform vec4 u_color;

void main(){
	// Output color = color specified in the vertex shader, 
	// interpolated between all 3 surrounding vertices
	color = vec4(texture(textureMap, fragment.texCoord*texScale).r)* u_color;
    if ( color.a < 0.0001 ) discard;
	//color = vec4(texCoord.x,texCoord.y,0,1);
}
