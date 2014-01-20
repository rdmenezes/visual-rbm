#pragma once

#include <stdint.h>
#include <vector>
#include <utility>
#include <algorithm>

#include "ContrastiveDivergence.h"

namespace OMLT
{
	template<typename T>
	class TrainingSchedule
	{
	public:
		TrainingSchedule(const struct T::ModelConfig& in_model_config, uint32_t in_minibatch_size)
			: model_config(in_model_config)
			, minibatch_size(in_minibatch_size)
			, epochs_remaining(0)
			, index(0)
		{
			
		}

		void AddTrainingConfig(struct T::TrainingConfig& in_train_config, uint32_t in_epochs)
		{
			train_config.push_back(std::pair<struct T::TrainingConfig, uint32_t>(in_train_config, in_epochs));
		}

		// returns true if we are done training
		bool TrainingComplete() const
		{
			// no more training configs, and no more epochs
			return index == (train_config.size()) && epochs_remaining == 0;
		}

		// returns true if we have a new training config, false if not
		bool NextEpoch(struct T::TrainingConfig& out_config)
		{
			if(epochs_remaining == 0)
			{
				index = std::min(index + 1, trainf_config.size());
				if(index == train_config.size())
				{
					return false;
				}
				else
				{
					out_config = train_config[index].first;
					epochs_remaining = trainf_config[index].second;
					return true;
				}
			}
			epochs_remaining--;
		}

		struct T::ModelConfig GetModelConfig() const
		{
			return model_config;
		}
		
		uint32_t GetMinibatchSize() const
		{
			return minibatch_size;
		}

		static TrainingSchedule* FromJSON(const std::string& json);
	private:
		struct T::ModelConfig model_config;
		std::vector<std::pair<struct T::TrainingConfig, uint32_t>> train_config;
		uint32_t minibatch_size;

		uint32_t epochs_remaining;
		uint32_t index;
	};
	// parsing specialization
	template<>
	TrainingSchedule<ContrastiveDivergence>* TrainingSchedule<ContrastiveDivergence>::FromJSON(const std::string& json);
}