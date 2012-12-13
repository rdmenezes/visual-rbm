using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;

public enum UnitType : byte
{
	Sigmoid = 0x00,
	Linear = 0xFF
}

unsafe class Memory
{
    public static void* malloc(Int32 size)
    {
        IntPtr ip = Marshal.AllocHGlobal(size);
        return ip.ToPointer();
    }

    public static void free(void* ptr)
    {
        Debug.Assert(ptr != null);
        Marshal.FreeHGlobal(new IntPtr(ptr));
    }
}

unsafe public class Sigmoid
{
	const float _max = 15.0f;
	const float _min = -_max;

	const int _count = 2048;
	const float _scale = (_count - 1) / _max;

    static float* _pos_vals;
    static float* _neg_vals;

	static Sigmoid()
	{
        _pos_vals = (float*)Memory.malloc(sizeof(float) * _count);
        _neg_vals = (float*)Memory.malloc(sizeof(float) * _count);

        const double _dscale = ((_count - 1) / (double)_max);

		for (int k = 0; k < _count; k++)
		{
			double x = k;
			_pos_vals[k] = (float)(1.0 / (1.0 + Math.Exp(-x / _dscale)));
			_neg_vals[k] = (float)(1.0 / (1.0 + Math.Exp(x / _dscale)));
		}
	}

	public static float Calc(float x)
	{
		if (x <= _min) return 0.0f;
		if (x >= _max) return 1.0f;

		float result = 0.5f;

		if (x > 0.0f)
		{
			result = _pos_vals[(int)(x * _scale + 0.5f)];
		}
		else if (x < 0.0f)
		{
			result = _neg_vals[(int)(-x * _scale + 0.5f)];
		}

		return result;
	}

	~Sigmoid()
	{
        Memory.free(_pos_vals);
        Memory.free(_neg_vals);
	}
}

// use same parmeters as Sigmoid since this is actually the integral of Sigmoid
unsafe public class LnOnePlusExp
{
	const float _max = 15.0f;
	const float _min = -_max;

	const int _count = 2048;
	const float _scale = (_count - 1) / _max;

	static float* _pos_vals;
	static float* _neg_vals;

	static LnOnePlusExp()
	{
		_pos_vals = (float*)Memory.malloc(sizeof(float) * _count);
		_neg_vals = (float*)Memory.malloc(sizeof(float) * _count);

		const double _dscale = ((_count - 1) / (double)_max);

		for (int k = 0; k < _count; k++)
		{
			double x = k;
			_pos_vals[k] = (float)Math.Log(1.0 + Math.Exp(x / _dscale));
			_neg_vals[k] = (float)Math.Log(1.0 + Math.Exp(-x / _dscale));
		}
	}

	public static float Calc(float x)
	{
		if (x <= _min) return 0.0f;
		if (x >= _max) return x;

		// ln(1 + e^x)  == ln(2)
		float result = 0.6931471805599453f;

		if (x > 0.0f)
		{
			result = _pos_vals[(int)(x * _scale + 0.5f)];	
		}
		else if (x < 0.0f)
		{
			result = _neg_vals[(int)(-x * _scale + 0.5f)];
		}

		return result;
	}

	~LnOnePlusExp()
	{
		Memory.free(_pos_vals);
		Memory.free(_neg_vals);
	}
}

unsafe public class RBM
{
	static Endianness _system_endianness = BitConverter.IsLittleEndian ? Endianness.LittleEndian : Endianness.BigEndian;

	ushort _visible;
	ushort _hidden;

	UnitType _visible_type;

    float* _visible_means;
    float* _visible_stddevs;

    float* _visible_biases;
    float* _hidden_biases;

    float** _visible_features;
    float** _hidden_features;

    uint* _random;

	Stream _rbm_stream;

	#region READS

	byte ReadUInt8()
	{
		int result = _rbm_stream.ReadByte();
		Debug.Assert(result >= 0 && result <= 255);
		return (byte)result;
	}

	unsafe ushort ReadUInt16()
	{
		byte* buff = stackalloc byte[2];
		ushort* result = (ushort*)buff;

		if (_system_endianness == Endianness.LittleEndian)
		{
			buff[0] = ReadUInt8();
			buff[1] = ReadUInt8();
		}
		else
		{
			buff[1] = ReadUInt8();
			buff[0] = ReadUInt8();
		}

		return *result;
	}

