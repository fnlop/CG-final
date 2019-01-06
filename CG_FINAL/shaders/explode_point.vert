#version 400

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 Texture;
layout(location = 3) in vec3 tPosition;

uniform mat4 modelview;
uniform mat4 proj;
uniform vec3 lightnow;
uniform mat4 Nmatrix;
uniform float showPercent;
uniform float meshEnlargeSize;
uniform float seed;
uniform vec3 mRaycastPoint;
uniform float spreadValue;
uniform float longestDis;

out vec2 texturetofrag;
out vec3 L;
out vec3 R;
out vec3 N;
out vec3 toeye;
out float dist;
flat out vec3 center;
flat out float value;

#define PI (3.14159)
const float expand = 2;

float rand(vec2);
float rand(vec3);
float rand(vec3, float);
vec3 randVec3(vec3);
mat4 rotationMatrix(vec3, float);

void main() {
	vec4 mdposition;
	if (spreadValue == 0)
		value = -100;
	else
		value = spreadValue - distance(mRaycastPoint, tPosition) / longestDis;
	
	if (value < 0) {							// not explode yet -> normal phong shading
		mdposition = modelview * vec4(Position, 1.0);
		gl_Position = proj * modelview * vec4(Position, 1.0);
	}
	else {											// explode effect
		// random rotation
		vec3 rotatePosition = vec3(rotationMatrix(randVec3(tPosition), rand(tPosition) * 2 * PI * value) * vec4(Position - tPosition, 1.0)) + tPosition;
		// push to outside and enlarge mesh with some random in scale
		vec3 expandPosition = tPosition * (1 + (expand - 1) * value) + (rotatePosition - tPosition) * meshEnlargeSize * (0.5 + rand(tPosition));
		mdposition = modelview * vec4(expandPosition, 1.0);
		gl_Position = proj * modelview * vec4(expandPosition, 1.0);
	}

	texturetofrag = Texture;
	L = normalize(vec3(mdposition) -lightnow);			// vector of incoming light 
	N = normalize(vec3(Nmatrix* vec4(Normal, 1.0)));
	R = reflect(L,N);
	dist = distance(vec3(mdposition),lightnow);
	toeye = normalize(vec3(-mdposition));				// the vector from position to eye position
	center = tPosition;
}

// magical random function
float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
float rand(vec3 co){
  return rand( vec2(rand(co.xy), co.z) );
}
float rand(vec3 co, float seed) {
	return rand( vec2(rand(co), seed) );
}
vec3 randVec3(vec3 co) {
	return vec3( rand(co.xyz), rand(co.zxy), rand(co.yzx) ) - 0.5;
}
// rotate by arbitrary axis
mat4 rotationMatrix(vec3 axis, float angle) {
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}