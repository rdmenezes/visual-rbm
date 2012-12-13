#pragma once

#include <stdint.h>
#include <assert.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>

class RBM
{
public:
	RBM(uint16_t in_visible_count, uint16_t in_hidden_count, bool in_visible_units_linear)
	{
		assert(in_visible_count > 0);
		assert(in_hidden_count > 0);

		_visible_count = in_visible_count;
		_hidden_count = in_hidden_count;
		_visible_units_linear = in_visible_units_linear;

		if(_visible_units_linear)
		{
			_visible_means = new float[_visible_count];
			_visible_stddev = new float[_visible_count];
		}

		_visible_biases = new float[_visible_count];
		_hidden_biases = new float[_hidden_count];
		_weights = new float[_visible_count * _hidden_count];
	}
	~RBM()
	{
		if(_visible_units_linear)
		{
			delete[] _visible_means;
			delete[] _visible_stddev;
		}

		delete[] _visible_biases;
		delete[] _hidden_biases;
		delete[] _weights;
	}
	
	inline float GetWeight(uint32_t i, uint32_t j) const
	{
		return _weights[i * _hidden_count + j];
	}

	/** RBM Binary Fomrat:
	 * Header:
	 *  4 bytes: ".RBM"
	 *  1 byte: 0xFF or 0x00, if 0xFF than uses linear visible (and will have means/stddev), 0x00 uses sigmoid
	 *  2 bytes: uint16 Visible Units
	 *  2 bytes: uint16 Hidden Units
	 * Data:
	 * Visible Data Means (if visible type is linear)
	 *  V * 4 bytes: single precision floating point
	 * Visible Data StdDev (if visible type is linear)
	 *  V * 4 bytes: single precision floating point
	 * Visible Biases
	 *  V * 4 bytes: single precision floating point
	 * Hidden Biases:
	 *  H * 4 bytes: single precision floating point
	 * Weights (stored in row major order, V rows containing H weights each)
	 *  V * H * 4 bytes: single precision floating point
	 */
	
	bool Save(const char* filename)
	{
		// verify we have good data
		assert(_visible_count != 0);
		assert(_hidden_count != 0);
		if(_visible_units_linear == true)
		{
			assert(_visible_means != 0);
			assert(_visible_stddev != 0);
		}
		assert(_visible_biases != 0);
		assert(_hidden_biases != 0);
		assert(_weights != 0);

		/** Save the RBM **/
		FILE* f = fopen(filename, "wb");
		
		if(f == NULL)
			return false;

		// write header
		write(f, ".RBM", 4);	
		// write byte for whether or not we are including data stats
		fputc(_visible_units_linear == true ? 0xFF : 0x00, f);
		write(f, &_visible_count);
		write(f, &_hidden_count);
		// stats
		if(_visible_units_linear == true)
		{
			write(f, _visible_means, _visible_count);
			write(f, _visible_stddev, _visible_count);
		}
		// biases
		write(f, _visible_biases, _visible_count);
		write(f, _hidden_biases, _hidden_count);
		// wights
		write(f, _weights, _hidden_count * _visible_count);	

		fclose(f);
		return true;
	}
	static RBM* Load(const char* filename)
	{
		RBM* result = NULL;
		FILE* f = fopen(filename, "rb");

		if(f == NULL)
			return NULL;

		
		char dest[4];
		fread(dest, 1, 4, f);
		if(memcmp(dest, ".RBM", 4) != 0)
		{
			goto End;
		}

		bool visible_units_linear;
		switch(fgetc(f))
		{
		case 0xFF:
			visible_units_linear = true;
			break;
		case 0x00:
			visible_units_linear = false;
			break;
		default:
			goto End;
		}
		uint16_t visible_count, hidden_count;
		read(f, &visible_count);
		read(f, &hidden_count);

		result = new RBM(visible_count, hidden_count, visible_units_linear);
		// stats
		if(visible_units_linear)
		{
			read(f, result->_visible_means, visible_count);
			read(f, result->_visible_stddev, visible_count);
		}
		// biases
		read(f, result->_visible_biases, visible_count);
		read(f, result->_hidden_biases, hidden_count);
		// weights
		read(f, result->_weights, hidden_count * visible_count);

	End:
		fclose(f);
		return result;		
	}

	uint16_t _visible_count;
	uint16_t _hidden_count;

	bool _visible_units_linear;

	float* _visible_means;
	float* _visible_stddev;

	float* _visible_biases;
	float* _hidden_biases;
	float* _weights;

private:
	template<typename T>
	static void write(FILE* File, const T* Value, uint32_t Count=1)
	{
		fwrite(Value, sizeof(T), Count, File);
	}

	template<typename T>
	static void read(FILE* File, T* Value, uint32_t Count = 1)
	{
		fread(Value, sizeof(T), Count, File);
	}

};