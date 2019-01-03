#version 400


uniform float threshold;
uniform sampler2D tex;
uniform sampler2D noise;

in vec2 texturetofrag;
out vec4 outColor;
//there should be a out vec4 in fragment shader defining the output color of fragment shader(variable name can be arbitrary)
void main() {
	float v = texture2D(noise,texturetofrag).r;
	float w = 0.1;
	vec4 color = vec4(1.0, 0.0, 1.0, 1.0);
	if(v > threshold+w){
		outColor = texture2D(tex,texturetofrag);
	}
	else{
		outColor = color;
	}
	if(v < threshold){
		discard;
	}
}