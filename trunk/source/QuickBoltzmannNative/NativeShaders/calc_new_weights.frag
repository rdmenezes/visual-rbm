#version 420 
#extension GL_ARB_texture_rectangle : enable
 
uniform sampler2DRect prev_weights; 
uniform sampler2DRect delta_weights; 

uniform float weight_factor;
uniform float hidden_factor;
uniform float visible_factor;

in vec2 tex_coordinate; 
 
out float new_weight; 

void main() 
{ 
	uint i = uint(tex_coordinate.y);
	uint j = uint(tex_coordinate.x);

	float factor;

	if(i == 0 && j == 0)
	{
		new_weight = 0.0f;
		return;
	}
	else if(i == 0)
	{
		factor = hidden_factor;
	}
	else if(j == 0)
	{
		factor = visible_factor;
	}
	else
	{
		factor = weight_factor;
	}

	// add weight deltas 
	new_weight = texture2DRect(prev_weights, tex_coordinate).x + factor * texture2DRect(delta_weights, tex_coordinate).x;
}