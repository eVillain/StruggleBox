#version 400

layout(location = 0) out vec4 diffuseColor;
layout(location = 1) out vec4 specularColor;
layout(location = 2) out vec3 normal;
layout(location = 3) out float depth;

in Fragment {
    smooth vec4 diffuse;
    smooth float specular;
    smooth vec3 normal;
    smooth float depth;
} fragment;
//in vec3 sunDirection;

//const vec4 skyTop = vec4(1.0f, 1.0f, 1.0f, 1.0f);
//const vec4 skyHorizon = vec4(0.5f, 0.5f, 0.7f, 1.0f);
//uniform float fogDensity;

void main(void) {
    // Calculate strength of fog
//    const float LOG2 = 1.442695;
//    float z = gl_FragCoord.z / gl_FragCoord.w;
//    float fogFactor = exp2( -fogDensity * fogDensity * z * z * LOG2 );
//    fogFactor = clamp(fogFactor, 0.0, 1.0);
    
////	float z = gl_FragCoord.z / gl_FragCoord.w;
////	float fog = clamp(exp(-fogDensity * z * z), 0.001, 1.0);
////    if ( color.a < 0.1 ) discard;
//	// Wanted color is a mix of the actual color and the fog color
////	color = mix(skyHorizon, fragment.color, fog);
//    vec4 sColor = mix(skyHorizon, skyTop, (normalize(sunDirection).y));
//	vec4 color = mix(sColor, fragment.color, fogFactor);
//
//    // Finally Set the diffuse value (darkness).
//    // This is done with a dot product between the normal and the light
//    float diffuse_value = max(dot(fragment.normal, sunDirection), 0.0);
//    float ambient_value = (1.0+normalize(sunDirection).y)*0.5;
//    float light_value = clamp(diffuse_value+ambient_value, 0.1, 1.0);
//    color.rgb = color.rgb*light_value;
//    color.a = fragment.color.a;
    normal = (fragment.normal+1.0)*0.5;
    diffuseColor = fragment.diffuse;
    specularColor = vec4(vec3(fragment.specular),1.0);
    depth = fragment.depth;
}
