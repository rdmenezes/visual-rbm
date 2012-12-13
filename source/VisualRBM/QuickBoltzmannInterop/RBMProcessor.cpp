#include "RBMProcessor.h"

#include "IDX.hpp"
#include "RBM.hpp"
#include "../QuickBoltzmannNative/RBMTrainer.h"


RBMTrainer* rbmtrainer = NULL;


using namespace System;
using namespace System::Windows::Forms;

namespace QuickBoltzmann
{
	void ShowError(String^ error)
	{
		System::Windows::Forms::MessageBox::Show(error, "Error", System::Windows::Forms::MessageBoxButtons::OK, System::Windows::Forms::MessageBoxIcon::Error);
	}

	void RBMProcessor::Run()
	{
		rbmtrainer = new RBMTrainer();

		if(rbmtrainer->GetLastErrorCode() == RequiredOpenGLVersionUnsupported)
		{
			ShowError("VisualRBM requires a GPU that supports at least OpenGL 3.3.  ");
			exit(1);
		}

		_message_queue = gcnew MessageQueue();

		_currentState = RBMProcessorState::Ready;
		while(true)
		{
			Message^ m = _message_queue->Dequeue();

			if(m != nullptr)
			{

				if(m->Type == "Start")
				{
					if(_epochs > 0)
					{
						_currentState = RBMProcessorState::Training;
						if(rbmtrainer->GetIsInitialized() == false)
						{
							rbmtrainer->Initialize();
							Action^ callback = dynamic_cast<Action^>(m["callback"]);
							if(callback != nullptr)
							{
								callback();
							}
						}
					}
				}
				else if(m->Type == "Pause")
				{
					_currentState = RBMProcessorState::Paused;
				}
				else if(m->Type == "Stop")
				{
					_currentState = RBMProcessorState::Ready;
					_epochs = 100;
					_total_iterations = 0;
					_iterations = 0;
					rbmtrainer->Reset();

				}
				else if(m->Type == "GetVisible")
				{
					if(rbmtrainer->GetIsInitialized() == false)
					{
						// this message needs to be gracefully discarded
						m["image"] = nullptr;
						m["reconstruction"] = nullptr;
						m["done"] = true;
					}
					else
					{
						float* image_buff = new float[rbmtrainer->GetMinibatchSize() * rbmtrainer->GetVisibleCount()];
						float* recon_buff = new float[rbmtrainer->GetMinibatchSize() * rbmtrainer->GetVisibleCount()];

						if(rbmtrainer->DumpVisible(image_buff, recon_buff))
						{
							List<array<float>^>^ images = gcnew List<array<float>^>();
							List<array<float>^>^ recons = gcnew List<array<float>^>();

							// hacky lambda function
							struct 
							{
								void operator() (uint32_t MiniBatches, uint32_t VisibleCount, float* RawBuff, List<array<float>^>^ ImageList)
								{
									for(uint32_t k = 0; k < MiniBatches; k++)
									{
										array<float>^ image = gcnew array<float>(VisibleCount);
										ImageList->Add(image);

										System::Runtime::InteropServices::Marshal::Copy(IntPtr(RawBuff), image, 0, VisibleCount);

										RawBuff += VisibleCount;
									}
								}
							} interop_copy;

							interop_copy(rbmtrainer->GetMinibatchSize(), rbmtrainer->GetVisibleCount(), image_buff, images);
							interop_copy(rbmtrainer->GetMinibatchSize(), rbmtrainer->GetVisibleCount(), recon_buff, recons);


							m["image"] = images;
							m["reconstruction"] = recons;
							m["done"] = true;
						}
						else
						{
							_message_queue->Enqueue(m);
						}

						delete[] image_buff;
						delete[] recon_buff;
					}

				}
				else if(m->Type == "GetHidden")
				{
					if(rbmtrainer->GetIsInitialized() == false)
					{
						// this message needs to be gracefully discarded
						m["probabilities"] = nullptr;
						m["done"] = true;
					}
					else
					{
						float* hidden_buff = new float[rbmtrainer->GetMinibatchSize() * rbmtrainer->GetHiddenCount()];

						if(rbmtrainer->DumpHidden(hidden_buff))
						{
							List<array<float>^>^ probs = gcnew List<array<float>^>();
							float* h_vec = hidden_buff;
							for(uint32_t k = 0; k < rbmtrainer->GetMinibatchSize(); k++)
							{
								array<float>^ activations = gcnew array<float>(rbmtrainer->GetHiddenCount());
								probs->Add(activations);

								System::Runtime::InteropServices::Marshal::Copy(IntPtr(h_vec), activations, 0, rbmtrainer->GetHiddenCount());

								h_vec += rbmtrainer->GetHiddenCount();
							}

							m["probabilities"] = probs;
							m["done"] = true;
						}
						else
						{
							_message_queue->Enqueue(m);
						}

						delete[] hidden_buff;
					}
				}
				else if(m->Type == "GetWeights")
				{
					if(rbmtrainer->GetIsInitialized() == false)
					{
						// this message needs to be gracefully discarded
						m["weights"] = nullptr;
						m["done"] = true;
					}
					else
					{
						array<float>^ weights = gcnew array<float>((rbmtrainer->GetVisibleCount() + 1) * (rbmtrainer->GetHiddenCount() + 1));
						pin_ptr<float> ptr = &weights[0];
						if(rbmtrainer->DumpWeights(ptr))
						{
							m["weights"] = weights;
							m["done"] = true;
						}
						else
						{
							_message_queue->Enqueue(m);
						}
					}
				}
				else if(m->Type == "Export")
				{
					RBM* rbm = rbmtrainer->GetRBM();

					IntPtr p = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(dynamic_cast<String^>(m["filename"]));
					char* filename = static_cast<char*>(p.ToPointer());

					rbm->Save(filename);


					delete rbm;
					System::Runtime::InteropServices::Marshal::FreeHGlobal(p);

				}
				else if(m->Type == "Import")
				{
					IntPtr p = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(dynamic_cast<String^>(m["filename"]));
					char* filename = static_cast<char*>(p.ToPointer());

					RBM* rbm = RBM::Load(filename);

					if(rbm == NULL)
					{
						ShowError(String::Format("Problem parsing {0} as RBM", m["filename"]));
					}
					else if(rbm->_visible_count == rbmtrainer->GetVisibleCount())
					{
						rbmtrainer->SetRBM(rbm);
					}
					else
					{
						delete rbm;
						ShowError(String::Format("Loaded RBM has {0} Visible units while our dataset has {1} Inputs", rbm->_visible_count, rbmtrainer->GetVisibleCount()));
					}

					

					System::Runtime::InteropServices::Marshal::FreeHGlobal(p);

					m["done"] = true;
				}
				else if(m->Type == "Shutdown")
				{
					if(rbmtrainer->GetIsInitialized() == true)
					{
						rbmtrainer->Reset();
					}
					delete rbmtrainer;
					m["done"] = true;
				}
				else if(m->Type == "SetTrainingData")
				{
					IDX* data = (IDX*)(LONG_PTR)m["idx"];
					bool calc = (bool)m["calculate"];

					m["result"] = rbmtrainer->SetTrainingData(data, calc);
					m["done"] = true;
				}
			}

			switch(_currentState)
			{
			case RBMProcessorState::Ready:
			case RBMProcessorState::Paused:
				Thread::Sleep(100);
				break;
			case RBMProcessorState::Training:
				rbmtrainer->Train();	/// training
				
				_iterations++;
				_total_iterations++;
				IterationCompleted(_total_iterations, rbmtrainer->GetReconstructionError(), rbmtrainer->GetValidationReconstructionError());

				if(_iterations == rbmtrainer->GetMinibatches())
				{
					_iterations = 0;
					EpochCompleted(_epochs--);

					if(_epochs == 0)
					{
						
						TrainingCompleted();
						Pause();
						break;
					}

					
				}
			}
		}
	}

