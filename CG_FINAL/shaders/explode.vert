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

out vec2 texturetofrag;
out vec3 L;
out vec3 R;
out vec3 N;
out vec3 toeye;
out float dist;
flat out vec3 center;

const float expand = 2;

float rand(vec2);
float rand(vec3);
float rand(vec3, float);

void main() {
	center = tPosition;
	vec3 expandPosition = tPosition * (1 + (expand - 1) * expandValue) + ((Position - tPosition) * meshEnlargeSize);
	vec4 mdposition = modelview * vec4(expandPosition, 1.0);
	gl_Position = proj * modelview * vec4(expandPosition, 1.0);
	texturetofrag = Texture;
	L = normalize(vec3(mdposition) -lightnow);			// vector of incoming light 
	N = normalize(vec3(Nmatrix* vec4(Normal, 1.0)));
	R = reflect(L,N);
	dist = distance(vec3(mdposition),lightnow);
	toeye = normalize(vec3(-mdposition));				// the vector from position to eye position
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
