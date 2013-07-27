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
			// calculate next values using Numerical recipes LCRG:  
			// http://en.wikipedia.org/wiki/Linear_congruential_generator  
			const uint32_t A = 1664525;
			const uint32_t C = 1013904223;
			out_seed = in_seeds(Index().X, Index().Y) * A + C;

			// bias is always enabled
			If(Index().X == 0 || ((Float)(DROPOUT_PROB * 4294967296.0f) < (Float)out_seed))
				out_state = 1;
			Else
				out_state = 0;
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
	int32_t VISIBLE_UNITS;

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
			const Int& m = Index().Y;	// what minibatch are we on
			const Int& j = Index().X;	// which hidden unit is this

			If(j == 0)	// bias is always on
				out_hidden = 1.0f;
			ElseIf(in_enabled_hidden(j, 0) == 0u)	// destination not enabled
				out_hidden = 0.0f;
			Else
				out_hidden = 0.0f;
			ForInRange(i, 0, VISIBLE_UNITS + 1)	// +1 for the bias
				Float& v_i = in_visible(i, m);
				Float& w_ij = in_weights(j, i);
				out_hidden = out_hidden +  (v_i * w_ij);
			EndFor
				out_hidden = ((Float)1.0f) / (Exp(-out_hidden) + 1.0f); 
			EndIf

		END_MAIN
	END_SOURCE
};

struct SourceCalcStates : public SiCKL::Source
{
	BEGIN_SOURCE
		BEGIN_CONST_DATA
		CONST_DATA(Buffer2D<UInt>, in_seeds)	
		CONST_DATA(Buffer2D<Float>, in_hidden)
		END_CONST_DATA

		BEGIN_OUT_DATA
		OUT_DATA(UInt, out_seed)
		OUT_DATA(Float, out_state)
		END_OUT_DATA

		BEGIN_MAIN
			// calculate next values using Numerical recipes LCRG:  
			// http://en.wikipedia.org/wiki/Linear_congruential_generator  
			const uint32_t A = 1664525;
			const uint32_t C = 1013904223;
			out_seed = in_seeds(Index().X, Index().Y) * A + C;

			//  and now calculate state
			If((in_hidden(Index().X, Index().Y) * 4294967296.0f) >= (Float)out_seed)
				out_state = 1.0f;
			Else
				out_state = 0.0f;
			EndIf
		END_MAIN
	END_SOURCE
};

struct SourceCalcVisible : public SiCKL::Source
{
	bool USE_SIGMOID;
	int32_t HIDDEN_UNITS;

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

			const Int& m = Index().Y;	// what minibatch are we on
			const Int& i = Index().X;	// which visible unit is this

			If(i == 0)	// bias is always on
				out_visible = 1.0f;
			ElseIf(in_enabled_visible(i, 0) == 0u)	// destination not enabled
				out_visible = 0.0f;
			Else
				out_visible = 0.0f;
			ForInRange(j, 0, HIDDEN_UNITS + 1)	// +1 for the bias
				Float& h_j = in_hidden(j, m);
				Float& w_ji = in_weights(j, i);
				out_visible = out_visible + (h_j * w_ji);
			EndFor
			// determined at shader compile time!
			if(USE_SIGMOID)
			{
				out_visible = ((Float)1.0f) / (Exp(-out_visible) + 1.0f); 
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
			Int& i = Index().Y;
			Int& j = Index().X;

			If(in_enabled_visible(i, 0) == 1u && in_enabled_hidden(j, 0) == 1u)

				// calculate the delta
				out_delta = 0.0f;
			ForInRange(m, 0, MINIBATCH_SIZE)
				Float vi = in_visible(i, m);
				Float vi_prime = in_visible_prime(i, m);

				Float hj = in_hidden(j, m);
				Float hj_prime = in_hidden_prime(j, m);

				out_delta = out_delta + (vi * hj);
				out_delta = out_delta - (vi_prime * hj_prime);
			EndFor
	
			out_delta = out_delta / (float)MINIBATCH_SIZE;

			out_delta = Float(1.0f - MOMENTUM) * out_delta + Float(MOMENTUM) * in_delta(Index().X, Index().Y);


			// get our previous weight
			Float prev = in_weight(Index().X, Index().Y);

			// calculate weight update
			// only L1
			if(L1_REGULARIZATION != 0.0f && L2_REGULARIZATION == 0.0f)
			{
				// factor comes out to 0 if we're on a bias, 1 if a weight
				Float factor = Clamp(Index().X, 0.0f, 1.0f) * Clamp(Index().Y, 0.0f, 1.0f);

				Float l1 = Sign(prev) * L1_REGULARIZATION;
				Float change = out_delta - l1 * factor;

				// update weight
				out_weight = prev + (change * LEARNING_RATE);				
			}
			// only L2
			else if(L1_REGULARIZATION == 0.0f && L2_REGULARIZATION != 0.0f)
			{
				// factor comes out to 0 if we're on a bias, 1 if a weight
				Float factor = Clamp(Index().X, 0.0f, 1.0f) * Clamp(Index().Y, 0.0f, 1.0f);

				Float l2 = prev * L2_REGULARIZATION;
				Float change = out_delta - l2 * factor;

				// update weight
				out_weight = prev + (change * LEARNING_RATE);
			} 
			// both L1 and L2
			else if(L1_REGULARIZATION != 0.0f && L2_REGULARIZATION != 0.0f)
			{
				// factor comes out to 0 if we're on a bias, 1 if a weight
				Float factor = Clamp(Index().X, 0.0f, 1.0f) * Clamp(Index().Y, 0.0f, 1.0f);

				Float l1 = Sign(prev) * L1_REGULARIZATION;
				Float l2 = prev * L2_REGULARIZATION;
				Float reg = l1 + l2;

				Float change = out_delta - (reg * factor);

				// update weight
				out_weight = prev + (change * LEARNING_RATE);
			}
			// neither
			else
			{
				// update weight
				out_weight = prev + (out_delta * LEARNING_RATE);
			}
			Else
				out_weight = in_weight(Index().X, Index().Y);
				out_delta = in_delta(Index().X, Index().Y);

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
				Float& v = in_visible(Index().X, k);
				Float& v_prime = in_visible_recon(Index().X, k);
				Float diff = v - v_prime;
				out_square_error = out_square_error + diff * diff;
			EndFor
		END_MAIN
	END_SOURCE
};