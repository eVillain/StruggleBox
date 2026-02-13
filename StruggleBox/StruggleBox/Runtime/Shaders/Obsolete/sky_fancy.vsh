#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vCoord;
// Output data, will be interpolated for each fragment.
smooth out vec3 Vertex;
// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform vec3 CameraPosition;
uniform float Time;

//void main() {
//    Vertex = vec3(vCoord.x, vCoord.y, vCoord.z);
//    gl_Position = MVP * vec4(vCoord.xyz + CameraPosition, 1.0);
//    gl_Position.z = gl_Position.w -0.00001; //fix to far plane.
//}

attribute vec4 Tangent;

varying vec4 fragPos;
varying vec3 wT, wB, wN; // tangent, binormal, normal
varying vec3 wPos, pos, viewPos, sunPos;

uniform mat4 ModelMatrix;
uniform vec3 cameraPos;

mat3 m3( mat4 m )
{
	mat3 result;
	
	result[0][0] = m[0][0];
	result[0][1] = m[0][1];
	result[0][2] = m[0][2];
    
	result[1][0] = m[1][0];
	result[1][1] = m[1][1];
	result[1][2] = m[1][2];
	
	result[2][0] = m[2][0];
	result[2][1] = m[2][1];
	result[2][2] = m[2][2];
	
	return result;
}

void main()
{
    wPos = vec3(ModelMatrix * vCoord);
	//pos = vec3(gl_Vertex);
    
	wT   = m3(ModelMatrix)*Tangent.xyz;
	wB   = m3(ModelMatrix)*cross(gl_Normal, Tangent.xyz);
	wN   = m3(ModelMatrix)*gl_Normal;
    
    //fragPos = ftransform();
    viewPos = wPos - cameraPos.xyz;
    sunPos = m3(ModelMatrix)*vec3(gl_ModelViewMatrixInverse*gl_LightSource[0].position);
    gl_Position = ftransform();
}
