#version 430 core

layout (local_size_x = 8, local_size_y = 8) in;

layout (rgba32f, binding = 0) uniform image2D frameBuffer;
layout (r8ui, binding = 1) uniform uimage3D blockBuffer;

// The camera specification
uniform vec3 eye;
uniform vec3 ray00;
uniform vec3 ray10;
uniform vec3 ray01;
uniform vec3 ray11;

struct box {
  vec3 min;
  vec3 max;
};

#define NUM_BOXES 2
const box boxes[] = {
  /* The ground */
  {vec3(-5.0, -0.1, -5.0), vec3(5.0, 0.0, 5.0)},
  /* Box in the middle */
  {vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0)}
};

vec2 intersectBox(vec3 origin, vec3 dir, const box b) {
  vec3 tMin = (b.min - origin) / dir;
  vec3 tMax = (b.max - origin) / dir;
  vec3 t1 = min(tMin, tMax);
  vec3 t2 = max(tMin, tMax);
  float tNear = max(max(t1.x, t1.y), t1.z);
  float tFar = min(min(t2.x, t2.y), t2.z);
  return vec2(tNear, tFar);
}

#define MAX_SCENE_BOUNDS 1000.0

struct hitinfo {
  vec2 lambda;
  int bi;
};

bool intersectBoxes(vec3 origin, vec3 dir, out hitinfo info) {
  float smallest = MAX_SCENE_BOUNDS;
  bool found = false;
  for (int i = 0; i < NUM_BOXES; i++) {
    vec2 lambda = intersectBox(origin, dir, boxes[i]);
    if (lambda.x > 0.0 && lambda.x < lambda.y && lambda.x < smallest) {
      info.lambda = lambda;
      info.bi = i;
      smallest = lambda.x;
      found = true;
    }
  }
  return found;
}

uint getVoxel(ivec3 c) {
    return imageLoad(blockBuffer, c).r;
}


float vertexAo(vec2 side, float corner) {
	return (side.x + side.y + max(corner, side.x * side.y)) / 3.0;
}

vec4 voxelAo(ivec3 pos, ivec3 d1, ivec3 d2) {
	vec4 side = vec4(getVoxel(pos + d1), getVoxel(pos + d2), getVoxel(pos - d1), getVoxel(pos - d2));
	vec4 corner = vec4(getVoxel(pos + d1 + d2), getVoxel(pos - d1 + d2), getVoxel(pos - d1 - d2), getVoxel(pos + d1 - d2));
	vec4 ao;
	ao.x = vertexAo(side.xy, corner.x);
	ao.y = vertexAo(side.yz, corner.y);
	ao.z = vertexAo(side.zw, corner.z);
	ao.w = vertexAo(side.wx, corner.w);
	return 1.0 - ao;
}


vec3 stepMask(vec3 sideDist) {
    // Yoinked from https://www.shadertoy.com/view/l33XWf
    bvec3 move;
    bvec3 pon = lessThan(sideDist.xyz, sideDist.yzx);

    move.x = pon.x && !pon.z;
    move.y = pon.y && !pon.x;
    move.z = !(move.x || move.y);

    return vec3(move);
}

vec4 traceBlock(vec3 rayPos, vec3 rayDir, vec3 iMask) {
    rayPos = clamp(rayPos, vec3(0.0001), vec3(7.9999));
    vec3 mapPos = floor(rayPos);
    vec3 raySign = sign(rayDir);
    vec3 deltaDist = 1.0 / rayDir;
    vec3 sideDist = ((mapPos - rayPos) + 0.5 + raySign * 0.5) * deltaDist;
    vec3 mask = iMask;
    
    while (mapPos.x <= 7.0 && mapPos.x >= 0.0 &&
           mapPos.y <= 7.0 && mapPos.y >= 0.0 &&
           mapPos.z <= 7.0 && mapPos.z >= 0.0) {
        if (getVoxel(ivec3(mapPos)) > 0)
            return vec4(floor(mapPos) / 8.0, 1.0);
            
       mask      = stepMask(sideDist);
       mapPos   += mask * raySign;
       sideDist += mask * raySign * deltaDist;
    }
    
    return vec4(0.0);
}

float sum(vec3 v) { return dot(v, vec3(1.0)); }

vec4 trace(vec3 rayPos, vec3 rayDir) {
    hitinfo i;
    if (intersectBoxes(rayPos, rayDir, i)) {
        const box b = boxes[i.bi];
        const vec3 boxSize = b.max - b.min;
        const vec3 intersectPos = rayPos + (rayDir * i.lambda.x);
        const vec3 rayBoxPos = (intersectPos - b.min);

        vec3 mapPos = floor(rayPos);
        vec3 raySign = sign(rayDir);
        vec3 deltaDist = 1.0 / rayDir;
        vec3 sideDist = ((mapPos - intersectPos) + 0.5 + raySign * 0.5) * deltaDist;
        vec3 mask = stepMask(sideDist);

        const vec3 uv3d = rayBoxPos;
        vec4 hit = traceBlock(uv3d * 8.0, rayDir, mask);
        if (hit.a > 0.95)
        {
            return vec4(rayBoxPos, 1.0);
//            vec4 ambient = voxelAo(ivec3(mapPos - raySign * mask), ivec3(mask.zxy), ivec3(mask.yzx));
//            vec3 endRayPos = rayDir / sum(mask * rayDir) * sum(mask * (mapPos + vec3(lessThan(rayDir, vec3(0))) - rayPos)) + rayPos;
//            vec2 uv = mod(vec2(dot(mask * endRayPos.yzx, vec3(1.0)), dot(mask * endRayPos.zxy, vec3(1.0))), vec2(1.0));
//            float interpAo = mix(mix(ambient.z, ambient.w, uv.x), mix(ambient.y, ambient.x, uv.x), uv.y);
//            interpAo = pow(interpAo, 1.0 / 3.0);
//            return vec4(interpAo, interpAo, interpAo, 1.0);
        }
    }
    return vec4(0.0, 0.0, 0.0, 1.0);
}

void main(void) {
  ivec2 pix = ivec2(gl_GlobalInvocationID.xy);
  ivec2 size = imageSize(frameBuffer);
  if (pix.x >= size.x || pix.y >= size.y) {
    return;
  }
  vec2 pos = vec2(pix) / vec2(size.x, size.y);
  vec3 dir = mix(mix(ray00, ray01, pos.y), mix(ray10, ray11, pos.y), pos.x);
  vec4 color = trace(eye, dir);
  imageStore(frameBuffer, pix, color);
}