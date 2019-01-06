#version 400

uniform vec4 Ka;
uniform vec4 Kd;
uniform vec4 Ks;
uniform float alpha;
uniform sampler2D tex;
uniform float showMeshValue;
uniform float expandValue;
uniform float showPercent;
uniform float fadeValue;
uniform float seed;
uniform float min_y;
uniform float max_y;
uniform float scanlineValue;

in vec2 texturetofrag;
in vec3 L;
in vec3 N;
in vec3 R;
in float dist;
in vec3 toeye;
flat in vec3 center;
in float normalize_y;
out vec4 outColor;

const vec4 La = vec4(0.2, 0.2, 0.2, 1.0);
const vec4 Ld = vec4(0.8, 0.8, 0.8, 1.0);
const vec4 Ls = vec4(0.5, 0.5, 0.5, 1.0);
const vec4 glowColor = vec4(120, 120, 255, 125) / 255.0;

float rand(vec2);
float rand(vec3);
float rand(vec3, float);

void main() {
	if (normalize_y < scanlineValue) {											
		// discard partial fragment
		float v = rand(center.zyx, seed);
		if(v > showPercent){
			discard;
		}
	}
	if (fadeValue >= 1) {
		discard;
	}

	if(scanlineValue - normalize_y > 0.3){
		discard;
	}

	// normal phong shading
	vec4 ambient = Ka * La * texture2D(tex, texturetofrag);
	vec4 diffuse = texture2D(tex, texturetofrag) * Kd * Ld * max(0, dot(-L,N)) / sqrt(dist);
	vec4 specular = Ks * Ls * pow(max(0,dot(toeye,R)), alpha) / sqrt(dist);
	vec4 phongColor = 3 * ambient + diffuse + specular;
	// broken fragments color
	vec4 fragColor = glowColor;
	fragColor.a *= 1 - fadeValue;
	// interpolation by showMeshValue
	if( abs (scanlineValue - normalize_y) <0.1 )
		outColor = phongColor + 10*abs(abs(scanlineValue - normalize_y)-0.1) * (fragColor - phongColor);
	else
		outColor = phongColor ;

	if(scanlineValue > normalize_y)
		outColor = fragColor;
	//outColor = phongColor + showMeshValue * (fragColor - phongColor);
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