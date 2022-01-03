#version 400

// Vertex shader for instancing of colored PBR cubes
// "v_vertex" contains current cube vertex
// "v_normal" contains the normal of the current side of the cube
// "v_tangent" contains the tangent vector of the current side of the cube
// "v_uv" contains the texture coordinates for the current vertex
// "instance_position" contains the cube center in xyz and size in w
// "instance_rotation" contains the cube rotation as a quaternion
// "instance_color" contains the cube color as RGBA
// "instance_material" contains roughness and metalness
layout (location = 0) in vec4 v_vertex;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec3 v_tangent;
layout (location = 3) in vec4 instance_position;
layout (location = 4) in vec4 instance_rotation;
layout (location = 5) in vec4 instance_color;
layout (location = 6) in vec3 instance_material;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

out Fragment {
    vec4 color;
    vec3 material;
    vec3 normal;
    smooth float depth;
    smooth vec3 fragPos;
} fragment;

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

    // Read size of instance from instance_position.w
    float instanceSize = instance_position.w * 2.0;
    vec3 vertex = modelMatrix * (v_vertex.xyz*instanceSize);
    vertex += instance_position.xyz;

    gl_Position = projectionMatrix*viewMatrix*(vec4(vertex.xyz, 1.0));

    // vec3 bitangent = cross(v_normal, v_tangent);
    // mat3 TBN = mat3(normalize(vec3(modelMatrix * v_tangent)),
    //                 normalize(vec3(modelMatrix * bitangent)),
    //                 normalize(vec3(modelMatrix * v_normal)));

    fragment.color = instance_color;
    fragment.material = instance_material;
    fragment.normal = normalize(modelMatrix * v_normal);
    fragment.fragPos = vertex;
    fragment.depth = gl_Position.z;
}
