struct SourceCalcEnabled : public SiCKL::Source
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

struct SourceCalcHidden : public SiCKL::Source
{
	ActivationFunction_t FUNC;
	float VISIBLE_DROPOUT_PROB;
	uint32_t VISIBLE_UNITS;

	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<Float>, in_visible)
			CONST_DATA(Buffer2D<UInt>, in_enabled_visible)
			CONST_DATA(Buffer2D<Float>, in_weights)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(Float, out_activation)
		END_OUT_DATA

		BEGIN_MAIN
			// hidden unit we are calculating
			Int j = Index().X;
			// hidden vector we are calculating
			Int m = Index().Y;

			Float accumulation = 0.0f;

			ForInRange(i, 0, VISIBLE_UNITS)
				Float v_i = in_visible(i, m);
				Float w_ij = in_weights(i+1, j+1);
				Float enabled = in_enabled_visible(i, 0);
				accumulation  = accumulation + (v_i * w_ij * enabled);
			EndFor
			// take input dropout into account
			accumulation = accumulation * (1.0f / (1.0f - VISIBLE_DROPOUT_PROB));
			// add bias
			accumulation = accumulation + in_weights(0, j+1);

			out_activation = CalcActivation(FUNC, accumulation);
		END_MAIN

	END_SOURCE
};

struct SourceCalcOutput : public SiCKL::Source
{
	ActivationFunction_t FUNC;
	float HIDDEN_DROPOUT_PROB;
	uint32_t HIDDEN_UNITS;

	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<Float>, in_hidden)
			CONST_DATA(Buffer2D<UInt>, in_enabled_hidden)
			CONST_DATA(Buffer2D<Float>, in_weights)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(Float, out_activation)
		END_OUT_DATA

		BEGIN_MAIN
			// output unit we are calculating
			Int k = Index().X;
			// output vector we are calculating
			Int m = Index().Y;

			Float accumulation = 0.0f;

			ForInRange(j, 0, HIDDEN_UNITS)
				Float h_j = in_hidden(j, m);
				Float w_jk = in_weights(k+1, j+1);
				Float enabled = in_enabled_hidden(j, 0);
				accumulation = accumulation + (h_j * w_jk * enabled);
			EndFor

			// take hidden dropout into acccount
			accumulation = accumulation * (1.0f / (1.0f - HIDDEN_DROPOUT_PROB));
			// add bias
			accumulation = accumulation + in_weights(k+1, 0);

			// calc activation
			out_activation = CalcActivation(FUNC, accumulation);
		END_MAIN

	END_SOURCE
};

struct SourceSoftmax : public SiCKL::Source
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

struct SourceCalcOutputSensitivities : public SiCKL::Source
{
	ActivationFunction_t FUNC;
	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<Float>, in_labels)
			CONST_DATA(Buffer2D<Float>, in_outputs)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(Float, out_sensitivity)
		END_OUT_DATA

		BEGIN_MAIN
			Int k = Index().X;
			Int m = Index().Y;

			Float label = in_labels(k, m);
			Float activation = in_outputs(k, m);

			Float diff = label - activation;
			if(FUNC == ActivationFunction::Softmax)
			{
				// cross entropy error
				out_sensitivity = diff;
			}
			else
			{
				// squared error
				out_sensitivity = diff * CalcActivationPrime(FUNC, activation);
			}

		END_MAIN

	END_SOURCE
};

struct SourceCalcHiddenSensitivities : public SiCKL::Source
{
	ActivationFunction_t FUNC;
	uint32_t VISIBLE_UNITS;
	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<Float>, in_output_sensitivities)
			CONST_DATA(Buffer2D<UInt>, in_enabled_visible)
			CONST_DATA(Buffer2D<Float>, in_weights)
			CONST_DATA(Buffer2D<Float>, in_hidden)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(Float, out_sensitivitiy)
		END_OUT_DATA

		BEGIN_MAIN
			Int j = Index().X;
			Int m = Index().Y;


			Float dp = 0.0f;
			ForInRange(k, 0, VISIBLE_UNITS)
				Float d_k = in_output_sensitivities(k, m);
				Float w_kj = in_weights(k+1, j+1);
				Float enabled = in_enabled_visible(k, 0);
				dp = dp + (w_kj * d_k);
			EndFor

			out_sensitivitiy = dp * CalcActivationPrime(FUNC, in_hidden(j, m));

			END_MAIN

	END_SOURCE
};

