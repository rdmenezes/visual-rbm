// cJSON
#include <cJSON.h>
// cppJSONStream
#include <cppJSONStream.hpp>
using namespace cppJSONStream;

// OMLT
#include "Model.h"
#include "RestrictedBoltzmannMachine.h"
#include "MultilayerPerceptron.h"
#include "AutoEncoder.h"

namespace OMLT
{
	bool Model::FromJSON(std::istream& in_stream, Model& out_model)
	{
		out_model.type = ModelType::Invalid;
		out_model.ptr = nullptr;

		// save our current position
		std::streampos pos = in_stream.tellg();

		cppJSONStream::Reader r(in_stream);

		SetReader(r);
		SetErrorResult(false);
		TryGetToken(Token::BeginObject);

		TryGetNameValuePair("Type", Token::String);

		std::string type = r.readString();

		// reset stream to beginning
		in_stream.seekg(pos, in_stream.beg);
		if(type == "RestrictedBoltzmannMachine")
		{
			RBM* rbm = RBM::FromJSON(in_stream);
			if(rbm != nullptr)
			{
				out_model.rbm = rbm;
				out_model.type = ModelType::RBM;
				return true;
			}
		}
		else if(type == "AutoEncoder")
		{
			AE* ae = AE::FromJSON(in_stream);
			if(ae != nullptr)
			{
				out_model.ae = ae;
				out_model.type = ModelType::AE;
				return true;
			}
		}
		else if(type == "MultilayerPerceptron")
		{
			MLP* mlp = MLP::FromJSON(in_stream);
			if(mlp != nullptr)
			{
				out_model.mlp = mlp;
				out_model.type = ModelType::MLP;
				return true;
			}
		}

		return false;
	}
}