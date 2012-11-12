#version 330
// input textures 
uniform sampler2DRect visible_states; 
uniform sampler2DRect rbm_weights; 

uniform int visible_units; 
 
smooth in vec2 tex_coordinate; 
 

out float probability; 
 
// matrix coordinates 
//x == j 
//y == i 

void main() 
{ 
	// bias 
	probability = texture(rbm_weights, vec2(tex_coordinate.x + 1.0, 0.5)).x; 
	for(int i = 0; i <  visible_units; i++) 
	{ 
		float v_i = texture(visible_states, vec2(float(i) + 0.5, tex_coordinate.y)).x; 
		float w_ij = texture(rbm_weights, vec2(tex_coordinate.x + 1.0, float(i) + 1.5)).x; 
		probability += v_i * w_ij; 
	} 
	 
	probability = 1.0 / (1.0 + exp(-probability));  
}