#version 330
// input textures 
uniform sampler2DRect hidden_states; 
uniform sampler2DRect rbm_weights; 
 
uniform int hidden_units; 
uniform bool sigmoid;
 
in vec2 tex_coordinate; 
 
out float reconstruction; 
 
// matrix coordinates 
//x == j 
//y == i 
 
void main() 
{ 
	// bias 
	reconstruction = texture(rbm_weights, vec2(0.5, tex_coordinate.x + 1.0)).x; 
	for(int j = 0; j <  hidden_units; j++) 
	{ 
		float h_j = texture(hidden_states, vec2(float(j) + 0.5, tex_coordinate.y)).x; 
		float w_ij = texture(rbm_weights, vec2(float(j) + 1.5, tex_coordinate.x + 1.0)).x; 
		reconstruction += h_j * w_ij; 
	} 
	 
	// sigmoid 
	if(sigmoid)
		reconstruction = 1.0 / (1.0 + exp(-reconstruction));  
}