#version 400

uniform vec4 Ka;
uniform vec4 Kd;
uniform vec4 Ks;
uniform float alpha;
uniform sampler2D tex;
uniform sampler2D ramp;

in vec2 texturetofrag;
in vec3 L;
in vec3 N;
in vec3 R;
in float dist;
in vec3 toeye;

out vec4 outColor;
//there should be a out vec4 in fragment shader defining the output color of fragment shader(variable name can be arbitrary)

void main() {

	vec4 La = vec4(0.2, 0.2, 0.2, 1.0);
	vec4 Ld = vec4(0.8, 0.8, 0.8, 1.0);
	vec4 Ls = vec4(0.5, 0.5, 0.5, 1.0);

	vec4 ambient = Ka*La * texture2D(tex,texturetofrag);
	vec4 diffuse=texture2D(tex,texturetofrag)* Kd*Ld * max(0,dot(-L,N))/sqrt(dist);
	vec4 specular = Ks * Ls * pow(max(0,dot(toeye,R)),alpha)/sqrt(dist);
	///outColor = ambient+diffuse+specular;
	diffuse = normalize(diffuse.rgba);

	diffuse.r = min(0.999, max(diffuse.r, 0.001));
	diffuse.g = min(0.999, max(diffuse.g, 0.001));
	diffuse.b = min(0.999, max(diffuse.b, 0.001));
	//diffuse.a = min(0.999, max(diffuse.a, 0.001));
	outColor.r = texture2D(ramp, vec2(diffuse.r, 0.1)).r;
	outColor.g = texture2D(ramp, vec2(diffuse.g, 0.1)).g;
	outColor.b = texture2D(ramp, vec2(diffuse.b, 0.1)).b;
	outColor.a = texture2D(ramp, vec2(diffuse.a, 0.1)).a;


}