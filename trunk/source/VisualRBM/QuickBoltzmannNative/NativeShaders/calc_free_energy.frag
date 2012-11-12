#version 330

uniform sampler2DRect visible;
uniform sampler2DRect rbm;
uniform sampler2DRect hidden_accumulation;

uniform int visible_units;
uniform int hidden_units;

smooth in vec2 tex_coordinate;

out float free_energy;

void main()
{
	free_energy = 0.0;
	// calc the first term
	for(int i = 0; i < visible_units; i++)
	{
		float vi = texture(visible, vec2(float(i) + 0.5, tex_coordinate.x)).x;
		float ai = texture(rbm, vec2(0.5, float(i) + 1.5)).x;
		free_energy -= vi * ai;
	}

	// calc the second term
	for(int j = 0; j < hidden_units; j++)
	{
		float xj = texture(hidden_accumulation, vec2(float(j) + 0.5, tex_coordinate.x)).x;
		float exp_xj = min(exp(xj), 1E38);
		free_energy -= log(1.0 + exp_xj);
	}
}