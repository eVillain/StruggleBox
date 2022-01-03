#version 400

out vec4 fragColor;

in Fragment {
    smooth vec2 uv;
    smooth vec2 material;
} fragment;

in vec4 sphereColor;
in vec4 spherePosition;
in float sphereRadius;

uniform sampler2D textureMap;
uniform mat4 Projection;
uniform mat4 View;
uniform mat3 NormalMatrix;
uniform vec3 CameraPosition;
uniform float time;

// ray marched fireball
// sgreen
const int _VolumeSteps = 128;
const float _StepSize = 0.02;
const float _Density = 0.2;

//const float _SphereRadius = 1.0;
const float _NoiseFreq = 2.0;
const float _NoiseAmp = 1.0;
const vec3 _NoiseAnim = vec3(0, -1, 0);

// iq's nice integer-less noise function
float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);
	f = f*f*(3.0-2.0*f);

  vec2 uv = (p.xy+vec2(37.0,17.0)*p.z) + f.xy;
	vec2 rg = texture( textureMap, vec2(uv.x + 0.5, 1.0 - uv.y + 0.5)/256.0, -100.0 ).yx;
	return mix( rg.x, rg.y, f.z )*2.0-1.0;
}

float fbm( vec3 p )
{
    float f = 0.0;
    float amp = 0.5;
    for(int i=0; i<4; i++)
    {
        //f += abs(noise(p)) * amp;
        f += noise(p) * amp;
        p *= 2.03;
        amp *= 0.5;
	}
    return f;
}

// returns signed distance to surface
float distanceFunc(vec3 p)
{
  	// distance to sphere
    float d = length(p) - sphereRadius;
  	// offset distance with noise
  	d += fbm((p*_NoiseFreq) + (_NoiseAnim*fragment.material.x)) * _NoiseAmp;
  	return d;
}

// shade a point based on distance
vec4 shade(float d)
{
    if (d >= 0.0 && d < 0.2) return (mix(vec4(3, 3, 3, 1), vec4(1, 1, 0, 1), d / 0.2));
	  if (d >= 0.2 && d < 0.4) return (mix(vec4(1, 1, 0, 1), vec4(1, 0, 0, 1), (d - 0.2) / 0.2));
	  if (d >= 0.4 && d < 0.6) return (mix(vec4(1, 0, 0, 1), vec4(0, 0, 0, 0), (d - 0.4) / 0.2));
    if (d >= 0.6 && d < 0.8) return (mix(vec4(0, 0, 0, 0), vec4(0, .5, 1, 0.2), (d - 0.6) / 0.2));
    if (d >= 0.8 && d < 1.0) return (mix(vec4(0, .5, 1, .2), vec4(0, 0, 0, 0), (d - 0.8) / 0.2));
    return vec4(0.0, 0.0, 0.0, 0.0);
}

vec2 rotate(vec2 v, float angle)
{
    return v * mat2(cos(angle),sin(angle),-sin(angle),cos(angle));
}
// procedural volume
// maps position to color
vec4 volumeFunc(vec3 p)
{
  p.xz = mix(p.xz, rotate(p.xz, p.y*2.0 + fragment.material.x), fragment.material.y);	// firestorm
	float d = distanceFunc(p);
	return shade(d);
}

// ray march volume from front to back
// returns color
vec4 rayMarch(vec3 rayOrigin, vec3 rayStep, out vec3 pos)
{
	vec4 sum = vec4(0, 0, 0, 0);
	pos = rayOrigin;
	for(int i=0; i<_VolumeSteps; i++) {
		vec4 col = volumeFunc(pos);
		col.a *= _Density;
		// pre-multiply alpha
		col.rgb *= col.a;
		sum = sum + col*(1.0 - sum.a);
		pos += rayStep;
	}
	return sum;
}

void main()
{
   vec2 p = fragment.uv*2.0-1.0;
   float zoom = 4.0;
   // camera
   vec3 ro = normalize(CameraPosition-spherePosition.xyz);
   ro.x *= -1.0;
   ro *= (zoom + sphereRadius);
   // vec3 ro = zoom*normalize(vec3(cos(roty), cos(rotx), sin(roty)));
   vec3 ww = normalize(vec3(0.0,0.0,0.0) - ro);
   vec3 uu = normalize(cross( vec3(0.0,1.0,0.0), ww ));
   vec3 vv = normalize(cross(ww,uu));
   vec3 rd = normalize( p.x*uu + p.y*vv + 1.5*ww );
   ro += rd*2.0;

   // volume render
   vec3 hitPos;
   vec4 col = rayMarch(ro, rd*_StepSize, hitPos);
   if (col.a == 0)
   {
      discard;
   }
   fragColor = col;
}
