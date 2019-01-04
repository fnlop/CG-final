#version 400

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 Texture;

uniform mat4 modelview;
uniform mat4 proj;
uniform vec3 lightnow;
uniform mat4 Nmatrix;

out vec2 texturetofrag;
out vec3 L;
out vec3 R;
out vec3 N;
out vec3 toeye;
out float dist;
void main() {
	vec4 mdposition = modelview*vec4(Position,1.0);
	gl_Position = proj * modelview * vec4(Position, 1.0);
	texturetofrag = Texture;
	L = normalize(vec3(mdposition) -lightnow);			// vector of incoming light 
	N = normalize(vec3(Nmatrix* vec4(Normal, 1.0)));
	R = reflect(L,N);
	dist = distance(vec3(mdposition),lightnow);
	toeye = normalize(vec3(-mdposition));				// the vector from position to eye position
}