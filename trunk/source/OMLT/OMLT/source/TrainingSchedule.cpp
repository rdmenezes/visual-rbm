#include "TrainingSchedule.h"

#include <cJSON.h>

namespace OMLT
{
	template<>
	TrainingSchedule<ContrastiveDivergence>* TrainingSchedule<ContrastiveDivergence>::FromJSON(const std::string& json)
	{
		TrainingSchedule<ContrastiveDivergence>* result = nullptr;

		cJSON* root = cJSON_Parse(json.c_str());
		if(root)
		{
			cJSON* cj_type = cJSON_GetObjectItem(root, "Type");

			if(!cj_type || (strcmp(cj_type->valuestring, "RBM") != 0 && strcmp(cj_type->valuestring, "RestrictedBoltzmannMachine") != 0))
			{
				goto Error;
			}

			cJSON* cj_visible_type = cJSON_GetObjectItem(root, "VisibleType");
			cJSON* cj_hidden_type = cJSON_GetObjectItem(root, "HiddenType");
			cJSON* cj_hidden_count = cJSON_GetObjectItem(root, "HiddenCount");
			cJSON* cj_minibatch_size = cJSON_GetObjectItem(root, "MinibatchSize");
			cJSON* cj_schedule = cJSON_GetObjectItem(root, "Schedule");

			if(cj_visible_type && cj_hidden_type &&
				cj_hidden_count && cj_minibatch_size &&
				cj_schedule)
			{
				ContrastiveDivergence::ModelConfig model_config = {0};
				model_config.VisibleType = (ActivationFunction_t)-1;
				model_config.HiddenType = (ActivationFunction_t)-1;
				uint32_t minibatch_size = 0;

				for(int func = 0; func < ActivationFunction::Count; func++)
				{
					if(strcmp(cj_visible_type->valuestring, ActivationFunctionNames[func]) == 0)
					{
						model_config.VisibleType = (ActivationFunction_t)func;
					}

					if(strcmp(cj_hidden_type->valuestring, ActivationFunctionNames[func]) == 0)
					{
						model_config.HiddenType = (ActivationFunction_t)func;
					}
				}

				// make sure we found both activation functions
				if(model_config.VisibleType == -1 || model_config.HiddenType == -1)
				{
					goto Error;
				}

				if(cj_hidden_count->valueint > 0)
				{
					model_config.HiddenUnits = cj_hidden_count->valueint;
				}
				else
				{
					goto Error;
				}

				if(cj_minibatch_size->valueint > 0)
				{
					minibatch_size = cj_minibatch_size->valueint;
				}
				else
				{
					goto Error;
				}

				// now step through schedule array 
				std::vector<std::pair<ContrastiveDivergence::TrainingConfig, uint32_t>> schedule;
				CD::TrainingConfig train_config;
				uint32_t epochs;
				const int schedule_length = cJSON_GetArraySize(cj_schedule);
				if(schedule_length == 0)
				{
					goto Error;
				}
				for(int k = 0; k < schedule_length; k++)
				{
					cJSON* cj_train_config = cJSON_GetArrayItem(cj_schedule, k);

					cJSON* cj_epochs = cJSON_GetObjectItem(cj_train_config, "Epochs");
					cJSON* cj_learning_rate = cJSON_GetObjectItem(cj_train_config, "LearningRate");
					cJSON* cj_momentum = cJSON_GetObjectItem(cj_train_config, "Momentum");
					cJSON* cj_l1 = cJSON_GetObjectItem(cj_train_config, "L1Regularization");
					cJSON* cj_l2 = cJSON_GetObjectItem(cj_train_config, "L2Regularization");
					cJSON* cj_visible_dropout = cJSON_GetObjectItem(cj_train_config, "VisibleDropout");
					cJSON* cj_hidden_dropout = cJSON_GetObjectItem(cj_train_config, "HiddenDropout");

					// epochs is the only thing required
					if(cj_epochs && cj_epochs->valueint > 0)
					{
						epochs = cj_epochs->valueint;
					}
					else
					{
						goto Error;
					}

					// the rest are optional

					if(cj_learning_rate)
					{
						if(cj_learning_rate->valuedouble >= 0.0)
						{
							train_config.LearningRate = (float)cj_learning_rate->valuedouble;
						}
						else
						{
							goto Error;
						}
					}
					if(cj_momentum)
					{
						if(cj_momentum->valuedouble >= 0.0 && cj_momentum->valuedouble < 1.0)
						{
							train_config.Momentum = (float)cj_momentum->valuedouble;
						}
						else
						{
							goto Error;
						}
						
					}
					if(cj_l1)
					{
						if(cj_l1->valuedouble >= 0.0)
						{
							train_config.L1Regularization = (float)cj_l1->valuedouble;
						}
						else
						{
							goto Error;
						}
					}
					if(cj_l2)
					{
						if(cj_l2->valuedouble >= 0.0)
						{
							train_config.L2Regularization = (float)cj_l2->valuedouble;
						}
						else
						{
							goto Error;
						}
					}
					if(cj_visible_dropout)
					{
						if(cj_visible_dropout->valuedouble >= 0.0 && cj_visible_dropout->valuedouble < 1.0)
						{
							train_config.VisibleDropout = (float)cj_visible_dropout->valuedouble;
						}
						else
						{
							goto Error;
						}
					}
					if(cj_hidden_dropout)
					{
						if(cj_hidden_dropout->valuedouble >= 0.0 && cj_hidden_dropout->valuedouble < 1.0)
						{
							train_config.HiddenDropout = (float)cj_hidden_dropout->valuedouble;
						}
						else
						{
							goto Error;
						}
					}

					// save off this schedule and epoch count
					schedule.push_back(std::pair<CD::TrainingConfig, uint32_t>(train_config, epochs));
				}

				// finally construct our training schedule
				result = new TrainingSchedule<ContrastiveDivergence>(model_config, minibatch_size);
				for(uint32_t k = 0; k < schedule.size(); k++)
				{
					result->AddTrainingConfig(schedule[k].first, schedule[k].second);
				}
			}
		}
Error:
		cJSON_Delete(root);
		return result;
	}

	template<>
	TrainingSchedule<BackPropagation>* TrainingSchedule<BackPropagation>::FromJSON(const std::string& json)
	{
		return nullptr;
	}
}