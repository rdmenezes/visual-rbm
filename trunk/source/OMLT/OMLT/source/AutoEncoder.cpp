#include <cJSON.h>
#include "AutoEncoder.h"


namespace OMLT
{
	AutoEncoder::AutoEncoder( uint32_t in_visible_count, uint32_t in_hidden_count, ActivationFunction_t in_hidden_type, ActivationFunction_t in_output_type )
		: visible_count(in_visible_count),
		  hidden_count(in_hidden_count),
		  hidden_type(in_hidden_type),
		  output_type(in_output_type)
	{

	}

	AutoEncoder::~AutoEncoder()
	{

	}

	void AutoEncoder::Encode( const float* in_raw, float* out_encoded ) const
	{

	}

	void AutoEncoder::Decode( const float* in_decoded, float* out_raw ) const
	{

	}

	AutoEncoder* AutoEncoder::FromJSON( struct cJSON* root )
	{
		return nullptr;
	}
}