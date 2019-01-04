#version 400

layout(location = 3) in vec3 Position;

uniform mat4 modelview;
uniform mat4 proj;

void main() {
	vec4 mdposition = modelview*vec4(Position,1.0);
	gl_Position = proj * modelview * vec4(Position, 1.0);
}