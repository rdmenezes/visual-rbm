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

	//float* head = nullptr;
	//float* buffer = nullptr;

	TrainerConfig t_config = {10, 0.1f, 0.5f};
	BackPropagation trainer(data->GetRowLength(), t_config);

	LayerConfig hlayer_config = {500, Sigmoid, 0.2f};
	trainer.AddLayer(hlayer_config);

	LayerConfig olayer_config = {data->GetRowLength(), Sigmoid, 0.5f};
	trainer.AddOutputLayer(olayer_config);

	trainer.Initialize();

	for(uint32_t b = 0; b < 10000; b++)
	{
		atlas.Next(tex_buffer);

		//printf("Error: %f\n", trainer.Train(tex_buffer, tex_buffer));
		trainer.Train(tex_buffer, tex_buffer);

		//BMP image;
		//image.SetSize(28 * 10, 28);
		//
		//tex_buffer.GetData(buffer);
		//head = buffer;

		//for(uint32_t k = 0; k < 10; k++)
		//{
		//	for(uint32_t y = 0; y < 28; y++)
		//	{
		//		for(uint32_t x = 0; x < 28; x++)
		//		{
		//			float& flum = *buffer++;
		//			uint8_t lum = (uint8_t)(flum * 255.0f);

		//			RGBApixel pixel;
		//			pixel.Red = pixel.Green =  pixel.Blue = lum;

		//			image.SetPixel(k * 28 + x, y, pixel);
		//		}
		//	}
		//}

		//image.WriteToFile("what.bmp");
		//buffer = head;
		//getc(stdin);
		
	}

	getc(stdin);
}