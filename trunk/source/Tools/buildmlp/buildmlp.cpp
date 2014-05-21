#include <Common.h>
#include <MultilayerPerceptron.h>
#include <RestrictedBoltzmannMachine.h>
#include <AutoEncoder.h>
using namespace OMLT;

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <vector>
#include <string>
using namespace std;

const char* Usage =
	"Construct a MLP from a stack of RBMs, AutoEncoders, and MLPs\n"
	"\n"
	"Usage: buildmlp [OPTIONS] files... -o [OUTPUT]\n"
	"\n"
	"Flags:\n"
	"  -f              Add subsequent models normally (default)\n"
	"  -t              Transpose subsequent models before adding\n"
	"  -n INDEX COUNT  Adds COUNT layers starting at layer INDEX (MLP only)\n"				
	"  -o              Subsequent filename will be destination for final MLP\n";


void copy(FeatureMap& dest, const FeatureMap& src)
{
	assert(dest.input_length == src.input_length);
	assert(dest.feature_count == src.feature_count);

	const auto input_length = dest.input_length;
	const auto feature_count = dest.feature_count;

	memcpy(dest.biases(), src.biases(), sizeof(float) * feature_count);
	for(uint32_t k = 0; k < feature_count; k++)
	{
		memcpy(dest.feature(k), src.feature(k), sizeof(float) * input_length);
	}
}

enum Flag
{
	Invalid = -1, Forward, Transpose, Output, Layers, Count
};

Flag parse_flag(const char* in_flag)
{
	static const char* flags[] = {"-f", "-t", "-o", "-n"};
	for(uint32_t k = 0; k < ArraySize(flags); k++)
	{
		if(strcmp(in_flag, flags[k]) == 0)
		{
			return (Flag)k;
		}
	}
	return Invalid;
}

struct Layer
{
	string filename;
	bool transposed;
	Model model;
	uint32_t idx;
	uint32_t count;
};

enum State
{
	ERROR = -1,
	START = 0,
	FILENAME = 0,
	F,
	T,
	N,
	IDX,
	COUNT,
	O,
	OUT_FILENAME,
};

