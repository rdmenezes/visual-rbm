struct SourceCalcEnabledUnits : public SiCKL::Source
{
	float DROPOUT_PROB;

	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<UInt4>, in_seeds)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(UInt4, out_seed)
			OUT_DATA(Float, out_enabled)
		END_OUT_DATA

		BEGIN_MAIN
			Float prob;
			NextFloat(in_seeds(Index().X, 0), out_seed, prob);

			If(prob > DROPOUT_PROB)
				out_enabled = 1.0f;
			Else
				out_enabled = 0.0f;
			EndIf

		END_MAIN
	END_SOURCE
};

struct SourceCopyVisible : public SiCKL::Source
{
	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<Float>, in_enabled)
			CONST_DATA(Buffer2D<Float>, in_visible)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(Float, out_visible)
		END_OUT_DATA

		BEGIN_MAIN
			If(in_enabled(Index().X, 0) == 1.0f)
				out_visible = in_visible(Index());
			Else
				out_visible = 0.0f;
			EndIf
		END_MAIN
	END_SOURCE
};

struct SourceFeedForward : public SiCKL::Source
{
	ActivationFunction_t FUNC;
	float INPUT_DROPOUT_PROB;
	uint32_t INPUT_COUNT;
	float NOISE_STDDEV;

	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<Float>, in_inputs)
			CONST_DATA(Buffer2D<Float>, in_enabled_outputs)
			CONST_DATA(Buffer2D<Float>, in_weights);
			CONST_DATA(Buffer2D<UInt4>, in_seeds)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(Float, out_activation)
			OUT_DATA(UInt4, out_seed)
		END_OUT_DATA

		BEGIN_MAIN

			// output unit we are calculating
			Int j = Index().X;
			// output vector we're calculating
			Int m = Index().Y;

			If(in_enabled_outputs(j, 0) == 1.0f)
				// bias
				Float accumulation = 0.0f;
				// calculate dot product between feature and input vector
				ForInRange(i, 0, INPUT_COUNT)
					Float input = in_inputs(i, m);
					// offset i by 1 because of bias column
					Float w_ij = in_weights(i + 1, j);

					accumulation = accumulation + (input * w_ij);
				EndFor
				// take input dropout into account
				accumulation = accumulation * (1.0f / (1.0f - INPUT_DROPOUT_PROB));
				// finally add bias
				accumulation = accumulation + in_weights(0, j);


				// add noise if required
				if(NOISE_STDDEV != 0.0f)
				{
					Float noise;
					NextGaussian(in_seeds(j, m), out_seed, noise);
					accumulation = accumulation + (noise * NOISE_STDDEV);
				}

				// activation function
				switch(FUNC)
				{
				case ActivationFunction::Linear:
					out_activation = accumulation;
					break;
				case ActivationFunction::RectifiedLinear:
					out_activation = Max(0.0f, accumulation);
					break;
				case ActivationFunction::Sigmoid:
					out_activation = Sigmoid(accumulation);
					break;
				}
			Else
				out_activation = 0.0f;
				out_seed = in_seeds(j, m);
			EndIf
		END_MAIN
	END_SOURCE
};

static void PartialDerivative(const Float& activation, ActivationFunction_t func, Float& out_partial)
{
	switch(func)
	{
	case ActivationFunction::Linear:
		{
			out_partial = 1.0f;
		}
		break;
	case ActivationFunction::RectifiedLinear:
		{
			out_partial = Max(0.0f, Sign(activation));
		}
		break;
	case ActivationFunction::Sigmoid:
		{
			const Float& sigmoid = activation;
			out_partial = ((1.0f - sigmoid) * sigmoid);
		}
		break;
	default:
		assert(false);
	}
}

struct SourceCalcTopSensitivities : public SiCKL::Source
{
	ActivationFunction_t FUNC;
	uint32_t MINIBATCH_SIZE;

	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<Float>, in_labels)
			CONST_DATA(Buffer2D<Float>, in_activations)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(Float, out_sensitivity)
		END_OUT_DATA

		BEGIN_MAIN

			Int j = Index().X;
			Int m = Index().Y;
			
			Float label = in_labels(j,m);
			Float activation = in_activations(j, m);

			Float diff = label - activation;

			Float partial;
			PartialDerivative(activation, FUNC, partial);
			out_sensitivity = diff * partial;

		END_MAIN
	END_SOURCE
};

