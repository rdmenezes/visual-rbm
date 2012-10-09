#version 330

uniform sampler2DRect enabled_rows;
uniform sampler2DRect enabled_columns;

uniform sampler2DRect prev_vals;

uniform int check_rows;
uniform int check_columns;
uniform int use_prev_vals;

uniform vec2 offset;

in vec2 tex_coordinate;

out float value;

void main()
{
	// this pixel should be disabled
	if ((check_rows == 1 && texture(enabled_rows, vec2(tex_coordinate.y - offset.y, 0.5)).x == 0.0)
	||  (check_columns == 1 && texture(enabled_columns, vec2(tex_coordinate.x - offset.x, 0.5)).x == 0.0))
	{
		if(use_prev_vals == 1)
		{
			value = texture(prev_vals, tex_coordinate).x;
		}
		else
		{
			value = 0.0;
		}
	}
	// this pixel should be enabled
	else
	{
		discard;

	}
}