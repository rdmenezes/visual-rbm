struct SourceCalcEnabled : public SiCKL::Source
{
	// probability a given unit will be enabled
	float DROPOUT_PROB;

	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<UInt>, in_seeds)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(UInt, out_state)
			OUT_DATA(UInt, out_seed)
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

struct SourceCopyVisible : public SiCKL::Source
{
	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<Float>, in_data)
			CONST_DATA(Buffer2D<UInt>, in_enabled_visible)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(Float, result)
		END_OUT_DATA

		BEGIN_MAIN
			If(in_enabled_visible(Index().X, 0) == 1u)
				result = in_data(Index().X, Index().Y);
			Else
				result = 0.0f;
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
			CONST_DATA(Buffer2D<UInt>, in_enabled_hidden)
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

			If(in_enabled_hidden(j, 0) == 0u)
				// destination is disabled
				out_activation = 0.0f;
			Else
				Float accumulation = 0.0f;

				ForInRange(i, 0, VISIBLE_UNITS)
					Float v_i = in_visible(i, m);
					Float w_ij = in_weights(i+1, j+1);
					accumulation  = accumulation + (v_i * w_ij);
				EndFor
				// take input dropout into account
				accumulation = accumulation * (1.0f / (1.0f - VISIBLE_DROPOUT_PROB));
				// add bias
				accumulation = accumulation + in_weights(0, j+1);

				out_activation = CalcActivation(FUNC, accumulation);
			EndIf

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
				accumulation = accumulation + (h_j * w_jk);
			EndFor

			// take hidden dropout into acccoount
			accumulation = accumulation + (1.0f / (1.0f - HIDDEN_DROPOUT_PROB));
			// add bias
			accumulation = accumulation + in_weights(k+1, 0);

			out_activation = CalcActivation(FUNC, accumulation);
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

			Float t_k = in_labels(k, m);
			Float z_k = in_outputs(k, m);

			out_sensitivity = (t_k - z_k) * CalcActivationPrime(FUNC, z_k);

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
			CONST_DATA(Buffer2D<UInt>, in_enabled_hidden)
			CONST_DATA(Buffer2D<Float>, in_weights)
			CONST_DATA(Buffer2D<Float>, in_hidden)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(Float, out_sensitivitiy)
		END_OUT_DATA

		BEGIN_MAIN
			Int j = Index().X;
			Int m = Index().Y;

			If(in_enabled_hidden(j, 0) == 1u)
				Float dp = 0.0f;
				ForInRange(k, 0, VISIBLE_UNITS)
					Float d_k = in_output_sensitivities(k, m);
					Float w_kj = in_weights(k+1, j+1);
					dp = dp + (w_kj * d_k);
				EndFor

				out_sensitivitiy = dp * CalcActivationPrime(FUNC, in_hidden(j, m));

			Else
				out_sensitivitiy = 0.0f;
			EndIf

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
	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<Float>, in_output_sensitivities)
			CONST_DATA(Buffer2D<Float>, in_hidden_sensitivities)
			CONST_DATA(Buffer2D<Float>, in_hidden)
			CONST_DATA(Buffer2D<Float>, in_visible)
			CONST_DATA(Buffer2D<Float>, in_prev_weights)
			CONST_DATA(Buffer2D<Float>, in_prev_weight_deltas)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(Float, out_weight)
			OUT_DATA(Float, out_weight_delta)
		END_OUT_DATA

		BEGIN_MAIN
			auto& d_k = in_output_sensitivities;
			auto& d_j = in_hidden_sensitivities;

			auto& i = Index().X;
			auto& j = Index().Y;
			auto& k = Index().X;

			If(j == 0 && k == 0)
				// top left corner, not a weight
				out_weight_delta = 0.0f;
				out_weight = 0.0f;
			ElseIf(j == 0)
				// output bias
				Float Dw_kj = 0.0f;
				ForInRange(m, 0, MINIBATCH_SIZE)
					Dw_kj = Dw_kj + d_k(k-1, m);
				EndFor
				Dw_kj = Dw_kj * (1.0f / MINIBATCH_SIZE);

				out_weight_delta = MOMENTUM * in_prev_weight_deltas(k, j) + 
					(1.0f - MOMENTUM) * Dw_kj;
				out_weight = in_prev_weights(k, j) + LEARNING_RATE * out_weight_delta;
			ElseIf(k == 0)
				// hidden bias
				Float Dw_ji = 0.0f;
				ForInRange(m, 0, MINIBATCH_SIZE)
					Dw_ji = Dw_ji + d_j(j-1, m);
				EndFor
				Dw_ji = Dw_ji * (1.0f / MINIBATCH_SIZE);

				out_weight_delta = MOMENTUM * in_prev_weight_deltas(i, j) + 
					(1.0f - MOMENTUM) * Dw_ji;
				out_weight = in_prev_weights(i, j) + LEARNING_RATE * out_weight_delta;
			Else
				Float Dw_ji = 0.0f;
				Float Dw_kj = 0.0f;
				ForInRange(m, 0, MINIBATCH_SIZE)
					Dw_ji = Dw_ji + d_j(j-1, m) * in_visible(i-1, m);
					Dw_kj = Dw_kj + d_k(k-1, m) * in_hidden(j-1, m);
				EndFor
				
				Float delta = (Dw_ji + Dw_kj) * (1.0f / (2.0f * MINIBATCH_SIZE));

				out_weight_delta = MOMENTUM * in_prev_weight_deltas(i, j) + 
				(1.0f - MOMENTUM) * delta;


				Float prev_weight = in_prev_weights(i, j);
				// Regularization logic (we don't want to do weight decay on biases)

				// only L1
				if(L1_REGULARIZATION != 0.0f && L2_REGULARIZATION == 0.0f)
				{
					Float l1 = Sign(prev_weight) * L1_REGULARIZATION;

					out_weight = prev_weight + LEARNING_RATE * (out_weight_delta - l1);
				}
				// only L2
				else if(L1_REGULARIZATION == 0.0f && L2_REGULARIZATION != 0.0f)
				{
					Float l2 = prev_weight * L2_REGULARIZATION;

					out_weight = prev_weight + LEARNING_RATE * (out_weight_delta - l2);
				}
				// both L1 and L2
				else if(L1_REGULARIZATION != 0.0f && L2_REGULARIZATION != 0.0f)
				{
					Float l1 = Sign(prev_weight) * L1_REGULARIZATION;
					Float l2 = prev_weight * L2_REGULARIZATION;

					out_weight = prev_weight + LEARNING_RATE * (out_weight_delta - l1 - l2);
				}
				// no L1 or L2 regularization
				else
				{
					out_weight = prev_weight + LEARNING_RATE * out_weight_delta;
				}
			EndIf
		END_MAIN
	END_SOURCE
};

struct SourceCalcErrorVector : public SiCKL::Source
{
	int32_t MINIBATCH_SIZE;

	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<Float>, in_visible)
			CONST_DATA(Buffer2D<Float>, in_visible_recon)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(Float, out_square_error)
		END_OUT_DATA

		BEGIN_MAIN
			Int i = Index().X;
			out_square_error = 0.0f;
			ForInRange(k, 0, MINIBATCH_SIZE)
				const Float v = in_visible(i, k);
				const Float v_prime = in_visible_recon(i, k);
				const Float diff = v - v_prime;

				out_square_error = out_square_error + (diff * diff);
			EndFor
			out_square_error = out_square_error * (1.0f / MINIBATCH_SIZE);

		END_MAIN
		END_SOURCE
};