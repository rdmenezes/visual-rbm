struct __CalcEnabled_Source  : public SiCKL::Source
{
	// probability a given unit will be enabled
	float enabled_probability;

	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<UInt>, rands)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(UInt, state)
			OUT_DATA(UInt, next_rand)
		END_OUT_DATA

		BEGIN_MAIN
			// calculate next values using Numerical recipes LCRG:  
			// http://en.wikipedia.org/wiki/Linear_congruential_generator  
			const uint32_t A = 1664525;
			const uint32_t C = 1013904223;
			next_rand = rands(Index().X, Index().Y) * A + C;

			// bias is always enabled
			If(Index().X == 0 || ((Float)(enabled_probability * 4294967296.0f) >= (Float)next_rand))
				state = 1;
			Else
				state = 0;
			EndIf

		END_MAIN

	END_SOURCE
} CalcEnabled_Source;

OpenGLProgram* CalcEnabledVisible;
OpenGLProgram* CalcEnabledHidden;

struct __CopyVisible_Source : public SiCKL::Source
{
	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<Float>, source)
			CONST_DATA(Buffer2D<UInt>, enabled)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(Float, result)
		END_OUT_DATA

		BEGIN_MAIN
			If(enabled(Index().X, 0) == 1)
				result = source(Index().X, Index().Y);
			Else
				result = 0.0f;
			EndIf

		END_MAIN

	END_SOURCE
} CopyVisible_Source;

OpenGLProgram* CopyVisible;

struct __CalcHiddenProbability_Source : public SiCKL::Source
{
	int32_t visible_units;

	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<Float>, visible)
			CONST_DATA(Buffer2D<Float>, weights)
			CONST_DATA(Buffer2D<UInt>, hidden_enabled)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(Float, hidden_probability)
		END_OUT_DATA

		BEGIN_MAIN
			
			const Int& m = Index().Y;	// what minibatch are we on
			const Int& j = Index().X;	// which hidden unit is this

			If(j == 0)	// bias is always on
				hidden_probability = 1.0f;
			ElseIf(hidden_enabled(j, 0) == 0)	// destination not enabled
				hidden_probability = 0.0f;
			Else
				hidden_probability = 0.0f;
				ForInRange(i, 0, visible_units + 1)	// +1 for the bias
					Float& v_i = visible(i, m);
					Float& w_ij = weights(j, i);
					hidden_probability += v_i * w_ij;
				EndFor
				hidden_probability = ((Float)1.0f) / (Exp(-hidden_probability) + 1.0f); 
			EndIf

		END_MAIN

	END_SOURCE
} CalcHiddenProbability_Source;

OpenGLProgram* CalcHiddenProbability;

struct __CalcStates_Source : public SiCKL::Source
{
	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<UInt>, rands)
			CONST_DATA(Buffer2D<Float>, probabilities)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(UInt, next_rand)
			OUT_DATA(Float, state)
		END_OUT_DATA

		BEGIN_MAIN
			// calculate next values using Numerical recipes LCRG:  
			// http://en.wikipedia.org/wiki/Linear_congruential_generator  
			const uint32_t A = 1664525;
			const uint32_t C = 1013904223;
			next_rand = rands(Index().X, Index().Y) * A + C;
		
			//  and now calculate state
			If((probabilities(Index().X, Index().Y) * 4294967296.0f) >= (Float)next_rand)
				state = 1.0f;
			Else
				state = 0.0f;
			EndIf
		END_MAIN
	END_SOURCE
} CalcStates_Source;

OpenGLProgram* CalcStates;

struct __CalcVisible_Source : public SiCKL::Source
{
	bool use_sigmoid;
	int32_t hidden_units;

	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<Float>, hidden)
			CONST_DATA(Buffer2D<Float>, weights)
			CONST_DATA(Buffer2D<UInt>, visible_enabled)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(Float, visible)
		END_OUT_DATA

		BEGIN_MAIN

			const Int& m = Index().Y;	// what minibatch are we on
			const Int& i = Index().X;	// which visible unit is this

			If(i == 0)	// bias is always on
				visible = 1.0f;
			ElseIf(visible_enabled(i, 0) == 0)	// destination not enabled
				visible = 0.0f;
			Else
				visible = 0.0f;
				ForInRange(j, 0, hidden_units + 1)	// +1 for the bias
					Float& h_j = hidden(j, m);
					Float& w_ji = weights(j, i);
					visible += h_j * w_ji;
				EndFor
				// determined at shader compile time!
				if(use_sigmoid)
				{
					visible = ((Float)1.0f) / (Exp(-visible) + 1.0f); 
				}

			EndIf

		END_MAIN

	END_SOURCE

} CalcVisible_Source;

