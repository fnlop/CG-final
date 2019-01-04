#version 400

uniform vec4 Ka;
uniform vec4 Kd;
uniform vec4 Ks;
uniform float alpha;
uniform sampler2D tex;
uniform float showPercent;

in vec2 texturetofrag;
in vec3 L;
in vec3 N;
in vec3 R;
in float dist;
in vec3 toeye;
flat in vec3 center;

out vec4 outColor;

const vec4 glowColor = vec4(120, 120, 255, 255) / 255.0;

float rand(vec2);
float rand(vec3);
float rand(vec3, float);

void main() {
	float v = rand(center);
	if(v > showPercent){
		discard;
	}

	vec4 La = vec4(0.2, 0.2, 0.2, 1.0);
	vec4 Ld = vec4(0.8, 0.8, 0.8, 1.0);
	vec4 Ls = vec4(0.5, 0.5, 0.5, 1.0);

	vec4 ambient = Ka * La * texture2D(tex, texturetofrag);
	vec4 diffuse = texture2D(tex, texturetofrag) * Kd * Ld * max(0, dot(-L,N)) / sqrt(dist);
	vec4 specular = Ks * Ls * pow(max(0,dot(toeye,R)), alpha) / sqrt(dist);
	//outColor = ambient + diffuse + specular;
	outColor = glowColor;
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