struct SourceCalcSensitivities : public SiCKL::Source
{
	ActivationFunction_t FUNC;
	uint32_t MINIBATCH_SIZE;
	uint32_t NEXT_OUTPUT_COUNT;

	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<Float>, in_weights)
			CONST_DATA(Buffer2D<Float>, in_sensitivities)
			CONST_DATA(Buffer2D<Float>, in_activations)
			CONST_DATA(Buffer2D<Float>, in_enabled)
		END_CONST_DATA
			
		BEGIN_OUT_DATA
			OUT_DATA(Float, out_sensitivity)
		END_OUT_DATA

		BEGIN_MAIN
			Int j = Index().X;
			Int m = Index().Y;

			If(in_enabled(j, 0) == 1.0f)
				Float dp = 0.0f;
				ForInRange(k, 0, NEXT_OUTPUT_COUNT)
					// no need to check for enabled outputs, since 
					// in_sensitivities(k, m) will be 0.0f
					Float w_jk = in_weights(j + 1, k);
					Float d_k = in_sensitivities(k, m);

					dp = dp + (w_jk * d_k);
				EndFor

				
				Float partial;
				PartialDerivative(in_activations(j, m), FUNC, partial);
				out_sensitivity = dp * partial;
			Else
				out_sensitivity = 0.0f;
			EndIf

		END_MAIN
	END_SOURCE
};

struct SourceUpdateWeights : public SiCKL::Source
{
	float LEARNING_RATE;
	float MOMENTUM;
	uint32_t MINIBATCH_SIZE;
	float L1_REGULARIZATION;
	float L2_REGULARIZATION;

	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<Float>, in_sensitivities)
			CONST_DATA(Buffer2D<Float>, in_inputs)
			CONST_DATA(Buffer2D<Float>, in_enabled_inputs)
			CONST_DATA(Buffer2D<Float>, in_enabled_outputs)
			CONST_DATA(Buffer2D<Float>, in_prev_weights)
			CONST_DATA(Buffer2D<Float>, in_prev_weight_deltas)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(Float, out_weight)
			OUT_DATA(Float, out_weight_delta)
		END_OUT_DATA

		BEGIN_MAIN
			Int& j = Index().X;
			Int& k = Index().Y;


			// bias update
			If(j == 0 && in_enabled_outputs(k, 0) == 1.0f)
				Float d_k = 0.0f;
				ForInRange(m, 0, MINIBATCH_SIZE)
					d_k = d_k + in_sensitivities(k, m);
				EndFor
				d_k = d_k * (1.0f / MINIBATCH_SIZE);

				out_weight_delta = MOMENTUM * in_prev_weight_deltas(j, k) + 
					(1.0f - MOMENTUM) * d_k;
				out_weight = in_prev_weights(j, k) + LEARNING_RATE * out_weight_delta;
			// dropout check
			ElseIf(in_enabled_inputs(j-1, 0) == 1.0f && in_enabled_outputs(k, 0) == 1.0f)

				Float d_k_y_j = 0.0f;
				ForInRange(m, 0, MINIBATCH_SIZE)
					Float d_k = in_sensitivities(k, m);
					Float y_j = in_inputs(j-1, m);
					d_k_y_j = d_k_y_j + (d_k * y_j);
				EndFor
				d_k_y_j = d_k_y_j * (1.0f / MINIBATCH_SIZE);

				out_weight_delta = MOMENTUM * in_prev_weight_deltas(j, k) + 
					(1.0f - MOMENTUM) * d_k_y_j;

				Float prev_weight = in_prev_weights(j, k);

				
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

			// not bias, not enabled so just copy old values over
			Else
				out_weight_delta = in_prev_weight_deltas(j, k);
				out_weight = in_prev_weights(j, k);
			EndIf

		END_MAIN
	END_SOURCE
};