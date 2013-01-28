#pragma once

#include <stdint.h>
#include <assert.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>

class RBM
{
public:
	RBM(uint16_t in_visible_count, uint16_t in_hidden_count)
	{
		assert(in_visible_count > 0);
		assert(in_hidden_count > 0);

		_visible_count = in_visible_count;
		_hidden_count = in_hidden_count;

		_visible_biases = new float[_visible_count];
		_hidden_biases = new float[_hidden_count];
		_weights = new float[_visible_count * _hidden_count];
	}
	~RBM()
	{
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
	 *  2 bytes: uint16 Visible Units
	 *  2 bytes: uint16 Hidden Units
	 * Data:
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
		assert(_visible_biases != 0);
		assert(_hidden_biases != 0);
		assert(_weights != 0);

		/** Save the RBM **/
		FILE* f = fopen(filename, "wb");
		
		if(f == NULL)
			return false;

		// write header
		write(f, ".RBM", 4);	
		write(f, &_visible_count);
		write(f, &_hidden_count);
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
		{
			return NULL;
		}

		
		char dest[4];
		fread(dest, 1, 4, f);
		if(memcmp(dest, ".RBM", 4) != 0)
		{
			goto End;
		}

		uint16_t visible_count, hidden_count;
		read(f, &visible_count);
		read(f, &hidden_count);

		result = new RBM(visible_count, hidden_count);
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