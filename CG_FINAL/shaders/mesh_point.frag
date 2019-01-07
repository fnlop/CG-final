#version 400

uniform float showMeshValue;
uniform float showPercentMesh;
uniform float seed;

flat in vec3 center;
flat in float value;
out vec4 outColor;

float rand(vec2);
float rand(vec3);
float rand(vec3, float);

const vec3 glowColor = vec3(120, 120, 255) / 255.0;
const vec4 lineColor = vec4(0, 0, 0, 1);
// fragment will fading out when startFadePercent < value < totalFadePercent
const float startFadePercent = -0.1;
const float totalFadePercent = 0;

void main() {
	if (value > totalFadePercent) {
		discard;
	}

	float v = rand(center, seed);
	if(v > showPercentMesh){
		discard;
	}
	
	outColor = lineColor;
	outColor.a *= min( showMeshValue, 1 - (clamp(value, startFadePercent, 1) - startFadePercent) / (totalFadePercent - startFadePercent));
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