#version 400

layout (location = 0) in vec4 v_vertex;
layout (location = 1) in vec4 v_normal;
layout (location = 2) in float v_materialOffset;
layout (location = 3) in vec3 instance_position;
layout (location = 4) in vec4 instance_rotation;
layout (location = 5) in vec3 instance_scale;

uniform mat4 MVP;
uniform sampler2D materialTexture;

out Fragment {
  smooth vec4 albedo;
  smooth vec4 material;
  smooth vec3 normal;
  smooth float depth;
} fragment;

// Quaternion rotation
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
    float len = length(vert);
    // Normalizing a vector of (0,0,0) will fail
    if ( len == 0.0 ) return vec3(0,0,0);

    vec3 vn = vert/len; // Manual normalization to make use of length calculation

	vec4 vecQuat, resQuat;
	vecQuat.x = vn.x;
	vecQuat.y = vn.y;
	vecQuat.z = vn.z;
	vecQuat.w = 0.0f;

	resQuat = multQ( vecQuat, getConjugate(rot) );
	resQuat = multQ( rot, resQuat );
    // We have to multiply by the length of original vert to maintain scale
	return vec3(resQuat.x, resQuat.y, resQuat.z)*len;
}

void main(void)
 {
    vec3 vertex = v_vertex.xyz*instance_scale;
    vertex = rotate( vertex, instance_rotation);
    vertex += instance_position;

    gl_Position = MVP*(vec4(vertex.xyz,1));

    vec4 albedo = texture(materialTexture, vec2(v_materialOffset, 0.25));
    vec4 material = texture(materialTexture, vec2(v_materialOffset, 0.75));

    // v_vertex.w contains vertex occlusion intensity
    fragment.albedo = vec4(albedo.rgb*(1.0-v_vertex.w), albedo.a);
    fragment.material = material;
    fragment.normal = rotate( v_normal.xyz, instance_rotation );
    fragment.depth = gl_Position.z;
}
