#version 400

layout(location = 0) out vec4 albedoOut;
layout(location = 1) out vec4 materialOut;
layout(location = 2) out vec3 normalOut;
layout(location = 3) out float depthOut;

in Fragment {
    vec4 albedo;
    vec4 material;
    smooth vec3 normal;
    smooth vec3 wPos;
    float depth;
} fragment;

#define GRID_LINES 0

void main(void) {
    albedoOut = fragment.albedo;
    materialOut = fragment.material;

    normalOut = (fragment.normal+1.0)*0.5; // Save normal as value between 0 and 1, not -1.0 and 1.0
    depthOut = fragment.depth;

    // Grid - darken material color around edges
    float gridMultiplier = 1.0;
#if (GRID_LINES)
    const float lineRadius = 0.025;
    const float lineRadiusNeg = 1.0-lineRadius;
    const float lineFactor = 1.0/lineRadius;
    if ( fragment.normal.x > 0.9 ||
        fragment.normal.x < -0.9 ) {
        float gridLineY = mod(fragment.wPos.y*2.0,1.0);
        if ( gridLineY > lineRadiusNeg ) { gridMultiplier += (lineRadiusNeg-gridLineY)*lineFactor; }
        else if ( gridLineY < lineRadius ) { gridMultiplier -= (lineRadius-gridLineY)*lineFactor; }
        float gridLineZ = mod(fragment.wPos.z*2.0,1.0);
        if ( gridLineZ > lineRadiusNeg ) { gridMultiplier += (lineRadiusNeg-gridLineZ)*lineFactor; }
        else if ( gridLineZ < lineRadius ) { gridMultiplier -= (lineRadius-gridLineZ)*lineFactor; }
    }
    else if ( fragment.normal.y > 0.9 ||
             fragment.normal.y < -0.9 ) {
        float gridLineX = mod(fragment.wPos.x*2.0,1.0);
        if ( gridLineX > lineRadiusNeg ) { gridMultiplier += (lineRadiusNeg-gridLineX)*lineFactor; }
        else if ( gridLineX < lineRadius ) { gridMultiplier -= (lineRadius-gridLineX)*lineFactor; }
        float gridLineZ = mod(fragment.wPos.z*2.0,1.0);
        if ( gridLineZ > lineRadiusNeg ) { gridMultiplier += (lineRadiusNeg-gridLineZ)*lineFactor; }
        else if ( gridLineZ < lineRadius ) { gridMultiplier -= (lineRadius-gridLineZ)*lineFactor; }
    }
    else if ( fragment.normal.z > 0.9 ||
             fragment.normal.z < -0.9 ) {
        float gridLineY = mod(fragment.wPos.y*2.0,1.0);
        if ( gridLineY > lineRadiusNeg ) { gridMultiplier += (lineRadiusNeg-gridLineY)*lineFactor; }
        else if ( gridLineY < lineRadius ) { gridMultiplier -= (lineRadius-gridLineY)*lineFactor; }
        float gridLineX = mod(fragment.wPos.x*2.0,1.0);
        if ( gridLineX > lineRadiusNeg ) { gridMultiplier += (lineRadiusNeg-gridLineX)*lineFactor; }
        else if ( gridLineX < lineRadius ) { gridMultiplier -= (lineRadius-gridLineX)*lineFactor; }
    }
    albedoOut.rgb = albedoOut.rgb*gridMultiplier;
#endif
}
