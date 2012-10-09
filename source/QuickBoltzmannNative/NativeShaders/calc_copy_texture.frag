#version 330

uniform sampler2DRect source;

in vec2 tex_coordinate;

out float value;

void main()
{
	value = texture(source, tex_coordinate).x;
}