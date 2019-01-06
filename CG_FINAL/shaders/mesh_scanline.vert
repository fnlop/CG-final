#version 400

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 cPosition;

uniform mat4 modelview;
uniform mat4 proj;
uniform float scanlineValue;
uniform float min_y;
uniform float max_y;

flat out vec3 center;
flat out float value;

void main() {
	center = cPosition;
	float normalize_y = (cPosition.y - max_y) / (min_y - max_y);
	value = scanlineValue - normalize_y;

	gl_Position = proj * modelview * vec4(Position, 1.0);
}