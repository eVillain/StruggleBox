#version 330 core

// Vertex shader for shadow mapping of colored cubes
// "vertex" contains one of the vertices of the cube
// "instance_position" contains the cube center in xyz and size in w
// "instance_color" contains the cube color in rgba values ( ignored )
layout (location = 0) in vec4 vertex;
layout (location = 1) in vec4 instance_position;
layout (location = 2) in vec4 instance_rotation;
layout (location = 3) in vec4 instance_color;

// Values that stay constant for the whole mesh.
uniform mat4 depthMVP;

vec4 multQ( vec4 oq, vec4 rq ) {
	// the constructor takes its arguments as (x, y, z, w)
	return vec4(oq.w * rq.x + oq.x * rq.w + oq.y * rq.z - oq.z * rq.y,
    oq.w * rq.y + oq.y * rq.w + oq.z * rq.x - oq.x * rq.z,
    oq.w * rq.z + oq.z * rq.w + oq.x * rq.y - oq.y * rq.x,
    oq.w * rq.w - oq.x * rq.x - oq.y * rq.y - oq.z * rq.z);
}
vec4 getConjugate( vec4 q ) { return vec4(-q.x, -q.y, -q.z, q.w); }
// Multiplying a quaternion q with a vector v applies the q-rotation to v
vec3 rotate(vec3 vert, vec4 rot) {
	vec3 vn = vec3(vert);
	vn = normalize(vn);
    
	vec4 vecQuat, resQuat;
	vecQuat.x = vn.x;
	vecQuat.y = vn.y;
	vecQuat.z = vn.z;
	vecQuat.w = 0.0f;
    
	resQuat = multQ( vecQuat, getConjugate(rot) );
	resQuat = multQ( rot, resQuat );
    
	return vec3(resQuat.x, resQuat.y, resQuat.z);
}

void main(void) {
    vec4 vertexPos = vec4( rotate( vertex.xyz, instance_rotation)*instance_position.w , 1);
    gl_Position = depthMVP*vec4((vertexPos.xyz+instance_position.xyz),1);
}
