#version 400

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 cPosition;

uniform mat4 modelview;
uniform mat4 proj;
uniform vec3 mRaycastPoint;
uniform float spreadValue;
uniform float longestDis;

flat out vec3 center;
flat out float value;

void main() {
	center = cPosition;
	if (spreadValue == 0)
		value = -100;
	else
		value = spreadValue - distance(mRaycastPoint, cPosition) / longestDis;
	
	gl_Position = proj * modelview * vec4(Position * 1.02, 1.0);
}