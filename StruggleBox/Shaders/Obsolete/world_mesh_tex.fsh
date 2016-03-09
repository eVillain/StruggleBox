#version 330 core

layout(location = 0) out vec4 color;
//layout(location = 1) out float depth;


// Input vertex
//in vec4 vTexCoord;
//in float depthVal;
//
// Output color
//out vec4 fragColor;
//
//uniform sampler2D textureMap;
//
//void main(void) {
//    vec2 coord2d;
//    float intensity;
//    // If the texture index is negative, it is a top or bottom face, otherwise a side face
//    // Side faces are less bright than top faces, simulating a sun at noon
//    if(vTexCoord.w < 0.0) {
//        coord2d = vec2((fract(vTexCoord.x) + vTexCoord.w) / 20.0, vTexCoord.z);
//        intensity = 1.0;
//    } else {
//        coord2d = vec2((fract(vTexCoord.x + vTexCoord.z) + vTexCoord.w) / 20.0, vTexCoord.y);
//        intensity = 0.85;
//    }
//    vec4 tex = texture(textureMap, coord2d);
//    color = vec4(tex.rgb*intensity, tex.a);
////     if(color.a < 0.5)
////    discard;
////    depth = depthVal;
//}

in VertexData {
    vec4 texCoord;
    vec3 normal;
    float intensity;
} VertexOut;

//varying vec4 texCoord;
//varying vec3 normal;
//varying float intensity;
uniform sampler2D textureMap;

const vec4 fogcolor = vec4(0.6, 0.8, 1.0, 1.0);
const float fogdensity = .00003;

void main(void) {
	vec4 fragColor;
    vec2 coord2d;
	// If the texture index is negative, it is a top or bottom face, otherwise a side face
	// Side faces are less bright than top faces, simulating a sun at noon
	if(VertexOut.normal.y != 0) {
//		fragColor = texture(textureMap, vec3(VertexOut.texCoord.x, VertexOut.texCoord.z, (VertexOut.texCoord.w + 0.5) / 16.0));
        coord2d = vec2((fract(VertexOut.texCoord.x) + VertexOut.texCoord.w) / 20.0, VertexOut.texCoord.z);
	} else {
//		fragColor = texture(textureMap, vec3(VertexOut.texCoord.x + VertexOut.texCoord.z, -VertexOut.texCoord.y, (VertexOut.texCoord.w + 0.5) / 16.0));
        coord2d = vec2((fract(VertexOut.texCoord.x + VertexOut.texCoord.z) + VertexOut.texCoord.w) / 20.0, VertexOut.texCoord.y);
	}
	fragColor = texture(textureMap, coord2d);
	// Very cheap "transparency": don't draw pixels with a low alpha value
	if(fragColor.a < 0.4)
		discard;
    
	// Attenuate
	fragColor *= VertexOut.intensity;
    
	// Calculate strength of fog
	float z = gl_FragCoord.z / gl_FragCoord.w;
	float fog = clamp(exp(-fogdensity * z * z), 0.2, 1);
    
	// Final color is a mix of the actual color and the fog color
	color = mix(fogcolor, fragColor, fog);
}

