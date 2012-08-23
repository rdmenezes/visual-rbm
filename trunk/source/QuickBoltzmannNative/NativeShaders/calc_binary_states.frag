#version 420 
#extension GL_ARB_texture_rectangle : enable
#extension GL_EXT_gpu_shader4 : enable
uniform usampler2DRect random; 
uniform sampler2DRect probabilities; 
 
in vec2 tex_coordinate; 
 
out float state; 
 
void main() 
{ 
	uint next_int = texture2DRect(random, tex_coordinate).x; 
	float prob = texture2DRect(probabilities, tex_coordinate).x; 
	if(prob > float(next_int) / 4294967296.0) 
		state = 1.0; 
	else 
		state = 0.0;
}