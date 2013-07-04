#pragma once

#include <stdint.h>
namespace SiCKL
{
	typedef int32_t symbol_id_t;
	typedef int32_t member_id_t;
	const symbol_id_t invalid_symbol = -1;
	const symbol_id_t temp_symbol = -2;
	const symbol_id_t member_symbol = -3;

	struct NodeType
	{
		enum Type
		{
			Invalid = -1,
			// Flow Control
			Program,
			ConstData,
			OutData,
			Main,
			Block,
			If,
			ElseIf,
			Else,
			While,
			ForInRange,
			// Variable Declaration
			OutVar,
			ConstVar,
			Var,
			Literal,
			// Binary Operators
			Assignment,
			// Comparison
			Equal,
			NotEqual,
			Greater,
			GreaterEqual,
			Less,
			LessEqual,

			LogicalAnd,
			LogicalOr,
			LogicalNot,

			BitwiseAnd,
			BitwiseOr,
			BitwiseXor,
			BitwiseNot,

			// Arithmetic 
			UnaryMinus,
			Add,
			Subtract,
			Multiply,
			Divide,
			Modulo,
			// Functions
			Constructor,
			Cast,
			Function,

			Sample1D,// sample from 1D buffer
			Sample2D,// sample from 2d buffer
			Member,// member variable access
			GetIndex,// get the index we're working on
			GetNormalizedIndex,// get normalized index we're working on
		};
	};
	// each type is denoted by a bit
	struct ReturnType
	{
		enum Type
		{
#			define SHIFT(X) (1 << X)
			Invalid = -1,
			Void = SHIFT(0),
			Bool = SHIFT(1),
			Int = SHIFT(2),
			UInt = SHIFT(3),
			Float = SHIFT(4),
			Int2 = SHIFT(5),
			UInt2 = SHIFT(6),
			Float2 = SHIFT(7),
			Int3 = SHIFT(8),
			UInt3 = SHIFT(9),
			Float3 = SHIFT(10),
			Int4 = SHIFT(11),
			UInt4 = SHIFT(12),
			Float4 = SHIFT(13),

			// Buffer types
			Buffer1D = SHIFT(30),
			Buffer2D = SHIFT(31),
#			undef SHIFT
		};
	};

	struct BuiltinFunction
	{
		enum Func
		{
			Invalid = -1,
			// info
			Index,
			NormalizedIndex,
			// trigonometry
			Sin,
			Cos,
			Tan,
			ASin,
			ACos,
			ATan,
			SinH,
			CosH,
			TanH,
			ASinH,
			ACosH,
			ATanH,
			// exponential functions
			Pow,
			Exp,
			Log,
			Exp2,
			Log2,
			Sqrt,
			// common
			Abs,
			Sign,
			Floor,
			Ceiling,
			Min,
			Max,
			Clamp,
			IsNan,
			IsInf,
			// vector math
			Length,
			Distance,
			Dot,
			Cross,
			Normalize,
		};
	};
}