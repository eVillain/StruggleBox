#version 400

layout(location = 0) out vec4 color;
layout(location = 1) out vec3 normal;
layout(location = 2) out float depth;

in Fragment {
    smooth vec4 color;
    smooth vec3 normal;
    smooth float depth;
} fragment;
in vec3 sunDirection;

uniform float fogDensity;

const vec4 skyTop = vec4(1.0f, 1.0f, 1.0f, 1.0f);
const vec4 skyHorizon = vec4(0.5f, 0.5f, 0.7f, 1.0f);
//const float fogdensity = .001;

void main(void) {
    // Calculate strength of fog
    const float LOG2 = 1.442695;
    float z = gl_FragCoord.z / gl_FragCoord.w;
    float fogFactor = exp2( -fogDensity * fogDensity * z * z * LOG2 );
    fogFactor = clamp(fogFactor, 0.0, 1.0);
//    if ( color.a < 0.1 ) discard;
	// Wanted color is a mix of the actual color and the fog color
//	color = mix(skyHorizon, fragment.color, fog);
    vec4 sColor = mix(skyHorizon, skyTop, (normalize(sunDirection).y));
	color = mix(sColor, fragment.color, fogFactor);

    // Finally Set the diffuse value (darkness).
    // This is done with a dot product betweenthe normal and the light
    float diffuse_value = max(dot(fragment.normal, sunDirection), 0.0);
    float ambient_value = (1.0+normalize(sunDirection).y)*0.5;
    float light_value = clamp(diffuse_value+ambient_value, 0.1, 1.0);
    color.rgb = color.rgb*light_value;
    color.a = fragment.color.a;
    normal = fragment.normal;
    depth = fragment.depth;
}
//
//void main(void) {
//    // Calculate strength of fog
//	float z = gl_FragCoord.z / gl_FragCoord.w;
//	float fog = clamp(exp(-fogdensity * z * z), 0.2, 1);
//    
//	// Final color is a mix of the actual color and the fog color
//	color = mix(skyHorizon, fragment.color, fog);
//    normal = fragment.normal;
//    depth = fragment.depth;
//}
