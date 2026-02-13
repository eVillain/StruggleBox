#version 400

// Interpolated values from the vertex shaders
in vec4 fragmentColor;
in vec2 vTexCoord;

// Ouput data
out vec4 color;
// Input data
uniform float exposure;
uniform float decay;
uniform float density;
uniform float weight;
uniform vec2 lightScrnPos;
uniform sampler2D textureMap;

const int NUM_SAMPLES = 100;

void main() {
 	color = vec4(0,0,0,0);    // Clear color or else we get noise on Intel GPUS
    vec2 deltaTextCoord = vec2( vTexCoord-lightScrnPos );

 	vec2 textCoord = vTexCoord;
 	deltaTextCoord *= 1.0 / (float(NUM_SAMPLES) * density);
 	float illuminationDecay = 1.0;
    
 	for(int i=0; i < NUM_SAMPLES ; i++) {
        textCoord -= deltaTextCoord;
        vec4 texSample = texture(textureMap, textCoord);
        texSample *= illuminationDecay * weight;
        color += texSample;
        illuminationDecay *= decay;
 	}
 	color *= exposure;
}
