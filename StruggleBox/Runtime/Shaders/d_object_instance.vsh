#version 400

layout (location = 0) in vec4 v_vertex;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec3 v_tangent;
layout (location = 3) in vec2 v_uv;
layout (location = 4) in vec2 v_materialOffset;
layout (location = 5) in vec3 instance_position;
layout (location = 6) in vec4 instance_rotation;
layout (location = 7) in vec3 instance_scale;

uniform mat4 MVP;

out Fragment {
    smooth vec2 uv;
    smooth float depth;
    vec2 material;
    smooth vec3 fragPos;
    mat3 tbn;
} fragment;

// Quaternion rotation
// vec4 multQ( vec4 oq, vec4 rq ) {
// 	// the constructor takes its arguments as (x, y, z, w)
// 	return vec4(oq.w * rq.x + oq.x * rq.w + oq.y * rq.z - oq.z * rq.y,
//                 oq.w * rq.y + oq.y * rq.w + oq.z * rq.x - oq.x * rq.z,
// 	            oq.w * rq.z + oq.z * rq.w + oq.x * rq.y - oq.y * rq.x,
// 	            oq.w * rq.w - oq.x * rq.x - oq.y * rq.y - oq.z * rq.z);
// }
// vec4 getConjugate( vec4 q ) { return vec4(-q.x, -q.y, -q.z, q.w); }
// // Multiplying a quaternion q with a vector v applies the q-rotation to v
// vec3 rotate(vec3 vert, vec4 rot) {
//     float len = length(vert);
//     // Normalizing a vector of (0,0,0) will fail
//     if ( len == 0.0 ) return vec3(0,0,0);
//
//     vec3 vn = vert/len; // Manual normalization to make use of length calculation
//
// 	vec4 vecQuat, resQuat;
// 	vecQuat.x = vn.x;
// 	vecQuat.y = vn.y;
// 	vecQuat.z = vn.z;
// 	vecQuat.w = 0.0f;
//
// 	resQuat = multQ( vecQuat, getConjugate(rot) );
// 	resQuat = multQ( rot, resQuat );
//     // We have to multiply by the length of original vert to maintain scale
// 	return vec3(resQuat.x, resQuat.y, resQuat.z)*len;
// }

mat3 mat3FromQuaternion(vec4 q)
{
  mat3 Result;
  float qxx = (q.x * q.x);
  float qyy = (q.y * q.y);
  float qzz = (q.z * q.z);
  float qxz = (q.x * q.z);
  float qxy = (q.x * q.y);
  float qyz = (q.y * q.z);
  float qwx = (q.w * q.x);
  float qwy = (q.w * q.y);
  float qwz = (q.w * q.z);

  Result[0][0] = 1 - 2 * (qyy +  qzz);
  Result[0][1] = 2 * (qxy + qwz);
  Result[0][2] = 2 * (qxz - qwy);

  Result[1][0] = 2 * (qxy - qwz);
  Result[1][1] = 1 - 2 * (qxx +  qzz);
  Result[1][2] = 2 * (qyz + qwx);

  Result[2][0] = 2 * (qxz + qwy);
  Result[2][1] = 2 * (qyz - qwx);
  Result[2][2] = 1 - 2 * (qxx +  qyy);
  return Result;
}

void main(void)
{
    mat3 modelMatrix = mat3FromQuaternion(instance_rotation);
    vec3 vertex = modelMatrix * (v_vertex.xyz*instance_scale);
    vertex += instance_position;

    gl_Position = MVP*(vec4(vertex.xyz, 1.0));

    // v_vertex.w contains vertex occlusion intensity
    // fragment.albedo = vec4(albedo.rgb*(1.0-v_vertex.w), albedo.a);

    vec3 bitangent = cross(v_normal, v_tangent);
    mat3 TBN = mat3(normalize(vec3(modelMatrix * v_tangent)),
                    normalize(vec3(modelMatrix * bitangent)),
                    normalize(vec3(modelMatrix * v_normal)));

    fragment.uv = v_uv;
    fragment.depth = gl_Position.z;
    fragment.material = v_materialOffset;
    fragment.tbn = TBN;
    fragment.fragPos = vertex;
}