	unsafe void ReadSingle(float* dest, int count)
	{
		byte* buff = stackalloc byte[2];
		float* result = (float*)buff;

		if (_system_endianness == Endianness.LittleEndian)
		{
            for (int k = 0; k < count; k++)
			{
				buff[0] = ReadUInt8();
				buff[1] = ReadUInt8();
				buff[2] = ReadUInt8();
				buff[3] = ReadUInt8();

				dest[k] = *result;
			}
		}
		else
		{
            for (int k = 0; k < count; k++)
			{
				buff[3] = ReadUInt8();
				buff[2] = ReadUInt8();
				buff[1] = ReadUInt8();
				buff[0] = ReadUInt8();

				dest[k] = *result;
			}
		}
	}

	#endregion

	private RBM()
	{

	}

    ~RBM()
    {
        if (_visible_type == UnitType.Linear)
        {
            Memory.free(_visible_means);
            Memory.free(_visible_stddevs);
        }

        Memory.free(_visible_biases);
        Memory.free(_hidden_biases);

        for (int i = 0; i < _visible; i++)
        {
            Memory.free(_hidden_features[i]);
        }
        Memory.free(_hidden_features);

        for(int j = 0; j < _hidden; j++)
        {
            Memory.free(_visible_features[j]);
        }
        Memory.free(_visible_features);

        Memory.free(_random);
    }

	public static RBM Load(Stream in_stream)
{	
		RBM result = new RBM();
		result._rbm_stream = in_stream;

		byte[] dot_rbm = new byte[4];
		dot_rbm[0] = result.ReadUInt8();
		dot_rbm[1] = result.ReadUInt8();
		dot_rbm[2] = result.ReadUInt8();
		dot_rbm[3] = result.ReadUInt8();

		if (System.Text.Encoding.ASCII.GetString(dot_rbm) != ".RBM")
		{
			return null;
		}

		// get the visible type
		result._visible_type = (UnitType)result.ReadUInt8();
		// read counts
		result._visible = result.ReadUInt16();
		result._hidden = result.ReadUInt16();
		// read stats
		if (result._visible_type == UnitType.Linear)
		{
            result._visible_means = (float*)Memory.malloc(sizeof(float) * result._visible);
			result.ReadSingle(result._visible_means, result._visible);
            result._visible_stddevs = (float*)Memory.malloc(sizeof(float) * result._visible);
			result.ReadSingle(result._visible_stddevs, result._visible);
		}
		// read biases
        result._visible_biases = (float*)Memory.malloc(sizeof(float) * result._visible);
		result.ReadSingle(result._visible_biases, result._visible);
        result._hidden_biases = (float*)Memory.malloc(sizeof(float) * result._hidden);
		result.ReadSingle(result._hidden_biases, result._hidden);

		// get the weights weights

        result._hidden_features = (float**)Memory.malloc(sizeof(float*) * result._visible);
		for (uint i = 0; i < result._visible; i++)
		{
			result._hidden_features[i] = (float*)Memory.malloc(sizeof(float) * result._hidden);
			result.ReadSingle(result._hidden_features[i], result._hidden);
		}

		// now transpose
		result._visible_features = (float**)Memory.malloc(sizeof(float*) * result._hidden);
		for (uint j = 0; j < result._hidden; j++)
		{
			result._visible_features[j] = (float*)Memory.malloc(sizeof(float) * result._visible);
			for (uint i = 0; i < result._visible; i++)
			{
				result._visible_features[j][i] = result._hidden_features[i][j];
			}
		}

        // get seeds for our random buffer
        result._random = (uint*)Memory.malloc(sizeof(int) * result._hidden);
        Random rand = new Random();
        for (int j = 0; j < result._hidden; j++)
        {
            result._random[j] = (uint)rand.Next();
        }

		return result;
	}

	public void CalcHiddenActivations(float[] in_visible_vector, float[] out_hidden)
	{
        // figure out how to fix these vectors

		Debug.Assert(in_visible_vector.Length == _visible);
		Debug.Assert(out_hidden.Length == _hidden);

		fixed (float* v = in_visible_vector, h = out_hidden)
		{
			for (uint j = 0; j < _hidden; j++)
			{
				h[j] = _hidden_biases[j];
				for (uint i = 0; i < _visible; i++)
				{
					h[j] += _visible_features[j][i] * v[i];
				}
			}
		}
	}