	void RBMProcessor::Start(Action^ act)
	{
		Message^ start = gcnew Message("Start");
		start["callback"] = act;
		_message_queue->Enqueue(start);
	}

	void RBMProcessor::Pause()
	{
		_message_queue->Enqueue(gcnew Message("Pause"));
	}

	void RBMProcessor::Stop()
	{
		_message_queue->Enqueue(gcnew Message("Stop"));
	}

	void RBMProcessor::GetCurrentVisible(List<array<float>^>^% visible_data, List<array<float>^>^% visible_reconstruction)
	{
		Message^ m = gcnew Message("GetVisible");
		m["done"] = false;

		
		_message_queue->Enqueue(m);
		while((bool)m["done"] == false)
		{
			Thread::Sleep(16);
		}

		visible_data = dynamic_cast<List<array<float>^>^>(m["image"]);
		visible_reconstruction = dynamic_cast<List<array<float>^>^>(m["reconstruction"]);
	}

	void RBMProcessor::GetCurrentHidden(List<array<float>^>^% hidden_prob)
	{
		Message^ m = gcnew Message("GetHidden");
		m["done"] = false;
		_message_queue->Enqueue(m);
		while((bool)m["done"] == false)
		{
			Thread::Sleep(16);
		}

		hidden_prob = dynamic_cast<List<array<float>^>^>(m["probabilities"]);
	}

