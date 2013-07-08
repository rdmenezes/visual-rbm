// windows
#include <intrin.h>
#include <malloc.h>
#include <windows.h>

// std
#include <iostream>

// OMLT
#include <DataAtlas.h>
#include <IDX.hpp>
#include <BackPropagation.h>
#include <MovingAverage.h>
using namespace OMLT;

// sandbox
#include <EasyBMP.h>

namespace OMLT
{
	extern __m128 _mm_sigmoid_ps(__m128 x0);
}

static uint32_t seed = 1;
uint32_t random()
{
	const uint32_t a = 1664525;
	const uint32_t c = 1013904223;

	seed = a * seed + c;

	//printf("seed: %u\n", seed);

	return seed;
}


uint32_t uniform(uint32_t (*in_random)(void))
{
	return in_random();
}

float next_float(uint32_t (*in_random)(void))
{
	uint32_t next24 = in_random() >> 8;
	return (float)next24 / (float)(1 << 24);
}

float normal(uint32_t (*in_random)(void))
{
	float u1 = 0.0f;
	float u2 = 0.0f;

	while(u1 == 0.0f)
	{
		u1 = next_float(in_random);
	}
	u2 = next_float(in_random);

	const float PI = 3.14159265359f;
	return std::sqrtf(-2.0f * std::logf(u1)) * std::sinf(2.0f * PI * u2);
}

inline float sign(const float x)
{
	union
	{
		float f;
		uint32_t u;
	};
	f = x;
	uint32_t abs = u & 0x80000000;
	f = 1.0f;
	u = abs | u;
	return f;
}

void test_sigmoid()
{
	const uint32_t block_count = 50000;
	float* buffer = (float*)_aligned_malloc(sizeof(float) * 4 * block_count, 16);

	float* simd_result = (float*)_aligned_malloc(sizeof(float) * 4 * block_count, 16);
	float* normal_result = (float*)_aligned_malloc(sizeof(float) * 4 * block_count, 16);
	float* slow_result = (float*)_aligned_malloc(sizeof(float) * 4 * block_count, 16);
	uint64_t simd_start;
	uint64_t simd_end;
	uint64_t normal_end;
	uint64_t slow_end;
	uint64_t memcpy_end;

	

	for(uint32_t k = 0; k < block_count * 4; k++)
	{
		buffer[k] = normal(random) * 2.0f;
	}

	float* buffer_head = buffer;
	float* simd_head = simd_result;

	QueryPerformanceCounter((LARGE_INTEGER*)&simd_start);

	for(uint32_t k = 0; k < block_count; k++)
	{
		__m128 in = _mm_load_ps(buffer_head);
		__m128 sigm = _mm_sigmoid_ps(in);
		_mm_store_ps(simd_head, sigm);
		buffer_head += 4;
		simd_head += 4;
	}

	QueryPerformanceCounter((LARGE_INTEGER*)&simd_end);

	for(uint32_t k = 0; k < block_count * 4; k++)
	{
		float& x = buffer[k];
		float diff = (std::abs(x) - 4.0f);

		float s = sign(x);

		normal_result[k] = -1.0f/32.0f * diff * diff * s + (s + 1.0f) / 2.0f;
	}

	QueryPerformanceCounter((LARGE_INTEGER*)&normal_end);

	for(uint32_t k = 0; k < block_count * 4; k++)
	{
		slow_result[k] = 1.0f / (1.0f + std::expf(-buffer[k]));
	}

	QueryPerformanceCounter((LARGE_INTEGER*)&slow_end);

	memcpy(normal_result, simd_result, 4 * block_count * sizeof(float));

	QueryPerformanceCounter((LARGE_INTEGER*)&memcpy_end);

	
	std::cout << "SIMD sigmoid: " << (simd_end - simd_start) << std::endl;
	std::cout << "Fast sigmoid: " << (normal_end - simd_end) << std::endl;
	std::cout << "Slow Sigmoid: " << (slow_end - normal_end) << std::endl;
	std::cout << "Memcpy: " << (memcpy_end - slow_end) << std::endl;
	getc(stdin);
	return;
	/*
	printf("input, simd, slow\n");
	for(uint32_t k = 0; k < block_count * 4; k++)
	{
		printf("%f,%f,%f\n", buffer[k], simd_result[k], slow_result[k]);
	}

	//getc(stdin);
	*/
}

void main(int argc, char** argv)
{
	//test_sigmoid();
	//return;

	SiCKL::OpenGLRuntime::Initialize();

	IDX* data = IDX::Load(argv[1]);

	DataAtlas atlas(data);
	atlas.Build(10, 512);

	SiCKL::OpenGLBuffer2D tex_buffer;

	TrainerConfig t_config = {10, 0.05f, 0.5f};
	BackPropagation trainer(data->GetRowLength(), t_config);

	LayerConfig hlayer_config = {100, NoisySigmoid, 0.0f};
	trainer.AddLayer(hlayer_config);

	LayerConfig olayer_config = {data->GetRowLength(), Sigmoid, 0.0f};
	trainer.AddLayer(olayer_config);

	trainer.Initialize();

	MovingAverage* average = MovingAverage::Build(100);

	//{
	//	MultilayerPerceptron* mlp = trainer.GetMultilayerPerceptron();
	//	std::string json = mlp->ToJSON();
	//	printf("%s\n", json.c_str());
	//}

	for(uint32_t e = 0; e < 100; e++)
	{
		for(uint32_t b = 0; b < atlas.GetTotalBatches(); b++)
		{
			atlas.Next(tex_buffer);
			average->AddEntry(trainer.Train(tex_buffer, tex_buffer));
			
			if((b + 1) % 100 == 0)
			{
				printf("Error %i:%i = %f\n", e, b+1, average->GetAverage());
			}
		}		
	}

	{
		MultilayerPerceptron* mlp = trainer.GetMultilayerPerceptron();
		std::string json = mlp->ToJSON();
		printf("%s\n", json.c_str());
	}

	//getc(stdin);
}

void ToBMP(float* in_buffer, int id)
{
	BMP image;
	image.SetSize(28 * 10, 28);

	for(uint32_t k = 0; k < 10; k++)
	{
		for(uint32_t y = 0; y < 28; y++)
		{
			for(uint32_t x = 0; x < 28; x++)
			{
				float& flum = *in_buffer++;
				uint8_t lum = (uint8_t)(flum * 255.0f);

				RGBApixel pixel;
				pixel.Red = pixel.Green =  pixel.Blue = lum;

				image.SetPixel(k * 28 + x, y, pixel);
			}
		}
	}

	char filename[32] = {0};
	sprintf(filename, "batch%04i.bmp", id);
	image.WriteToFile(filename);
}
