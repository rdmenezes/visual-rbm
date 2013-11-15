struct SourceCalcEnabled  : public SiCKL::Source
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

struct SourceCalcHiddenAndStates : public SiCKL::Source
{
	ActivationFunction_t FUNCTION;	
	int32_t VISIBLE_UNITS;
	float VISIBLE_DROPOUT_PROB;

	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<Float>, in_visible)
			CONST_DATA(Buffer2D<Float>, in_weights)
			CONST_DATA(Buffer2D<UInt>, in_enabled_hidden)
			CONST_DATA(Buffer2D<UInt>, in_seeds)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(UInt, out_seed)	
			OUT_DATA(Float, out_hidden)
			OUT_DATA(Float, out_state)
		END_OUT_DATA

		BEGIN_MAIN
			const Int m = Index().Y;	// what minibatch are we on
			const Int j = Index().X;	// which hidden unit is this

			UInt seed = in_seeds(Index().X, Index().Y);

			If(in_enabled_hidden(j, 0) == 0u)
				// destination is disabled
				out_hidden = 0.0f;
				out_state = 0.0f;
				out_seed = seed;
			Else
				Float accumulation = 0.0f;

				ForInRange(i, 0, VISIBLE_UNITS)
					Float v_i = in_visible(i, m);
					Float w_ij = in_weights(i+1, j+1);
					accumulation = accumulation +  (v_i * w_ij);
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
							noise = noise * Sqrt(Sigmoid(accumulation));
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
			CONST_DATA(Buffer2D<UInt>, in_enabled_visible)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(Float, out_visible)
		END_OUT_DATA
	
		BEGIN_MAIN
			const Int m = Index().Y;	// what minibatch are we on
			const Int i = Index().X;	// which visible unit is this

			If(in_enabled_visible(i, 0) == 0u)
				// destination is disabled
				out_visible = 0.0f;
			Else
				Float accumulation = 0.0f;

				ForInRange(j, 0, HIDDEN_UNITS)
					Float h_j = in_hidden(j, m);
					Float w_ij = in_weights(i+1, j+1);
					accumulation = accumulation + (h_j * w_ij);
				EndFor
				// take input dropout into account
				accumulation = accumulation  * (1.0f / (1.0f - HIDDEN_DROPOUT_PROB));
				// add bias
				accumulation = accumulation + in_weights(i+1, 0);

				switch(FUNCTION)
				{
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
			EndIf

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
			CONST_DATA(Buffer2D<UInt>, in_enabled_hidden)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(Float, out_hidden)
		END_OUT_DATA

		BEGIN_MAIN
			const Int m = Index().Y;	// what minibatch are we on
			const Int j = Index().X;	// which hidden unit is this

			If(in_enabled_hidden(j, 0) == 0u)
				// destination is disabled
				out_hidden = 0.0f;
			Else
				Float accumulation = 0.0f;

				ForInRange(i, 0, VISIBLE_UNITS)
					Float v_i = in_visible(i, m);
					Float w_ij = in_weights(i+1, j+1);
					accumulation = accumulation +  (v_i * w_ij);
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
			EndIf
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
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(Float, out_delta)
			OUT_DATA(Float, out_weight)
		END_OUT_DATA

		BEGIN_MAIN
			const Int i = Index().X;
			const Int j = Index().Y;


			const Float prev_delta = in_delta(i, j);
			const Float prev_weight = in_weight(i, j);

			If(i == 0 && j == 0)
				// top left corner, not a weight
				out_delta = 0.0f;
				out_weight = 0.0f;
			ElseIf(i == 0)
				// visible bias
				out_delta = 0.0f;
				ForInRange(m, 0, MINIBATCH_SIZE)
					const Float hj = in_hidden(j - 1, m);
					const Float hj_prime = in_hidden_prime(j - 1, m);

					out_delta = out_delta + (hj - hj_prime);
				EndFor
				out_delta = out_delta * (1.0f / (float)MINIBATCH_SIZE);
				out_weight = prev_weight + (out_delta * LEARNING_RATE);
			ElseIf(j == 0)
				// hidden bias
				out_delta = 0.0f;
				ForInRange(m, 0, MINIBATCH_SIZE)
					const Float vi = in_visible(i - 1, m);
					const Float vi_prime = in_visible_prime(i - 1, m);

					out_delta = out_delta + (vi - vi_prime);
				EndFor
				out_delta = out_delta * (1.0f / (float)MINIBATCH_SIZE);
				out_weight = prev_weight + (out_delta * LEARNING_RATE);
			ElseIf(in_enabled_visible(i - 1, 0) == 1u && in_enabled_hidden(j - 1, 0) == 1u)
				// regular weight
				out_delta = 0.0f; 
				ForInRange(m, 0, MINIBATCH_SIZE)
					Float vi = in_visible(i - 1, m);
					Float vi_prime = in_visible_prime(i - 1, m);

					Float hj = in_hidden(j - 1, m);
					Float hj_prime = in_hidden_prime(j - 1, m);

					out_delta = out_delta + (vi * hj);
					out_delta = out_delta - (vi_prime * hj_prime);
				EndFor
				out_delta = out_delta * (1.0f / (float)MINIBATCH_SIZE);
				out_delta = prev_delta * MOMENTUM + (1.0f - MOMENTUM) * out_delta;

				/// L1/L2 regularization
				
				// only L1
				if(L1_REGULARIZATION != 0.0f && L2_REGULARIZATION == 0.0f)
				{
					Float l1 = Sign(prev_weight) * L1_REGULARIZATION;
					Float change = out_delta - l1;

					// update weight
					out_weight = prev_weight + (change * LEARNING_RATE);				
				}
				// only L2
				else if(L1_REGULARIZATION == 0.0f && L2_REGULARIZATION != 0.0f)
				{
					Float l2 = prev_weight * L2_REGULARIZATION;
					Float change = out_delta - l2;

					// update weight
					out_weight = prev_weight + (change * LEARNING_RATE);
				} 
				// both L1 and L2
				else if(L1_REGULARIZATION != 0.0f && L2_REGULARIZATION != 0.0f)
				{
					Float l1 = Sign(prev_weight) * L1_REGULARIZATION;
					Float l2 = prev_weight * L2_REGULARIZATION;

					Float change = out_delta - l1 - l2;

					// update weight
					out_weight = prev_weight + (change * LEARNING_RATE);
				}
				// neither
				else
				{
					// update weight
					out_weight = prev_weight + (out_delta * LEARNING_RATE);
				}
			Else
				// unit is dropped out, do nothing
				out_delta = prev_delta;
				out_weight = prev_weight;
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
			out_square_error = 0.0f;
			ForInRange(k, 0, MINIBATCH_SIZE)
				const Float v = in_visible(Index().X, k);
				const Float v_prime = in_visible_recon(Index().X, k);
				const Float diff = v - v_prime;

				out_square_error = out_square_error + (diff * diff);
			EndFor
		END_MAIN
	END_SOURCE
};