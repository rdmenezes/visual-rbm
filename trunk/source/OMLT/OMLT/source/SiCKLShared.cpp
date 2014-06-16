#include <SiCKL.h>
using namespace SiCKL;

#include "Enums.h"
#include "SiCKLShared.h"

namespace OMLT
{

	/// SiCKL Kernel Methods

	 void NextSeed(const SiCKL::UInt4& in_seed, SiCKL::UInt4& out_seed)
	{
		UInt t = in_seed.X ^ (in_seed.X << 11u);
		out_seed.X = in_seed.Y;
		out_seed.Y = in_seed.Z;
		out_seed.Z = in_seed.W;
		out_seed.W = in_seed.W ^ (in_seed.W >> 19u) ^ t ^ (t >> 8u);
	}
	 void NextFloat(const SiCKL::UInt4& in_seed, SiCKL::UInt4& out_seed, SiCKL::Float& out_float)
	{
		NextSeed(in_seed, out_seed);

		// top 24 bits 
		UInt next24 = out_seed.X >> 8u;

		out_float = (Float)next24 / (float)(1 << 24);
	}
	 void NextGaussian(const SiCKL::UInt4& in_seed, SiCKL::UInt4& out_seed, SiCKL::Float& out_gaussian)
	{
		Float u1 = 0.0f;
		Float u2 = 0.0f;

		UInt4 seed0 = in_seed;
		UInt4 seed1(0,0,0,0);

		// get our random values
		NextFloat(seed0, seed1, u1);
		u1 = Max(u1, 0.00000005960464478f);
		seed0 = seed1;		
		NextFloat(seed0, seed1, u2);
		out_seed = seed1;

		// calculate a normally distributed variable
		const float PI = 3.14159265359f;
		out_gaussian = Sqrt(-2.0f * Log(u1)) * Sin(2.0f * PI * u2);
	}

	Float Sigmoid( const Float& in_x )
	{
		return 1.0f / (1.0f + Exp(-in_x));
	}

	Float RectifiedLinear(const Float& in_x)
	{
		return Max(in_x, 0.0f);
	}
	
	Float CalcActivation(ActivationFunction_t in_func, const Float& in_accumulation)
	{
		COMPUTE_ASSERT(in_func == ActivationFunction::Linear ||
		               in_func == ActivationFunction::Sigmoid ||
					   in_func == ActivationFunction::RectifiedLinear ||
					   in_func == ActivationFunction::Softmax);

		switch(in_func)
		{
		case ActivationFunction::Softmax:
		case ActivationFunction::Linear:
			return in_accumulation;
		case ActivationFunction::Sigmoid:
			return Sigmoid(in_accumulation);
		case ActivationFunction::RectifiedLinear:
			return RectifiedLinear(in_accumulation);
		}
		
		return Float(0.0f);
	}

	Float CalcActivationPrime(ActivationFunction_t in_func, const Float& in_activation)
	{
		COMPUTE_ASSERT(in_func == ActivationFunction::Linear ||
			in_func == ActivationFunction::Sigmoid ||
			in_func == ActivationFunction::RectifiedLinear ||
			in_func == ActivationFunction::Softmax);

		switch(in_func)
		{
		case ActivationFunction::Linear:
			return Float(1.0f);
		case ActivationFunction::Sigmoid:
		case ActivationFunction::Softmax:
			return (1.0f - in_activation) * in_activation;
		case ActivationFunction::RectifiedLinear:
			return Max(0.0f, Sign(in_activation));
		}

		return Float(0.0f);
	}

	ErrorCalculator::ErrorCalculator(uint32_t minibatch_size, uint32_t data_width, ErrorFunction_t error_function)
	{
		struct SourceCalcErrorVector : public SiCKL::Source
		{
			int32_t DATA_WIDTH;
			ErrorFunction_t ERROR_FUNC;

			BEGIN_SOURCE
				BEGIN_CONST_DATA
					CONST_DATA(Buffer2D<Float>, in_calculated)
					CONST_DATA(Buffer2D<Float>, in_labels)
				END_CONST_DATA

				BEGIN_OUT_DATA
					OUT_DATA(Float, out_error)
				END_OUT_DATA

				BEGIN_MAIN
					Int batch = Index().X;
					out_error = 0.0f;
					ForInRange(k, 0, DATA_WIDTH)
						const Float z = in_calculated(k, batch);
						const Float t = in_labels(k, batch);

						if(ERROR_FUNC == ErrorFunction::SquareError)
						{
							const Float diff = z - t;
							out_error = out_error + (diff * diff);
						}
						else if(ERROR_FUNC == ErrorFunction::CrossEntropy)
						{
							out_error = out_error - t * Log(Max(z, 1.1754943508e-38f));
						}
						else
						{
							assert(false);
						}
					EndFor

					// average together
					out_error = out_error * (1.0f / DATA_WIDTH);

				END_MAIN
			END_SOURCE
		} source;

		source.DATA_WIDTH = data_width;
		source.ERROR_FUNC = error_function;
		source.Parse();
		
		OpenGLCompiler compiler;

		_calc_error = compiler.Build(source);
		_calc_error->Initialize(minibatch_size, 1);

		_error_texture = OpenGLBuffer2D(minibatch_size, 1, ReturnType::Float, nullptr);

		_error_buffer.Acquire(minibatch_size);
	}

	ErrorCalculator::~ErrorCalculator()
	{
		delete _calc_error;
	}

	float ErrorCalculator::CalcError(const OpenGLBuffer2D& calculated, const OpenGLBuffer2D& expected)
	{
		// calc error
		_calc_error->SetInput(0, calculated);
		_calc_error->SetInput(1, expected);
		_calc_error->BindOutput(0, _error_texture);
		_calc_error->Run();

		// dump to CPU
		float* head = _error_buffer;
		_calc_error->GetOutput(0, head);

		// calculate the last bit
		__m128 sum = _mm_setzero_ps();

		for(uint32_t k =0 ; k < _error_buffer.BlockCount(); k++)
		{
			sum = _mm_add_ps(sum, _mm_load_ps(head));

			head += 4;
		}

		sum = _mm_hadd_ps(sum, sum);
		sum = _mm_hadd_ps(sum, sum);

		float result;
		_mm_store_ss(&result, sum);

		result /= _error_texture.Width;
		return result;
	}
}