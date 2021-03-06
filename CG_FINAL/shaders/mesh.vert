#version 400

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 cPosition;

uniform mat4 modelview;
uniform mat4 proj;

flat out vec3 center;

void main() {
	center = cPosition;
	gl_Position = proj * modelview * vec4(Position, 1.0);
}