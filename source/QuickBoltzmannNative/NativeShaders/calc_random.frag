#version 420 
#extension GL_EXT_gpu_shader4 : enable
// calculate next values using Numerical recipes LCRG:  
// http://en.wikipedia.org/wiki/Linear_congruential_generator  

const uint a = 1664525u;  
const uint c = 1013904223u;  

uniform usampler2DRect seeds;  
  
in vec2 tex_coordinate;  
 
out uint next_int;  
  
void main()  
{  
	next_int = texture2DRect(seeds, tex_coordinate).x * a + c;  
} 