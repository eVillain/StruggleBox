// Atmospheric scattering fragment shader
// Author: Sean O'Neil
// Copyright (c) 2004 Sean O'Neil

#version 400

layout(location = 0) out vec4 diffuseColor;
layout(location = 1) out vec4 specularColor;
layout(location = 2) out vec3 normal;
layout(location = 3) out float depth;

in Fragment {
    vec3 v3Direction;
    vec4 frontColor;
    vec4 frontSecondaryColor;
    vec2 texCoord;
    float depth;
} fragment;

uniform vec3 v3LightPos;
uniform float g;
uniform float g2;
uniform sampler2D skyTexture;
//uniform sampler2D depthMap;

void main (void)
{
	float fCos = dot(v3LightPos, fragment.v3Direction) / length(fragment.v3Direction);
	float fRayleighPhase = 1.0 + fCos * fCos;
	float fMiePhase = (1.0 - g2) / (2.0 + g2) * (1.0 + fCos * fCos) / pow(1.0 + g2 - 2.0 * g * fCos, 1.5);
	diffuseColor = vec4(1.0 - exp(-1.5 * (fRayleighPhase * fragment.frontColor.rgb + fMiePhase * fragment.frontSecondaryColor.rgb)), 1.0);
    specularColor = diffuseColor;
    // If textured
//    color += vec4(texCoord.x,texCoord.y,0,1);
//    color = vec4(texture(skyTexture, texCoord).rgb,1);
//    vec3 skyCol = abs( vec3(1.0 - exp(-1.5 * (fRayleighPhase * frontColor.rgb + fMiePhase * frontSecondaryColor.rgb))) );
//    if (skyCol.r <= 1.0 && skyCol.r > 0.0) color.r += skyCol.r;
//    if (skyCol.g <= 1.0 && skyCol.g > 0.0) color.g += skyCol.g;
//    if (skyCol.b <= 1.0 && skyCol.b > 0.0) color.b += skyCol.b;
    normal = vec3(0.0);
    depth = fragment.depth;
}
