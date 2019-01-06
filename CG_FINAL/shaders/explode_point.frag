#version 400

uniform vec4 Ka;
uniform vec4 Kd;
uniform vec4 Ks;
uniform float alpha;
uniform sampler2D tex;
uniform float showMeshValue;
uniform float showPercent;
uniform float seed;
uniform float spreadValue;
uniform vec3 mRaycastPoint;

in vec2 texturetofrag;
in vec3 L;
in vec3 N;
in vec3 R;
in float dist;
in vec3 toeye;
flat in vec3 center;
flat in float value;

out vec4 outColor;

const vec4 La = vec4(0.2, 0.2, 0.2, 1.0);
const vec4 Ld = vec4(0.8, 0.8, 0.8, 1.0);
const vec4 Ls = vec4(0.5, 0.5, 0.5, 1.0);
const vec4 glowColor = vec4(120, 120, 255, 125) / 255.0;
// fragment will fading out when startFadePercent < value < totalFadePercent
const float startFadePercent = 0.1;
const float totalFadePercent = 0.3;
// fragment will change color from origin color to glow color when -changeColorPercent < value < 0
const float changeColorPercent = 0.1;
// fragment will have a little glow color when showing mesh line, with the max interpolation ratio = maxShowMeshColorPercent
const float maxShowMeshColorPercent = 0.2;

float rand(vec2);
float rand(vec3);
float rand(vec3, float);

void main() {
	if (value >= 0) {											
		// discard partial fragment
		float v = rand(center.zyx, seed);
		if(v > showPercent){
			discard;
		}
	}
	if (value >= totalFadePercent) {
		discard;
	}

	// normal phong shading
	vec4 ambient = Ka * La * texture2D(tex, texturetofrag);
	vec4 diffuse = texture2D(tex, texturetofrag) * Kd * Ld * max(0, dot(-L,N)) / sqrt(dist);
	vec4 specular = Ks * Ls * pow(max(0,dot(toeye,R)), alpha) / sqrt(dist);
	vec4 phongColor = 3 * ambient + diffuse + specular;
	// broken fragments color
	vec4 fragColor = glowColor;
	fragColor.a *= 1 - (clamp(value, startFadePercent, 1) - startFadePercent) / (totalFadePercent - startFadePercent);
	if (gl_FrontFacing) {
		// interpolation by showMeshValue and value
		outColor = phongColor + clamp((value + changeColorPercent) / changeColorPercent, showMeshValue * maxShowMeshColorPercent, 1) * (fragColor - phongColor);
	}
	else {
		// directy use fragColor
		outColor = fragColor;
	}
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