	void RBMProcessor::GetCurrentWeights(array<float>^% weights)
	{
		Message^ m = gcnew Message("GetWeights");
		m["done"] = false;
		_message_queue->Enqueue(m);
		while((bool)m["done"] == false)
		{
			Thread::Sleep(16);
		}

		weights = dynamic_cast<array<float>^>(m["weights"]);		
	}

	float RBMProcessor::GetReconstructionError()
	{
		return 0.0f;
	}

	bool RBMProcessor::SetTrainingData(String^ in_filename, bool calc_stats)
	{
		IntPtr p = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(in_filename);
		char* filename = static_cast<char*>(p.ToPointer());

		IDX* idx = IDX::Load(filename);
		System::Runtime::InteropServices::Marshal::FreeHGlobal(p);

		// verify it's the right data type
		if(idx->GetDataFormat() != DataFormat::Single)
		{
			ShowError("Error: IDX training data must have type 'Float' (0x0D)");
			_has_training_data = false;
			return false;
		}

		Message^ m = gcnew Message("SetTrainingData");
		m["idx"] = (LONG_PTR)idx;
		m["calculate"] = calc_stats;
		m["done"] = false;
		_message_queue->Enqueue(m);

		while((bool)m["done"] == false)
		{
			Thread::Sleep(16);
		}

		if((bool)m["result"])
		{
			_has_training_data = true;
			return true;		
		}

		// there was a problem, print a useful error code
		_has_training_data = false;
		switch(rbmtrainer->GetLastErrorCode())
		{
		case DataHasIncorrectNumberOfVisibleInputs:
			ShowError(String::Format("RBM has {0} visible units while proposed Training data is length {1}", VisibleUnits, idx->GetRowLength()));
			break;
		case BinaryDataOutsideZeroOne:
			ShowError("Input data has values outside of [0,1] so 'Binary' Visible Type may not be used; instead use 'Gaussian'");
			break;
		case DataContainsInfinite:
			ShowError("Input data contains values that are +/-Infinity");
			break;
		case DataContainsNaN:
			ShowError("Input data contains values that are NaN");
			break;
		}
		return false;
	}

	bool RBMProcessor::SetValidationData(String^ in_filename)
	{
		assert(_has_training_data == true);

		if(in_filename == nullptr)
		{
			// clear out validation data and just don't use it
			rbmtrainer->SetValidationData(NULL);
			_has_validation_data = false;
			return true;
		}
		else
		{
			IntPtr p = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(in_filename);
			char* filename = static_cast<char*>(p.ToPointer());

			IDX* idx = IDX::Load(filename);
			System::Runtime::InteropServices::Marshal::FreeHGlobal(p);

			if(idx->GetDataFormat() != DataFormat::Single)
			{
				ShowError("Error: IDX validation data must have type 'Float' (0x0D)");
				_has_validation_data = false;
				return false;
			}

			if(rbmtrainer->SetValidationData(idx))
			{
				_has_validation_data = true;
				return true;
			}

			// there was a problem loading up data, so throw up an error box
			_has_validation_data = false;
			switch(rbmtrainer->GetLastErrorCode())
			{
			case DataHasIncorrectNumberOfVisibleInputs:
				ShowError(String::Format("Training data is length {0} while Validation data is length {1}", VisibleUnits, idx->GetRowLength()));
				break;
			case BinaryDataOutsideZeroOne:
				ShowError("Validation data has values outside of [0,1] so 'Binary' Visible Type may not be used; instead use 'Gaussian'");
				break;
			case DataContainsInfinite:
				ShowError("Validation data contains values that are +/-Infinity");
				break;
			case DataContainsNaN:
				ShowError("Validation data contains values that are NaN");
				break;
			}
			return false;
		}
	}

	void RBMProcessor::SaveRBM(String^ in_filename)
	{
		Message^ m = gcnew Message("Export");
		m["filename"] = in_filename;
		_message_queue->Enqueue(m);
	}

	void RBMProcessor::LoadRBM(String^ in_filename,  uint32_t% hidden_units, bool% linear_units)
	{
		Message^ m = gcnew Message("Import");
		m["filename"] = in_filename;
		m["done"] = false;
		_message_queue->Enqueue(m);
		while((bool)m["done"] == false)
		{
			Thread::Sleep(16);
		}
		hidden_units = rbmtrainer->GetHiddenCount();
		linear_units = rbmtrainer->GetVisibleType() == Gaussian;
	}

	void RBMProcessor::Shutdown()
	{
		Message^ m = gcnew Message("Shutdown");
		m["done"] = false;
		_message_queue->Enqueue(m);
		while((bool)m["done"] == false)
		{
			Thread::Sleep(16);
		}
	}
}