struct SourceCalcEnabled  : public SiCKL::Source
{
	// probability a given unit will be enabled
	float DROPOUT_PROB;

	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<UInt4>, in_seeds)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(UInt, out_state)
			OUT_DATA(UInt4, out_seed)
		END_OUT_DATA

		BEGIN_MAIN
			Float p;
			NextFloat(in_seeds(Index().X, Index().Y), out_seed, p);

			If(p > DROPOUT_PROB)
				out_state = 1u;
			Else
				out_state = 0u;
			EndIf
		END_MAIN

	END_SOURCE
};

struct SourceCalcHiddenAndStates : public SiCKL::Source
{
	ActivationFunction_t FUNCTION;	
	int32_t VISIBLE_UNITS;
	float VISIBLE_DROPOUT_PROB;

	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<Float>, in_visible)
			CONST_DATA(Buffer2D<Float>, in_weights)
			CONST_DATA(Buffer2D<UInt>, in_enabled_visible)
			CONST_DATA(Buffer2D<UInt4>, in_seeds)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(UInt4, out_seed)	
			OUT_DATA(Float, out_hidden)
			OUT_DATA(Float, out_state)
		END_OUT_DATA

		BEGIN_MAIN
			const Int m = Index().Y;	// what minibatch are we on
			const Int j = Index().X;	// which hidden unit is this

			const auto& seed = in_seeds(Index().X, Index().Y);

			Float accumulation = 0.0f;

			ForInRange(i, 0, VISIBLE_UNITS)
				Float v_i = in_visible(i, m);
				Float w_ij = in_weights(i+1, j+1);
				Float enabled = in_enabled_visible(i, 0);
				accumulation = accumulation +  (v_i * enabled * w_ij);
			EndFor
			// take input dropout into account
			accumulation = accumulation  * (1.0f / (1.0f - VISIBLE_DROPOUT_PROB));
			// add bias
			accumulation = accumulation + in_weights(0, j+1);

			switch(FUNCTION)
			{
				case ActivationFunction::Linear:
					{
						out_hidden = accumulation;
						Float noise;
						NextGaussian(seed, out_seed, noise);
						out_state = accumulation + noise;
					}
					break;
				case ActivationFunction::RectifiedLinear:
					{
						out_hidden = Max(accumulation, 0.0f);
						Float noise;
						// mean 0, variance sigmoid(x) per http://www.cs.toronto.edu/~hinton/absps/reluICML.pdf
						// "Rectified linear units improve restricted Boltzmann machines."
						NextGaussian(seed, out_seed, noise);
						auto variance = Sigmoid(accumulation);
						auto stddev = Sqrt(variance);
						noise = noise * stddev;
						out_state = Max(accumulation + noise, 0.0f);
					}
					break;
				case ActivationFunction::Sigmoid:
					{
						out_hidden = Sigmoid(accumulation);
						Float prob;
						NextFloat(seed, out_seed, prob);

						If(prob <= out_hidden)
							out_state = 1.0f;
						Else
							out_state = 0.0f;
						EndIf
					}
					break;
			}
		END_MAIN
	END_SOURCE
};

struct SourceCalcHiddenSoftmaxStates : public SiCKL::Source
{
	uint32_t ROW_LENGTH;

	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<Float>, in_accumulation)
			CONST_DATA(Buffer2D<UInt4>, in_seeds)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(UInt4, out_seed)
			OUT_DATA(Float, out_softmax)
			OUT_DATA(Float, out_state)
		END_OUT_DATA

		BEGIN_MAIN
			const auto row = Index().Y;

			// first we need to find the max activation (for numerical stability)
			Float max = -FLT_MAX;

			ForInRange(i, 0, ROW_LENGTH)
				max = Max(max, in_accumulation(i, row));
			EndFor
			
			// calculate the denominator
			Float denominator = 0.0f;
			ForInRange(i, 0, ROW_LENGTH)
				denominator = denominator + Exp(in_accumulation(i, row) - max);
			EndFor

			// calculate the numerator
			Float numerator = Exp(in_accumulation(Index()) - max);

			// result
			out_softmax = numerator / denominator;

			// now set hidden state
			auto& seed = in_seeds(Index());
			Float prob;
			NextFloat(seed, out_seed, prob);

			If(prob <= out_softmax)
				out_state = 1.0f;
			Else
				out_state = 0.0f;
			EndIf
		END_MAIN
	END_SOURCE
};


