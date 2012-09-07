#version 420 
#extension GL_ARB_texture_rectangle : enable
 
uniform sampler2DRect prev_weights; 
uniform sampler2DRect delta_weights; 

// conversion factors so updates are weights * learning_rate sized
uniform float weight_factor;
uniform float hidden_factor;
uniform float visible_factor;

uniform float learning_rate;

// regularization values
uniform float l1_regularization;
uniform float l2_regularization;

in vec2 tex_coordinate; 

out float new_weight; 

void main() 
{ 
	uint i = uint(tex_coordinate.y);
	uint j = uint(tex_coordinate.x);

	
	float delta = texture2DRect(delta_weights, tex_coordinate).x;

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

		float l1 = l1_regularization * sign(texture2DRect(prev_weights, tex_coordinate).x);
		float l2 = l2_regularization * texture2DRect(prev_weights, tex_coordinate).x;

		delta -= (l1 + l2);
	}

	// add weight deltas 
	new_weight = texture2DRect(prev_weights, tex_coordinate).x + (factor * learning_rate * delta);

}