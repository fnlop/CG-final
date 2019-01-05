#version 400

uniform float showMeshValue;

flat in vec3 PositionToFrag;

out vec4 outColor;

float rand(vec2);
float rand(vec3);
float rand(vec3, float);

const vec3 glowColor = vec3(120, 120, 255) / 255.0;

void main() {

	float v = rand(PositionToFrag);
	/*
	if(rand(vec2(v,v)) > 0.487){
		discard;
	}
	*/

	if(showMeshValue == 0)
		outColor = vec4(0.0,0.0,0.0, 0.0);
	else
		outColor = vec4(glowColor,rand(vec2(v,v))/100 + showMeshValue);

}


float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
float rand(vec3 co){
  return rand( vec2(rand(co.xy), co.z) );
}
float rand(vec3 co, float seed) {
	return rand( vec2(rand(co), seed) );
}