int main(int argc, char** argv)
{
	if(argc < 2)
	{
		printf(Usage);
		return -1;
	}

	FILE* model_dest = nullptr;

	// parse command line options
	std::vector<Layer> layers;
	{
		bool transposed = false;
		uint32_t idx = 0;
		uint32_t count = 0;

		State current_state = START;
		State next_state;
		Flag flag;
		for(int32_t k = 1; k < argc; k++)
		{
			flag = parse_flag(argv[k]);
			
			// figure out the next state
			switch(current_state)
			{
			case FILENAME:
				switch(flag)
				{
				case Forward:
					next_state = F;
					break;
				case Transpose:
					next_state = T;
					break;
				case Layers:
					next_state = N;
					break;
				case Output:
					next_state = O;
					break;
				case Invalid:
					next_state = FILENAME;
					break;
				default:
					next_state = ERROR;
					break;
				}
				break;
			case F:
				switch(flag)
				{
				case Layers:
					next_state = N;
					break;
				case Invalid:
					next_state = FILENAME;
					break;
				default:
					next_state = ERROR;
					break;
				}
				break;
			case T:
				switch(flag)
				{
				case Invalid:
					next_state = FILENAME;
					break;
				default:
					next_state = ERROR;
				}
				break;
			case N:
				next_state = IDX;
				break;
			case IDX:
				next_state = COUNT;
				break;
			case COUNT:
				next_state = FILENAME;
				break;
			case O:
				next_state = OUT_FILENAME;
				break;
			case OUT_FILENAME:
				next_state = ERROR;
				break;
			}

			switch(next_state)
			{
			case ERROR:
				printf("Error parsing command line arguments\n");
				return -1;
				break;
			case FILENAME:
				{
					Layer lay;
					lay.filename = argv[k];
					lay.transposed = transposed;
					lay.idx = idx;
					lay.count = count;
					layers.push_back(lay);
					idx = count = 0;
				}
				break;
			case F:
				transposed = false;
				break;
			case T:
				transposed = true;
				break;
			case IDX:
				if(sscanf(argv[k], "%u", &idx) != 1)
				{
					printf("Could not parse \"%s\" as index\n", argv[k]);
					return -1;
				}
				break;
			case COUNT:
				if(sscanf(argv[k], "%u", &count) != 1)
				{
					printf("Could not parse \"%s\" as count\n", argv[k]);
					return -1;
				}
				else if(count == 0)
				{
					printf("Count must be a positive integer\n");
					return -1;
				}
				break;
			case OUT_FILENAME:
				model_dest = fopen(argv[k], "wb");
				if(model_dest == nullptr)
				{
					printf("Could not open \"%s\" for writing\n");
					return -1;
				}
				break;

			}
			current_state = next_state;
		}
	}

	// load up models
	for(auto it = layers.begin(); it < layers.end(); ++it)
	{
		string json;
		if(ReadTextFile(it->filename, json) == false)
		{
			printf("Could not load model \"%s\"\n", it->filename);
			return -1;
		}

		if(Model::FromJSON(json, it->model) == false)
		{
			printf("Could not parse model \"%s\"\n", it->filename);
			return -1;
		}
	}

	// now construct our MLP
	std::vector<MLP::Layer*> mlp_layers;
	for(auto it = layers.begin(); it < layers.end(); ++it)
	{
		const auto& model = it->model;

		switch(model.type)
		{
		case ModelType::RBM:
			{
				RBM* rbm = model.rbm;
				MLP::Layer* layer = nullptr;
				if(it->transposed)
				{
					layer = new MLP::Layer(rbm->hidden_count, rbm->visible_count, rbm->visible_type);
					copy(layer->weights, rbm->visible);
				}
				else
				{
					layer = new MLP::Layer(rbm->visible_count, rbm->hidden_count, rbm->hidden_type);
					copy(layer->weights, rbm->hidden);
				}
				mlp_layers.push_back(layer);
				
			}
			break;
		case ModelType::AE:
			{
				AE* ae = model.ae;
				MLP::Layer* layer = nullptr;
				if(it->transposed)
				{
					layer = new MLP::Layer(ae->hidden_count, ae->visible_count, ae->output_type);
					copy(layer->weights, ae->decoder);
				}
				else
				{
					layer = new MLP::Layer(ae->visible_count, ae->hidden_count, ae->hidden_type);
					copy(layer->weights, ae->encoder);
				}
				mlp_layers.push_back(layer);
			}
			break;
		case ModelType::MLP:
			{
				if(it->transposed)
				{
					printf("MLP cannot be transposed\n");
					return -1;
				}

				MLP* mlp = model.mlp;

				uint32_t start, end;
				if(it->count > 0)
				{
					start = it->idx;
					end = start + it->count;
				}
				else
				{
					start  = 0;
					end = mlp->LayerCount();
				}
				
				for(uint32_t k = start; k < end; k++)
				{
					MLP::Layer* old_layer = mlp->GetLayer(k);
					MLP::Layer* new_layer = new MLP::Layer(old_layer->inputs, old_layer->outputs, old_layer->function);
					copy(new_layer->weights, old_layer->weights);
					mlp_layers.push_back(new_layer);
				}
			}
			break;
		}
	}
	
	// ensure the dimensions all line up

	for(size_t k = 1; k < mlp_layers.size(); k++)
	{
		if(mlp_layers[k-1]->outputs != mlp_layers[k]->inputs)
		{
			printf("Inconsistent layer dimensions detected\n");
			return -1;
		}
	}

	// build the mlp
	MLP* result = new MLP();
	for(auto it = mlp_layers.begin(); it < mlp_layers.end(); ++it)
	{
		result->AddLayer(*it);
	}

	// convert to json
	string mlp_json = result->ToJSON();

	// print to file
	fwrite(mlp_json.c_str(), sizeof(uint8_t), mlp_json.size(), model_dest);

	return 0;
}