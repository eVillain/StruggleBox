#version 330 core

layout(location = 0) out vec4 color;
layout(location = 1) out float depth;

in VertexData {
    vec4 vColor;
    vec3 normal;
    float intensity;
} VertexOut;

const vec4 skyHorizon = vec4(0.3294f, 0.92157f, 1.0f, 1.0f);
const vec4 fogcolor = vec4(0.6, 1.0, 0.7, 1.0);
const float fogdensity = .0005;

const float pi = 3.14159;
uniform float waterHeight;
uniform float time;

void main(void) {
	vec4 fragColor = VertexOut.vColor;
    
	// Attenuate
	fragColor.rgb *= VertexOut.intensity;
    
	// Calculate strength of fog
	float z = gl_FragCoord.z / gl_FragCoord.w;
	float fog = clamp(exp(-fogdensity * z * z), 0.03, 2);
    fog += sin(time*pi)*0.05;
	// Final color is a mix of the actual color and the fog color
	color = mix(fogcolor, fragColor, fog);
}

