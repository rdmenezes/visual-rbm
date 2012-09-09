#version 330
// calculate next values using Numerical recipes LCRG:  
// http://en.wikipedia.org/wiki/Linear_congruential_generator  

const uint a = 1664525u;  
const uint c = 1013904223u;  

uniform usampler2DRect seeds;  
  
in vec2 tex_coordinate;  
 
out uint next_int;  
  
void main()  
{  
	next_int = texture(seeds, tex_coordinate).x * a + c;  
} 