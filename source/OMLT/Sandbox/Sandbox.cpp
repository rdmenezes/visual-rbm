// std
#include <iostream>
#include <fstream>

// windows
#include <intrin.h>
#include <malloc.h>
#include <windows.h>

// OMLT
#include <DataAtlas.h>
#include <IDX.hpp>
#include <ContrastiveDivergence.h>
#include <BackPropagation.h>
#include <MovingAverage.h>
#include <ConfusionMatrix.h>
#include <RestrictedBoltzmannMachine.h>

using namespace OMLT;

// sandbox
#include <EasyBMP.h>


float CalcSquareError(const float* buff1, const float* buff2, const uint32_t size)
{
	float result = 0.0f;
	for(uint32_t i = 0; i < size; i++)
	{
		float diff = buff1[i] - buff2[i];
		result += diff * diff;
	}
	return result / size;
}


void main(int argc, char** argv)
{
/*
	SiCKL::OpenGLRuntime::Initialize();
	TestContrastiveDivergence("C:\\Users\\pospeselr\\Projects\\VisualRBM\\data\\mnist-train-images.idx", "rbm-recon.idx");
*/
	//getc(stdin);
#if 0
	SiCKL::OpenGLRuntime::Initialize();

	IDX* data = IDX::Load(argv[1]);
	IDX* labels = IDX::Load(argv[2]);

	assert(data->GetRowCount() == labels->GetRowCount());

	const uint32_t minibatch_size = 10;

	DataAtlas data_atlas(data);
	data_atlas.Build(minibatch_size, 512);

	DataAtlas label_atlas(labels);
	label_atlas.Build(minibatch_size, 512);

	SiCKL::OpenGLBuffer2D data_buffer, label_buffer;

	TrainerConfig t_config = {0};
	{
		t_config.MinibatchSize = 10;
		t_config.LearningRate = 0.1f;
		t_config.Momentum = 0.5f;
		t_config.L1Regularization = 0.0f;
		t_config.L2Regularization = 0.0f;
	}
	BackPropagation trainer(data->GetRowLength(), t_config);


	LayerConfig hlayer1_config = {500, ActivationFunction::Sigmoid, true, 0.2f};
	trainer.AddLayer(hlayer1_config);

	//LayerConfig hlayer2_config = {500, ActivationFunction::Sigmoid, true, 0.5f};
	//trainer.AddLayer(hlayer2_config);

	//LayerConfig olayer_config = {labels->GetRowLength(), ActivationFunction::Sigmoid, false, 0.5f};
	LayerConfig olayer_config = {data->GetRowLength(), ActivationFunction::Sigmoid, false, 0.5f};

	trainer.AddLayer(olayer_config);

	trainer.Initialize();

	const uint32_t epoch_count = 20;
	const uint32_t ma_count = 50;
	MovingAverage* average = MovingAverage::Build(ma_count);

	for(uint32_t e = 0; e < epoch_count; e++)
	{
		for(uint32_t b = 0; b < data_atlas.GetTotalBatches(); b++)
		{
			data_atlas.Next(data_buffer);
			label_atlas.Next(label_buffer);
			average->AddEntry(trainer.Train(data_buffer, data_buffer));
			
			if((b + 1) % ma_count == 0)
			{
				printf("Error %i:%i = %f\n", e, b+1, average->GetAverage());
			}
		}		
	}

	MultilayerPerceptron* mlp = trainer.GetMultilayerPerceptron();
#if 0

	ConfusionMatrix cm(labels->GetRowLength());

	data->ReadRow(0, input_vector);
	labels->ReadRow(0, label_vector);

	// cpu feed forward
	mlp->FeedForward(input_vector, output_vector);
	
	data_buffer.SetData(input_vector);
	label_buffer.SetData(label_vector);

	// gpu feed forward
	trainer.Train(data_buffer, label_buffer);
	
	

	printf("");
#endif
//	float CalculateMeanSquareError(MultilayerPerceptron* mlp, IDX* inputs, IDX* labels, const char* output_idx);
//	printf("DataSet Average Square Error: %f\n", CalculateMeanSquareError(mlp, data, data, "recon-sigmoid.idx"));

	MultilayerPerceptron* encoder = trainer.GetMultilayerPerceptron(0, 0);
	float* input_buffer = new float[data->GetRowLength()];
	float* encoding_buffer = new float[500];
	
	IDX* encodings = IDX::Create("encoded.idx", LittleEndian, Single, 500);
	for(uint32_t k = 0; k < data->GetRowCount(); k++)
	{
		data->ReadRow(k, input_buffer);
		encoder->FeedForward(input_buffer, encoding_buffer);

		encodings->AddRow(encoding_buffer);
	}
	encodings->Close();


	printf("Done");
#if 0

	cm.Print();
#endif

	getc(stdin);

#endif
}

float CalculateMeanSquareError(MultilayerPerceptron* mlp, IDX* inputs, IDX* labels, const char* output_idx)
{

	const uint32_t input_count = inputs->GetRowLength();
	const uint32_t output_count = labels->GetRowLength();

	IDX* recon = IDX::Create(output_idx, LittleEndian, Single, output_count);



	assert(mlp->GetLayer(0)->inputs == input_count);
	assert(mlp->GetLayer(mlp->LayerCount() - 1)->outputs == output_count);
	assert(inputs->GetRowCount() == labels->GetRowCount());
	
	float* input_vector = new float[input_count];
	float* label_vector = new float[output_count];
	float* output_vector = new float[output_count];

	double error = 0.0f;
	for(uint32_t k = 0; k < inputs->GetRowCount(); k++)
	{
		inputs->ReadRow(k, input_vector);
		labels->ReadRow(k, label_vector);

		mlp->FeedForward(input_vector, output_vector);

		recon->AddRow(output_vector);

		// errro calculationg
		double row_error = 0.0f;
		for(uint32_t j = 0; j < output_count; j++)
		{
			double diff = (double)label_vector[j] - (double)output_vector[j];
			row_error += diff * diff;
		}
		error += row_error;
	}

	recon->Close();

	error /= output_count * inputs->GetRowCount();

	return (float)error;
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
