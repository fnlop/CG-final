#version 400

layout(location = 0) in vec3 Position;

uniform mat4 modelview;
uniform mat4 proj;

flat out vec3 PositionToFrag;

void main() {
	vec4 mdposition = modelview*vec4(Position,1.0);
	PositionToFrag = Position;
	gl_Position = proj * modelview * vec4(Position, 1.0);
}