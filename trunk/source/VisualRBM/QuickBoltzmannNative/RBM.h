#pragma once

#include <stdint.h>

class RBM
{
public:
	RBM(uint16_t in_visible_count, uint16_t in_hidden_count, bool in_visible_units_linear);
	~RBM();

	bool Save(const char* filename);
	static RBM* Load(const char* filename);

	uint16_t _visible_count;
	uint16_t _hidden_count;

	bool _visible_units_linear;

	float* _visible_means;
	float* _visible_stddev;

	float* _visible_biases;
	float* _hidden_biases;
	float* _weights;


};