	public void CalcHiddenProbabilities(float[] in_visible_vector, float[] out_hidden)
	{
		fixed (float* v = in_visible_vector, h = out_hidden)
		{
			for (uint j = 0; j < _hidden; j++)
			{
				h[j] = _hidden_biases[j];
				for (uint i = 0; i < _visible; i++)
				{
					h[j] += _visible_features[j][i] * v[i];
				}
				h[j] = Sigmoid.Calc(h[j]);
			}
		}
    }

	public void CalcHiddenStates(float[] in_visible_vector, float[] out_hidden)
	{
		// val for our prng
		// http://en.wikipedia.org/wiki/Linear_congruential_generator (Numerical Recipes vals)
        const uint a = 1664525;
        const uint c = 1013904223;

		fixed (float* v = in_visible_vector, h = out_hidden)
		{
			for (uint j = 0; j < _hidden; j++)
			{
				// activation
				h[j] = _hidden_biases[j];
				for (uint i = 0; i < _visible; i++)
				{
					h[j] += _visible_features[j][i] * v[i];
				}
				// probability
				h[j] = Sigmoid.Calc(h[j]);
				_random[j] = _random[j] * a + c;
				// state
				h[j] = (_random[j] < h[j] * uint.MaxValue) ? 1.0f : 0.0f;
			}
		}
	}

    public void CalcVisible(float[] in_hidden, float[] out_visible)
    {
		fixed (float* h = in_hidden, v = out_visible)
		{
			for (int i = 0; i < _visible; i++)
			{
				v[i] = _visible_biases[i];
				for (int j = 0; j < _hidden; j++)
				{
					v[i] += _hidden_features[i][j] * h[j];
				}
			}

			if (_visible_type == UnitType.Sigmoid)
			{
				for (int i = 0; i < _visible; i++)
				{
					v[i] = Sigmoid.Calc(v[i]);
				}
			}
		}
    }

    public float CalcFreeEnergy(float[] in_visible)
    {
        // equation 25 in Practical Guide

		float F_v = 0.0f;

		fixed (float* v = in_visible)
		{
			if (_visible_type == UnitType.Sigmoid)
			{
				// http://www.cs.toronto.edu/~hinton/absps/guideTR.pdf equation 25
				for (int i = 0; i < _visible; i++)
				{
					F_v -= _visible_biases[i] * v[i];
				}

				for (int j = 0; j < _hidden; j++)
				{
					float x_j = _hidden_biases[j];
					for (int i = 0; i < _visible; i++)
					{
						x_j += _visible_features[j][i] * v[i];
					}

					// lookup table similar to Sigmoid
					F_v -= LnOnePlusExp.Calc(x_j);
				}
			}
			else if (_visible_type == UnitType.Linear)
			{
				// http://www.cs.toronto.edu/~hinton/absps/fmrinips.pdf equation 4
				for (int i = 0; i < _visible; i++)
				{
					float diff =  _visible_biases[i] * v[i];
					F_v += diff * diff;
				}
				F_v *= 0.5f;

				for (int j = 0; j < _hidden; j++)
				{
					float x_j = _hidden_biases[j];
					for (int i = 0; i < _visible; i++)
					{
						x_j += _visible_features[j][i] * v[i];
					}

					// lookup table similar to Sigmoid
					x_j = LnOnePlusExp.Calc(x_j);
					F_v -= x_j;
				}
			}

		}
		return F_v;
    }

	public void NormalizeVisibleVector(float[] in_out_visible)
	{
		fixed (float* v = in_out_visible)
		{
			for(int i = 0; i < _visible; i++)
			{
				v[i] = (v[i] - _visible_means[i]) / _visible_stddevs[i];
			}
		}
	}

	public float GetVisibleMean(int i)
	{
		return _visible_means[i];
	}

	public float GetVisibleStddev(int i)
	{
		return _visible_stddevs[i];
	}

	public uint GetVisibleCount()
	{
		return _visible;
	}

	public uint GetHiddenCount()
	{
		return _hidden;
	}
}

