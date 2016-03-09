#version 400

layout(location = 0) out vec4 diffuseColor;
layout(location = 1) out vec4 specularColor;
layout(location = 2) out vec3 normal;
layout(location = 3) out float depth;

in Fragment {
    vec4 diffColor;
    float specColor;
    smooth vec3 normal;
    smooth vec3 wPos;
    float depth;
} fragment;

uniform bool renderGrid = false;

void main(void) {
    diffuseColor = fragment.diffColor;
    specularColor = vec4(vec3(fragment.specColor),1.0);
    
    normal = (fragment.normal+1.0)*0.5; // Save normal as value between 0 and 1, not -1.0 and 1.0
    depth = fragment.depth;
    
    // Grid - darken material color around edges
    float gridMultiplier = 1.0;
    if ( renderGrid ) {
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
        diffuseColor.rgb = diffuseColor.rgb*gridMultiplier;
    }
}
