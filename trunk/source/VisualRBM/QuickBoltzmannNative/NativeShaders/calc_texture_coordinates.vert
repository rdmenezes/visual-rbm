#version 330
// calculate the texture coordinates 
 
layout (location = 0) in vec2 position; 

// viewport size
uniform vec2 size;
uniform float depth;

smooth out vec2 tex_coordinate; 
 
void main(void) 
{ 
	gl_Position.xy = 2.0 * (position / size) - vec2(1.0, 1.0);

	gl_Position.z = depth;
	gl_Position.w =  1.0;

	tex_coordinate = position; 
}