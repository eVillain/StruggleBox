#version 330
precision highp float;

// Vertex shader for instancing of colored cubes
// "vertex" contains one of the vertices of the cube
// "instance_position" contains the cube center in xyz and size in w
// "instance_rotation" contains a rotation quaternion
// "instance_texture" contains the texture coordinate in xy and offset in zw
// NOTE: I'm just using vertex coordinates for the textures, not great but works

layout (location = 0) in vec4 vertex;
layout (location = 1) in vec4 instance_position;
layout (location = 2) in vec4 instance_rotation;
layout (location = 3) in vec4 instance_texture;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;

out Fragment {
    smooth vec2 texCoord;
    smooth float depthVal;
} fragment;

// Quaternion math
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
    vec4 vertexPos = vec4( rotate( vertex.xyz, instance_rotation )*instance_position.w , 1);
    gl_Position = MVP*vec4((vertexPos.xyz+instance_position.xyz),1);

    if ( gl_VertexID < 24 ) {
        float side = instance_texture.w;
        if ( side == 7 ) { side = 18; }
        else if ( side == 6 ) { side = 19; }
        float sideOffset = side/20.0;
        if ( gl_VertexID < 6 ) {
            fragment.texCoord = vec2((vertex.x+0.5)/20+sideOffset, (vertex.y+0.5) );
        } else if ( gl_VertexID < 12 ) {
            fragment.texCoord = vec2((vertex.z+0.5)/20+sideOffset, (vertex.y+0.5) );
        } else if ( gl_VertexID < 18 ) {
            fragment.texCoord = vec2((vertex.x+0.5)/20+sideOffset, (vertex.y+0.5) );
        } else {
            fragment.texCoord = vec2((vertex.z+0.5)/20+sideOffset, (vertex.y+0.5) );
        }
    } else if ( gl_VertexID < 30 ) {
        float bottom = instance_texture.w;
        if ( bottom == 7 || bottom == 6 ) { bottom = 5; }
        float bottomOffset = (bottom/20.0);
        fragment.texCoord = vec2((vertex.x+0.5)/20+bottomOffset, (vertex.z+0.5) );
    } else {
        float topOffset = instance_texture.w/20.0;
        fragment.texCoord = vec2((vertex.x+0.5)/20+topOffset, (vertex.z+0.5) );
    }
    fragment.depthVal = gl_Position.z;
}
