#version 400

// Ouput data
layout(location = 0) out float fragmentDepth;

void main(){
    // Not really needed, OpenGL does it anyway
    fragmentDepth = gl_FragCoord.z;
}
