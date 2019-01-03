#version 400

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 Texture;

uniform mat4 modelview;
uniform mat4 proj;
out vec2 texturetofrag;

void main() {
	gl_Position = proj * modelview * vec4(Position, 1.0);
	texturetofrag = Texture;
}