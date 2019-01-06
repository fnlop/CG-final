#version 400

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 Texture;
layout(location = 3) in vec3 tPosition;

uniform mat4 modelview;
uniform mat4 proj;
uniform vec3 lightnow;
uniform mat4 Nmatrix;
uniform float expandValue;
uniform float showPercent;
uniform float meshEnlargeSize;
uniform float seed;
uniform float scanlineValue;
uniform float min_y;
uniform float max_y;

out vec2 texturetofrag;
out vec3 L;
out vec3 R;
out vec3 N;
out vec3 toeye;
out float dist;
flat out vec3 center;
out float normalize_y;

#define PI (3.14159)
const float expand = 2;

float rand(vec2);
float rand(vec3);
float rand(vec3, float);
vec3 randVec3(vec3);
mat4 rotationMatrix(vec3, float);

void main() {
	vec4 mdposition;
	normalize_y = (tPosition.y - max_y) / (min_y - max_y);

	if (normalize_y >= scanlineValue) {							// not explode yet -> normal phong shading
		mdposition = modelview * vec4(Position, 1.0);
		gl_Position = proj * modelview * vec4(Position, 1.0);
	}
	else {											// explode effect
		// random rotation
		vec3 rotatePosition = vec3(rotationMatrix(randVec3(tPosition), rand(tPosition) * 2 * PI * expandValue) * vec4(Position - tPosition, 1.0)) + tPosition;
		// push to outside and enlarge mesh with some random in scale
		float upVector = 3*(scanlineValue-normalize_y) ; //the vector used to make point move upward
		vec3 expandPosition =normalize(Position) +  vec3(0,upVector,0) + (rotatePosition - tPosition) * meshEnlargeSize * (0.5 + rand(tPosition)) ;
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