#version 330
// calculate the texture coordinates 
 
layout (location = 0) in vec2 position; 


uniform uint width;
uniform uint height; 
smooth out vec2 tex_coordinate; 
 
void main(void) 
{ 
	gl_Position.x = position.x * (2.0 / float(width)) - 1.0;
	gl_Position.y = position.y * (2.0 / float(height)) - 1.0;
	gl_Position.z = 0.0;
	gl_Position.w =  1.0;

	tex_coordinate = position; 
}