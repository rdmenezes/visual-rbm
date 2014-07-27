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
			cJSON* cj_visible_count = cJSON_GetObjectItem(root, "VisibleCount");
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

				if(cj_visible_count->valueint > 0)
				{
					model_config.VisibleUnits = cj_visible_count->valueint;
				}
				else
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
					cJSON* cj_adadelta_decay = cJSON_GetObjectItem(cj_train_config, "AdadeltaDecay");

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
					if(cj_adadelta_decay)
					{
						if(cj_adadelta_decay->valuedouble >= 0.0 && cj_adadelta_decay->valuedouble < 1.0)
						{
							train_config.AdadeltaDecay = (float)cj_hidden_dropout->valuedouble;
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
		TrainingSchedule<BackPropagation>* result = nullptr;
		BackPropagation::ModelConfig model_config;
		uint32_t minibatch_size;

		std::vector<std::pair<BackPropagation::TrainingConfig, uint32_t>> schedule;

		cJSON* root = cJSON_Parse(json.c_str());
		if(root)
		{
			cJSON* cj_type = cJSON_GetObjectItem(root, "Type");
			if(!cj_type || (strcmp(cj_type->valuestring, "MLP") != 0 && strcmp(cj_type->valuestring, "MultilayerPerceptron") != 0))
			{
				goto Error;
			}

			cJSON* cj_layers = cJSON_GetObjectItem(root, "Layers");
			cJSON* cj_activation_functions = cJSON_GetObjectItem(root, "ActivationFunctions");
			cJSON* cj_minibatch_size = cJSON_GetObjectItem(root, "MinibatchSize");
			cJSON* cj_schedule = cJSON_GetObjectItem(root, "Schedule");

			// make sure we have the mandatory bits
			if(cj_layers && cj_activation_functions &&
			   cj_minibatch_size && cj_schedule)
			{
				// verify we hae the right number of layer values and activation function values
				if(cJSON_GetArraySize(cj_layers) > 1 && cJSON_GetArraySize(cj_layers) == (cJSON_GetArraySize(cj_activation_functions) + 1))
				{
					// get our array iterators and populate the model config
					cJSON* cj_layer_val;
					cJSON* cj_func_val;

					cj_layer_val = cJSON_GetArrayItem(cj_layers, 0);
					model_config.InputCount = cj_layer_val->valueint;

					// parse network structure/functions
					for(int k = 0; k < cJSON_GetArraySize(cj_activation_functions); k++)
					{
						cj_layer_val = cJSON_GetArrayItem(cj_layers, k + 1);
						cj_func_val = cJSON_GetArrayItem(cj_activation_functions, k);

						if(cj_layer_val && cj_func_val)
						{
							BackPropagation::LayerConfig layer_config;
							layer_config.OutputUnits = cj_layer_val->valueint;
							layer_config.Function = ParseFunction(cj_func_val->valuestring);

							if(cj_layer_val->valueint < 1)
							{
								goto Error;
							}
							if(layer_config.Function == ActivationFunction::Invalid)
							{
								goto Error;
							}

							model_config.LayerConfigs.push_back(layer_config);
						}
						else
						{
							goto Error;
						}
					}

					// minibatch size
					if(cj_minibatch_size->valueint < 1)
					{
						goto Error;
					}
					minibatch_size = cj_minibatch_size->valueint;

					// parse our schedules
					if(cJSON_GetArraySize(cj_schedule) == 0)
					{
						goto Error;
					}

					BackPropagation::TrainingConfig train_config;
					uint32_t layer_count = model_config.LayerConfigs.size();
					for(uint32_t  k = 0; k < layer_count; k++)
					{
						BackPropagation::LayerParameters layer_params;;
						train_config.Parameters.push_back(layer_params);
					}

					// now read each traing config in the schedule
					for(int k = 0; k < cJSON_GetArraySize(cj_schedule); k++)
					{
						cJSON* cj_train_config = cJSON_GetArrayItem(cj_schedule, k);

						// this function will read an object and if it's an array, populate each member of the array to the appropriate layer config
						// if it is a single value, all the values in the array are given that value
						auto read_params = [&] (uint32_t index, const char* param, float min, float max) -> bool
						{
							cJSON* cj_param = cJSON_GetObjectItem(cj_train_config, param);
							if(cj_param)
							{
								if(cj_param->type == cJSON_Array)
								{
									// ensure we have the right number of params
									if(cJSON_GetArraySize(cj_param) != layer_count)
									{
										return false;
									}

									for(uint32_t j = 0; j < layer_count; j++)
									{
										cJSON* cj_val = cJSON_GetArrayItem(cj_param, j);
										if(cj_val->type != cJSON_Number)
										{
											return false;
										}

										float val = float(cj_val->valuedouble);

										if(val < min || val > max)
										{
											return false;
										}

										train_config.Parameters[j].Data[index] = val; 
										
									}
								}
								else if(cj_param->type == cJSON_Number)
								{
									for(uint32_t j = 0; j < layer_count; j++)
									{
										float val = float(cj_param->valuedouble);
										
										if(val < min || val > max)
										{
											return false;
										}

										train_config.Parameters[j].Data[index] = val;
									}
								}
							}
							return true;
						};
#						define GET_INDEX(NAME) (((uint32_t)( &( (BackPropagation::LayerParameters*)(void*)0)->NAME) )/sizeof(float))
#						define READ_PARAMS(NAME, MIN, MAX) if(read_params(GET_INDEX(NAME), #NAME, MIN, MAX) == false) goto Error;

						READ_PARAMS(LearningRate, 0.0f, FLT_MAX)
						READ_PARAMS(Momentum, 0.0f, 1.0f)
						READ_PARAMS(L1Regularization, 0.0f, FLT_MAX)
						READ_PARAMS(L2Regularization, 0.0f, FLT_MAX)
						READ_PARAMS(Dropout, 0.0f, 1.0f)
						READ_PARAMS(Noise, 0.0f, FLT_MAX)
						READ_PARAMS(AdadeltaDecay, 0.0f, 1.0f)

						//now get the number of epochs
						cJSON* cj_epochs = cJSON_GetObjectItem(cj_train_config, "Epochs");
						if(cj_epochs == nullptr || cj_epochs->type != cJSON_Number || cj_epochs->valueint < 1)
						{
							goto Error;
						}
						uint32_t epochs = cj_epochs->valueint;
						schedule.push_back(std::pair<BackPropagation::TrainingConfig, uint32_t>(train_config, epochs));
					}
				}
				else
				{
					goto Error;
				}
			}
			else
			{
				goto Error;
			}
		}
		else
		{
			goto Error;
		}

		// create result here
		result = new TrainingSchedule<BackPropagation>(model_config, minibatch_size);
		for(uint32_t k = 0; k < schedule.size(); k++)
		{
			result->AddTrainingConfig(schedule[k].first, schedule[k].second);
		}
Error:
		cJSON_Delete(root);
		return result;
	}

	template<>
	TrainingSchedule<AutoEncoderBackPropagation>* TrainingSchedule<AutoEncoderBackPropagation>::FromJSON(const std::string& json)
	{
		TrainingSchedule<AutoEncoderBackPropagation>* result = nullptr;

		cJSON* root = cJSON_Parse(json.c_str());
		if(root)
		{
			cJSON* cj_type = cJSON_GetObjectItem(root, "Type");

			if(!cj_type || strcmp(cj_type->valuestring, "AutoEncoder") != 0)
			{
				goto Error;
			}

			cJSON* cj_visible_count = cJSON_GetObjectItem(root, "VisibleCount");
			cJSON* cj_hidden_count = cJSON_GetObjectItem(root, "HiddenCount");
			cJSON* cj_hidden_type = cJSON_GetObjectItem(root, "HiddenType");
			cJSON* cj_output_type = cJSON_GetObjectItem(root, "OutputType");
			cJSON* cj_minibatch_size = cJSON_GetObjectItem(root, "MinibatchSize");
			cJSON* cj_schedule = cJSON_GetObjectItem(root, "Schedule");

			if(cj_output_type && cj_hidden_type &&
				cj_hidden_count && cj_minibatch_size &&
				cj_schedule)
			{
				AutoEncoderBackPropagation::ModelConfig model_config = {0};
				model_config.HiddenType = (ActivationFunction_t)-1;
				model_config.OutputType = (ActivationFunction_t)-1;
				uint32_t minibatch_size = 0;

				for(int func = 0; func < ActivationFunction::Count; func++)
				{
					if(strcmp(cj_hidden_type->valuestring, ActivationFunctionNames[func]) == 0)
					{
						model_config.HiddenType = (ActivationFunction_t)func;
					}

					if(strcmp(cj_output_type->valuestring, ActivationFunctionNames[func]) == 0)
					{
						model_config.OutputType = (ActivationFunction_t)func;
					}
				}

				// make sure we found both activation functions
				if(model_config.HiddenType == -1 || model_config.OutputType == -1)
				{
					goto Error;
				}

				if(cj_visible_count->valueint > 0)
				{
					model_config.VisibleCount = cj_visible_count->valueint;
				}
				else
				{
					goto Error;
				}

				if(cj_hidden_count->valueint > 0)
				{
					model_config.HiddenCount = cj_hidden_count->valueint;
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
				std::vector<std::pair<AutoEncoderBackPropagation::TrainingConfig, uint32_t>> schedule;
				AutoEncoderBackPropagation::TrainingConfig train_config;
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
					cJSON* cj_adadelta_decay = cJSON_GetObjectItem(cj_train_config, "AdadeltaDecay");

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
					if(cj_adadelta_decay)
					{
						if(cj_adadelta_decay->valuedouble >= 0.0 && cj_adadelta_decay->valuedouble < 1.0)
						{
							train_config.AdadeltaDecay = (float)cj_hidden_dropout->valuedouble;
						}
					}

					// save off this schedule and epoch count
					schedule.push_back(std::pair<AutoEncoderBackPropagation::TrainingConfig, uint32_t>(train_config, epochs));
				}

				// finally construct our training schedule
				result = new TrainingSchedule<AutoEncoderBackPropagation>(model_config, minibatch_size);
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
}