OpenGLProgram* CalcVisible;

struct __CalcWeightUpdates_Source : public SiCKL::Source
{
	/// compile time constants
	int32_t minibatch_size;
	float learning_rate;
	float momentum;
	float l1_regularization;
	float l2_regularization;

	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<Float>, visible)
			CONST_DATA(Buffer2D<Float>, hidden)
			CONST_DATA(Buffer2D<Float>, visible_prime)
			CONST_DATA(Buffer2D<Float>, hidden_prime)
			CONST_DATA(Buffer2D<Float>, prev_delta)
			CONST_DATA(Buffer2D<Float>, prev_weight)
			CONST_DATA(Buffer2D<UInt>, enabled_visible)
			CONST_DATA(Buffer2D<UInt>, enabled_hidden)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(Float, delta)
			OUT_DATA(Float, weight)
		END_OUT_DATA

		BEGIN_MAIN
			Int& i = Index().Y;
			Int& j = Index().X;

			If(enabled_visible(i, 0) == 1 && enabled_hidden(j, 0) == 1)

				// calculate the delta
				delta = 0.0f;
				ForInRange(m, 0, minibatch_size)
					Float vi = visible(i, m);
					Float vi_prime = visible_prime(i, m);

					Float hj = hidden(j, m);
					Float hj_prime = hidden_prime(j, m);

					delta = delta + (vi * hj);
					delta = delta - (vi_prime * hj_prime);
				EndFor
				delta /= (float)minibatch_size;

				delta = Float(1.0f - momentum) * delta + Float(momentum) * prev_delta(Index().X, Index().Y);
			
			
				// get our previous weight
				Float prev = prev_weight(Index().X, Index().Y);
			
				// calculate weight update
				// only L1
				if(l1_regularization != 0.0f && l2_regularization == 0.0f)
				{
					// factor comes out to 0 if we're on a bias, 1 if a weight
					Float factor = Clamp(Index().X, 0.0f, 1.0f) * Clamp(Index().Y, 0.0f, 1.0f);

					Float l1 = Sign(prev) * l1_regularization;
					Float change = delta - l1 * factor;

					// update weight
					weight = prev + (change * learning_rate);				
				}
				// only L2
				else if(l1_regularization == 0.0f && l2_regularization != 0.0f)
				{
					// factor comes out to 0 if we're on a bias, 1 if a weight
					Float factor = Clamp(Index().X, 0.0f, 1.0f) * Clamp(Index().Y, 0.0f, 1.0f);

					Float l2 = prev * l2_regularization;
					Float change = delta - l2 * factor;

					// update weight
					weight = prev + (change * learning_rate);
				} 
				// both L1 and L2
				else if(l1_regularization != 0.0f && l2_regularization != 0.0f)
				{
					// factor comes out to 0 if we're on a bias, 1 if a weight
					Float factor = Clamp(Index().X, 0.0f, 1.0f) * Clamp(Index().Y, 0.0f, 1.0f);

					Float l1 = Sign(prev) * l1_regularization;
					Float l2 = prev * l2_regularization;
					Float reg = l1 + l2;

					Float change = delta - (reg * factor);

					// update weight
					weight = prev + (change * learning_rate);
				}
				// neither
				else
				{
					// update weight
					weight = prev + (delta * learning_rate);
				}
			Else
				weight = prev_weight(Index().X, Index().Y);
				delta = prev_delta(Index().X, Index().Y);

			EndIf
		END_MAIN


	END_SOURCE
} CalcWeightUpdates_Source;

OpenGLProgram* CalcWeightUpdates;

struct __CalcErrorVector_Source : public SiCKL::Source
{
	int32_t minibatch_size;

	BEGIN_SOURCE
		BEGIN_CONST_DATA
			CONST_DATA(Buffer2D<Float>, visible)
			CONST_DATA(Buffer2D<Float>, visible_reconstruction)
		END_CONST_DATA

		BEGIN_OUT_DATA
			OUT_DATA(Float, mean_square_error)
		END_OUT_DATA

		BEGIN_MAIN
			mean_square_error = 0.0f;
			ForInRange(k, 0, minibatch_size)
				Float& v = visible(Index().X, k);
				Float& v_prime = visible_reconstruction(Index().X, k);
				Float diff = v - v_prime;
				mean_square_error = mean_square_error + diff * diff;
			EndFor
		END_MAIN


	END_SOURCE
} CalcErrorVector_Source;

OpenGLProgram* CalcErrorVector;