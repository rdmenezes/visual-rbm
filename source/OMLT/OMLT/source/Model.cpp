// cJSON
#include <cJSON.h>

// OMLT
#include "Model.h"
#include "RestrictedBoltzmannMachine.h"
#include "MultilayerPerceptron.h"
#include "AutoEncoder.h"

namespace OMLT
{
	bool Model::FromJSON(const std::string& in_json, Model& out_model)
	{
		bool result = false;
		out_model.type = ModelType::Invalid;
		out_model.ptr = nullptr;

		cJSON* cj_root = cJSON_Parse(in_json.c_str());
		if(cj_root)
		{
			if(cJSON* cj_type = cJSON_GetObjectItem(cj_root, "Type"))
			{
				if(strcmp(cj_type->valuestring, "RestrictedBoltzmannMachine") == 0)
				{
					if(RBM* rbm = RBM::FromJSON(cj_root))
					{
						out_model.type = ModelType::RBM;
						out_model.rbm = rbm;
						result = true;
					}
				}
				else if(strcmp(cj_type->valuestring, "MultilayerPerceptron") == 0)
				{
					if(MLP* mlp = MLP::FromJSON(cj_root))
					{
						out_model.type = ModelType::MLP;
						out_model.mlp = mlp;
						result = true;
					}
				}
				else if(strcmp(cj_type->valuestring, "AutoEncoder") == 0)
				{
					if(AE* ae = AE::FromJSON(cj_root))
					{
						out_model.type = ModelType::AE;
						out_model.ae = ae;
						result = true;
					}
				}
			}
		}

		cJSON_Delete(cj_root);
		return result;
	}
}