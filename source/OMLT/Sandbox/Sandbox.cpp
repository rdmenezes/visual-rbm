// OMLT
#include <DataAtlas.h>
#include <IDX.hpp>
#include <BackPropagation.h>
#include <MovingAverage.h>
using namespace OMLT;

// sandbox
#include <EasyBMP.h>

void main(int argc, char** argv)
{
	SiCKL::OpenGLRuntime::Initialize();

	IDX* data = IDX::Load(argv[1]);

	DataAtlas atlas(data);
	atlas.Build(10, 512);

	SiCKL::OpenGLBuffer2D tex_buffer;

	TrainerConfig t_config = {10, 0.01f, 0.5f};
	BackPropagation trainer(data->GetRowLength(), t_config);

	LayerConfig hlayer_config = {500, NoisySigmoid, 0.1f};
	trainer.AddLayer(hlayer_config);

	LayerConfig olayer_config = {data->GetRowLength(), Sigmoid, 0.5f};
	trainer.AddOutputLayer(olayer_config);

	trainer.Initialize();

	MovingAverage* average = MovingAverage::Build(100);

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

	getc(stdin);
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
