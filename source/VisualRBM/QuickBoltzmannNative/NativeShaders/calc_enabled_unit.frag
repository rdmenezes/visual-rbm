#version 330
uniform usampler2DRect random; 
uniform float probability;
 
smooth in vec2 tex_coordinate; 
 
out float state; 
 
void main() 
{ 
	uint next_int = texture(random, tex_coordinate).x; 
	if(uint(probability * 4294967296.0) > next_int) 
		state = 1.0; 
	else 
		state = 0.0;
}