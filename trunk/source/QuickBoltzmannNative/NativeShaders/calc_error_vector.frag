#version 330 
 
uniform sampler2DRect visible; 
uniform sampler2DRect visible_reconstruction; 
 
uniform int minibatch_size; 
 
in vec2 tex_coordinate; 
 
out float mean_square_error; 
 
void main() 
{ 
	mean_square_error = 0.0; 
 
	for(int k = 0; k < minibatch_size; k++) 
	{ 
		float v = texture(visible, vec2(tex_coordinate.x, float(k) + 0.5)).x; 
		float v_prime = texture(visible_reconstruction, vec2(tex_coordinate.x, float(k) + 0.5)).x; 
 
		float diff = v - v_prime;  
		mean_square_error += diff*diff; 
	} 
	mean_square_error /= float(minibatch_size); 
}