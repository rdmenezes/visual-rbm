#version 330

// textures
uniform sampler2DRect visible;
uniform sampler2DRect hidden;

uniform sampler2DRect visible_prime;
uniform sampler2DRect hidden_prime;

uniform sampler2DRect prev_weight_deltas;
uniform sampler2DRect prev_weights;

// training parameters
uniform float momentum;

uniform int minibatch_size;

in vec2 tex_coordinate;

out float delta;


void main()
{
	delta = 0.0;

	uint i = uint(tex_coordinate.y);
	uint j = uint(tex_coordinate.x);

	float vi, vi_prime;
	float hj, hj_prime;

	if(i == 0u && j == 0u)
	{
		return;
	}
	// hidden bias
	else if(i == 0u)
	{
		for(int k = 0; k  < minibatch_size; k++)
		{
			hj = texture(hidden, vec2(tex_coordinate.x - 1.0, float(k) + 0.5)).x;
			hj_prime = texture(hidden_prime, vec2(tex_coordinate.x - 1.0, float(k) + 0.5)).x;

			delta += hj - hj_prime;
		}
		delta /=  float(minibatch_size);
	}
	// visible bias
	else if(j == 0u)
	{
		for(int k = 0; k  < minibatch_size; k++)
		{
			vi = texture(visible, vec2(tex_coordinate.y - 1.0, float(k) + 0.5)).x;
			vi_prime = texture(visible_prime, vec2(tex_coordinate.y - 1.0, float(k) + 0.5)).x;

			delta += vi - vi_prime;
		}
		delta /= float(minibatch_size);
	}
	// regular weight
	else
	{
		for(int k = 0; k  < minibatch_size; k++)
		{
			vi = texture(visible, vec2(tex_coordinate.y - 1.0, float(k) + 0.5)).x;
			vi_prime = texture(visible_prime, vec2(tex_coordinate.y - 1.0, float(k) + 0.5)).x;
			
			hj = texture(hidden, vec2(tex_coordinate.x - 1.0, float(k) + 0.5)).x;
			hj_prime = texture(hidden_prime, vec2(tex_coordinate.x - 1.0, float(k) + 0.5)).x;

			delta += vi * hj - vi_prime * hj_prime;
		}
		delta /= float(minibatch_size);
	}

	delta = (1.0 - momentum) * delta + momentum * texture(prev_weight_deltas, tex_coordinate).x;
}