struct SourceCalcVisible : public SiCKL::Source
{
	ActivationFunction_t FUNCTION;
	int32_t HIDDEN_UNITS;
	float HIDDEN_DROPOUT_PROB;

	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<Float>, in_hidden)
			CONST_DATA(Buffer2D<Float>, in_weights)
			CONST_DATA(Buffer2D<UInt>, in_enabled_hidden)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(Float, out_visible)
		END_OUT_DATA
	
		BEGIN_MAIN
			const Int m = Index().Y;	// what minibatch are we on
			const Int i = Index().X;	// which visible unit is this

			Float accumulation = 0.0f;

			ForInRange(j, 0, HIDDEN_UNITS)
				Float h_j = in_hidden(j, m);
				Float w_ij = in_weights(i+1, j+1);
				Float enabled = in_enabled_hidden(j, 0);
				accumulation = accumulation + (h_j * w_ij * enabled);
			EndFor
			// take input dropout into account
			accumulation = accumulation  * (1.0f / (1.0f - HIDDEN_DROPOUT_PROB));
			// add bias
			accumulation = accumulation + in_weights(i+1, 0);

			switch(FUNCTION)
			{
			case ActivationFunction::Softmax:
			case ActivationFunction::Linear:
				{
					out_visible = accumulation;
				}
				break;
			case ActivationFunction::RectifiedLinear:
				{
					out_visible = Max(accumulation, 0.0f);
				}
				break;
			case ActivationFunction::Sigmoid:
				{
					out_visible = Sigmoid(accumulation);
				}
				break;
			}
		END_MAIN
	END_SOURCE
};

struct SourceCalcSoftmax : public SiCKL::Source
{
	uint32_t ROW_LENGTH;
	
	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<Float>, in_inputs)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(Float, out_output)
		END_OUT_DATA

		BEGIN_MAIN
			const auto row = Index().Y;

			// first we need to find the max activation (for numerical stability)
			Float max = -FLT_MAX;

			ForInRange(i, 0, ROW_LENGTH)
				max = Max(max, in_inputs(i, row));
			EndFor
			
			// calculate the denominator
			Float denominator = 0.0f;
			ForInRange(i, 0, ROW_LENGTH)
				denominator = denominator + Exp(in_inputs(i, row) - max);
			EndFor

			// calculate the numerator
			Float numerator = Exp(in_inputs(Index()) - max);

			// result
			out_output = numerator / denominator;
		END_MAIN
	END_SOURCE
};

struct SourceCalcHidden : public SiCKL::Source
{
	ActivationFunction_t FUNCTION;	
	int32_t VISIBLE_UNITS;
	float VISIBLE_DROPOUT_PROB;

	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<Float>, in_visible)
			CONST_DATA(Buffer2D<Float>, in_weights)
			CONST_DATA(Buffer2D<UInt>, in_enabled_visible)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(Float, out_hidden)
		END_OUT_DATA

		BEGIN_MAIN
			const Int m = Index().Y;	// what minibatch are we on
			const Int j = Index().X;	// which hidden unit is this

			Float accumulation = 0.0f;

			ForInRange(i, 0, VISIBLE_UNITS)
				Float v_i = in_visible(i, m);
				Float w_ij = in_weights(i+1, j+1);
				Float enabled = in_enabled_visible(i, 0);
				accumulation = accumulation +  (v_i * w_ij * enabled);
			EndFor
			// take input dropout into account
			accumulation = accumulation  * (1.0f / (1.0f - VISIBLE_DROPOUT_PROB));
			// add bias
			accumulation = accumulation + in_weights(0, j+1);

			switch(FUNCTION)
			{
				case ActivationFunction::Softmax:
				case ActivationFunction::Linear:
					{
						out_hidden = accumulation;
					}
					break;
				case ActivationFunction::RectifiedLinear:
					{
						out_hidden = Max(accumulation, 0.0f);
					}
					break;
				case ActivationFunction::Sigmoid:
					{
						out_hidden = Sigmoid(accumulation);
					}
					break;
			}
		END_MAIN
	END_SOURCE
};