struct SourceUpdateWeights : public SiCKL::Source
{
	uint32_t MINIBATCH_SIZE;
	float LEARNING_RATE;
	float MOMENTUM;
	float L1_REGULARIZATION;
	float L2_REGULARIZATION;
	float VISIBLE_DROPOUT;
	float HIDDEN_DROPOUT;
	float ADADELTA_DECAY;

	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<Float>, in_output_sensitivities)
			CONST_DATA(Buffer2D<Float>, in_hidden_sensitivities)
			CONST_DATA(Buffer2D<Float>, in_hidden)
			CONST_DATA(Buffer2D<Float>, in_visible)
			CONST_DATA(Buffer2D<Float>, in_prev_weights)
			CONST_DATA(Buffer2D<Float>, in_prev_weight_deltas)
			CONST_DATA(Buffer2D<Float2>, in_mean_square)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(Float, out_weight)
			OUT_DATA(Float, out_weight_delta)
			OUT_DATA(Float2, out_mean_square)
			OUT_DATA(Float, out_nesterov_weight)
		END_OUT_DATA

		BEGIN_MAIN
			auto& d_k = in_output_sensitivities;
			auto& d_j = in_hidden_sensitivities;

			auto& i = Index().X;
			auto& j = Index().Y;
			auto& k = Index().X;

			// epsilon for calculating Adadelta scaling factor
			const float eps = 1.0e-6f;

			Float delta_w = 0.0f;
			Float weight_decay = 0.0f;

			const Float prev_weight = in_prev_weights(i, j);

			If(j == 0 && k == 0)
				// top left corner, not a weight
				delta_w = 0.0f;
				weight_decay = 0.0f;
			ElseIf(j == 0)
				// output bias
				Float Dw_kj = 0.0f;
				ForInRange(m, 0, MINIBATCH_SIZE)
					Dw_kj = Dw_kj + d_k(k-1, m);
				EndFor

				delta_w = Dw_kj;
			ElseIf(k == 0)
				// hidden bias
				Float Dw_ji = 0.0f;
				ForInRange(m, 0, MINIBATCH_SIZE)
					Dw_ji = Dw_ji + d_j(j-1, m);
				EndFor

				delta_w = Dw_ji;
			Else
				Float Dw_ji = 0.0f;
				Float Dw_kj = 0.0f;
				ForInRange(m, 0, MINIBATCH_SIZE)
					// visible to hidden
					Dw_ji = Dw_ji + d_j(j-1, m) * in_visible(i-1, m);
					// hidden to output
					Dw_kj = Dw_kj + d_k(k-1, m) * in_hidden(j-1, m);
				EndFor
				
				Dw_ji = Dw_ji / (1.0f - VISIBLE_DROPOUT);
				Dw_kj = Dw_kj / (1.0f - HIDDEN_DROPOUT);

				delta_w = (Dw_ji + Dw_kj) / 2.0f;

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
			EndIf

			delta_w = delta_w * (1.0f / float(MINIBATCH_SIZE));

			// calculate mean square weight derivative
			auto& out_mean_square_derivative = out_mean_square.X;
			auto& prev_mean_square_derivative = in_mean_square(Index()).X;

			out_mean_square_derivative = (1.0f - ADADELTA_DECAY) * (delta_w*delta_w) + ADADELTA_DECAY * prev_mean_square_derivative;

			// calculate weight delta
			auto& prev_delta = in_prev_weight_deltas(Index());
			auto& prev_mean_square_delta = in_mean_square(Index()).Y;

			Float adadelta_factor = Sqrt(prev_mean_square_delta + eps) / Sqrt(out_mean_square_derivative + eps);

			out_weight_delta = MOMENTUM * prev_delta + LEARNING_RATE * adadelta_factor * (delta_w - weight_decay);

			// calculate mean square weight delta
			auto& out_mean_square_delta = out_mean_square.Y;

			out_mean_square_delta = (1.0f - ADADELTA_DECAY) * (out_weight_delta * out_weight_delta) + ADADELTA_DECAY * prev_mean_square_delta;

			// update the weight
			out_weight = prev_weight + out_weight_delta;

			// calculate the weight for t + 1/2 for nesterov momentum
			out_nesterov_weight = out_weight + MOMENTUM * out_weight_delta;

		END_MAIN
	END_SOURCE
};