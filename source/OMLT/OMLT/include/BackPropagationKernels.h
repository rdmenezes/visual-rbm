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

struct SourceFeedForward : public SiCKL::Source
{
	ActivationFunction_t FUNC;
	float INPUT_DROPOUT_PROB;
	uint32_t INPUT_COUNT;
	float NOISE_STDDEV;

	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<Float>, in_inputs)
			CONST_DATA(Buffer2D<Float>, in_enabled_inputs)
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

			// bias
			Float accumulation = 0.0f;
			// calculate dot product between feature and input vector
			ForInRange(i, 0, INPUT_COUNT)
				// get our input
				Float input = in_inputs(i, m);
				// get whether the input is enabled
				Float input_enabled = in_enabled_inputs(i, 0);
				// get weight, offset i by 1 because of bias column
				Float w_ij = in_weights(i + 1, j);

				accumulation = accumulation + (input * input_enabled * w_ij);
			EndFor
			// take input dropout and dropconnect into account
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

				
				Float partial = CalcActivationPrime(FUNC, in_activations(j, m));
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
	float ADADELTA_DECAY;

	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<Float>, in_sensitivities)
			CONST_DATA(Buffer2D<Float>, in_inputs)
			CONST_DATA(Buffer2D<Float>, in_enabled_inputs)
			CONST_DATA(Buffer2D<Float>, in_enabled_outputs)
			CONST_DATA(Buffer2D<Float>, in_prev_weights)
			CONST_DATA(Buffer2D<Float>, in_prev_weight_deltas)
			// mean square weight derivatives stored in X component
			// mean square weight updates stored in Y component
			CONST_DATA(Buffer2D<Float2>, in_prev_mean_square)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(Float, out_weight)
			OUT_DATA(Float, out_weight_delta)
			OUT_DATA(Float, out_nesterov_weight)
			OUT_DATA(Float2, out_mean_square)
		END_OUT_DATA

		BEGIN_MAIN
			Int& j = Index().X;
			Int& k = Index().Y;

			// epsilon for calculating Adadelta scaling factor
			const float eps = 1.0e-6f;

			// calculate weight derivatives
			Float delta_w = 0.0f;
			Bool dropped_out = false;
			// bias update
			If(j == 0 && in_enabled_outputs(k, 0) == 1.0f)

				Float d_k = 0.0f;
				ForInRange(m, 0, MINIBATCH_SIZE)
					d_k = d_k + in_sensitivities(k, m);
				EndFor
				delta_w = d_k;
			// weight update
			ElseIf(in_enabled_inputs(j-1, 0) == 1.0f && in_enabled_outputs(k, 0) == 1.0f)

				Float d_k_y_j = 0.0f;
				ForInRange(m, 0, MINIBATCH_SIZE)
					Float d_k = in_sensitivities(k, m);
					Float y_j = in_inputs(j-1, m);
					d_k_y_j = d_k_y_j + (d_k * y_j);
				EndFor
				delta_w = d_k_y_j;
			// no update because dropout
			Else
				dropped_out = true;
			EndIf

			If(dropped_out == false)

				delta_w = delta_w * (1.0f / float(MINIBATCH_SIZE));

				// calculate mean square weight derivative
				auto& out_mean_square_derivative = out_mean_square.X;
				auto& prev_mean_square_derivative = in_prev_mean_square(Index()).X;
			
				out_mean_square_derivative = (1.0f - ADADELTA_DECAY) * (delta_w*delta_w) + ADADELTA_DECAY * prev_mean_square_derivative;

				// calculate weight decay
				auto& prev_weight = in_prev_weights(Index());
				Float weight_decay = 0.0f;
				{
					// Regularization logic (we don't want to do weight decay on biases)
					// only L1
					if(L1_REGULARIZATION != 0.0f && L2_REGULARIZATION == 0.0f)
					{
						Float l1 = Sign(prev_weight) * L1_REGULARIZATION;
						// j == 0 for bias, so Sign(j) == 0 which negates weight decay term
						weight_decay = l1 * (Float)Sign(j);
					}
					// only L2
					else if(L1_REGULARIZATION == 0.0f && L2_REGULARIZATION != 0.0f)
					{
						Float l2 = prev_weight * L2_REGULARIZATION;
						weight_decay = l2 * (Float)Sign(j);
					}
					// both L1 and L2
					else if(L1_REGULARIZATION != 0.0f && L2_REGULARIZATION != 0.0f)
					{
						Float l1 = Sign(prev_weight) * L1_REGULARIZATION;
						Float l2 = prev_weight * L2_REGULARIZATION;

						weight_decay = (l1 + l2) * (Float)Sign(j);
					}
				}

				// calculate weight delta
				auto& prev_weight_delta = in_prev_weight_deltas(Index());
				auto& prev_mean_square_delta = in_prev_mean_square(Index()).Y;
			
				Float adadelta_factor = Sqrt(prev_mean_square_delta + eps) / Sqrt(out_mean_square_derivative + eps);

				out_weight_delta = LEARNING_RATE * adadelta_factor * (delta_w - weight_decay);

				// calculate mean square weight delta
				auto& out_mean_square_delta = out_mean_square.Y;
			
				out_mean_square_delta = (1.0f - ADADELTA_DECAY) * (out_weight_delta * out_weight_delta) + ADADELTA_DECAY * prev_mean_square_delta;

				// update the weight
				out_weight = prev_weight + out_weight_delta;

				// calculate the weight for t + 1/2 for nesterov momentum
				out_nesterov_weight = out_weight + MOMENTUM * out_weight_delta;
			Else
				out_weight = in_prev_weights(Index());
				out_weight_delta = in_prev_weight_deltas(Index());
				out_mean_square = in_prev_mean_square(Index());
				out_nesterov_weight = out_weight + MOMENTUM * out_weight_delta;
			EndIf
		END_MAIN
	END_SOURCE
};