struct SourceCalcWeightUpdates : public SiCKL::Source
{
	/// compile time constants
	int32_t MINIBATCH_SIZE;
	float LEARNING_RATE;
	float MOMENTUM;
	float L1_REGULARIZATION;
	float L2_REGULARIZATION;
	float ADADELTA_DECAY;

	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<Float>, in_visible)
			CONST_DATA(Buffer2D<Float>, in_hidden)
			CONST_DATA(Buffer2D<Float>, in_visible_prime)
			CONST_DATA(Buffer2D<Float>, in_hidden_prime)
			CONST_DATA(Buffer2D<Float>, in_delta)
			CONST_DATA(Buffer2D<Float>, in_weight)
			CONST_DATA(Buffer2D<UInt>, in_enabled_visible)
			CONST_DATA(Buffer2D<UInt>, in_enabled_hidden)
			CONST_DATA(Buffer2D<Float2>, in_mean_square)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(Float, out_delta)
			OUT_DATA(Float, out_weight)
			OUT_DATA(Float2, out_mean_square)
			OUT_DATA(Float, out_nesterov_weight)
		END_OUT_DATA

		BEGIN_MAIN
			const Int& i = Index().X;
			const Int& j = Index().Y;

			// epsilon for calculating Adadelta scaling factor
			const float eps = 1.0e-6f;


			Float delta_w = 0.0f;
			Float weight_decay = 0.0f;
			Bool dropped_out = false;

			const Float prev_weight = in_weight(Index());

			If(i == 0 && j == 0)
				// top left corner, not a weight
				delta_w = 0.0f;
				weight_decay = 0.0f;
			ElseIf(i == 0 && (in_enabled_hidden(j - 1, 0) == 1u))
				// hidden bias
				delta_w = 0.0f;
				ForInRange(m, 0, MINIBATCH_SIZE)
					const Float hj = in_hidden(j - 1, m);
					const Float hj_prime = in_hidden_prime(j - 1, m);

					delta_w = delta_w + (hj - hj_prime);
				EndFor
				weight_decay = 0.0f;				
			ElseIf(j == 0 && (in_enabled_visible(i - 1, 0) == 1u))
				// visible bias
				delta_w = 0.0f;
				ForInRange(m, 0, MINIBATCH_SIZE)
					const Float vi = in_visible(i - 1, m);
					const Float vi_prime = in_visible_prime(i - 1, m);

					delta_w = delta_w + (vi - vi_prime);
				EndFor
				weight_decay = 0.0f;
			ElseIf(in_enabled_visible(i - 1, 0) == 1u && in_enabled_hidden(j - 1, 0) == 1u)
				// regular weight
				delta_w = 0.0f; 
				ForInRange(m, 0, MINIBATCH_SIZE)
					Float vi = in_visible(i - 1, m);
					Float vi_prime = in_visible_prime(i - 1, m);

					Float hj = in_hidden(j - 1, m);
					Float hj_prime = in_hidden_prime(j - 1, m);

					delta_w = delta_w + (vi * hj);
					delta_w = delta_w - (vi_prime * hj_prime);
				EndFor
				
				/// L1/L2 regularization

				// only L1
				if(L1_REGULARIZATION != 0.0f && L2_REGULARIZATION == 0.0f)
				{
					Float l1 = Sign(prev_weight) * L1_REGULARIZATION;
					weight_decay = l1;
				}
				// only L2
				else if(L1_REGULARIZATION == 0.0f && L2_REGULARIZATION != 0.0f)
				{
					Float l2 = prev_weight * L2_REGULARIZATION;
					weight_decay = l2;
				} 
				// both L1 and L2
				else if(L1_REGULARIZATION != 0.0f && L2_REGULARIZATION != 0.0f)
				{
					Float l1 = Sign(prev_weight) * L1_REGULARIZATION;
					Float l2 = prev_weight * L2_REGULARIZATION;
					weight_decay = l1 + l2;
				}
				// neither
				else
				{
					weight_decay = 0.0f;
				}
			Else
				dropped_out = true;
			EndIf

			If(dropped_out == false)

				delta_w = delta_w * (1.0f / float(MINIBATCH_SIZE));

				// calculate mean square weight derivative
				auto& out_mean_square_derivative = out_mean_square.X;
				auto& prev_mean_square_derivative = in_mean_square(Index()).X;

				out_mean_square_derivative = (1.0f - ADADELTA_DECAY) * (delta_w*delta_w) + ADADELTA_DECAY * prev_mean_square_derivative;

				// calculate weight delta
				auto& prev_delta = in_delta(Index());
				auto& prev_mean_square_delta = in_mean_square(Index()).Y;

				Float adadelta_factor = Sqrt(prev_mean_square_delta + eps) / Sqrt(out_mean_square_derivative + eps);

				out_delta = MOMENTUM * prev_delta + LEARNING_RATE * adadelta_factor * (delta_w - weight_decay);

				// calculate mean square weight delta
				auto& out_mean_square_delta = out_mean_square.Y;

				out_mean_square_delta = (1.0f - ADADELTA_DECAY) * (out_delta * out_delta) + ADADELTA_DECAY * prev_mean_square_delta;

				// update the weight
				out_weight = prev_weight + out_delta;

				// calculate the weight for t + 1/2 for nesterov momentum
				out_nesterov_weight = out_weight + MOMENTUM * out_delta;

			Else
				out_delta = in_delta(Index());
				out_weight = prev_weight;
				out_mean_square = in_mean_square(Index());
				out_nesterov_weight = out_weight + MOMENTUM * out_delta;
			EndIf
		END_MAIN
	END_SOURCE
};
