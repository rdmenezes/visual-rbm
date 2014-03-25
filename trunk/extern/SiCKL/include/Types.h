#pragma region Operators

/// boolean operators

#define EQUALS_OP(L, R)\
	BINARY_OP(Bool, L, R, Equal, ==)\
	BINARY_OP(Bool, L, R, NotEqual, !=)

#define COMPARISON_OP(L, R)\
	BINARY_OP(Bool, L, R, Less, <)\
	BINARY_OP(Bool, L, R, LessEqual, <=)\
	BINARY_OP(Bool, L, R, Greater, >)\
	BINARY_OP(Bool, L, R, GreaterEqual, >=)

/// math ops

#define NEGATION_OP(T)\
	UNARY_OP(T, T, UnaryMinus,-)

#define ADD_SUB_OP(Ret, L, R)\
	BINARY_OP(Ret, L, R, Add, +)\
	BINARY_OP(Ret, L, R, Subtract, -)

#define MULTIPLY_OP(Ret, L, R)\
	BINARY_OP(Ret, L, R, Multiply, *)

#define DIVIDE_OP(Ret, L, R)\
	BINARY_OP(Ret, L, R, Divide, /)

#define MODULO_OP(Ret, L, R)\
	BINARY_OP(Ret, L, R, Modulo, %)

/// bitwise ops

#define BITWISE_NOT_OP(T)\
	UNARY_OP(T, T, BitwiseNot, ~)

#define BITWISE_OP(Ret, L, R)\
	BINARY_OP(Ret, L, R, BitwiseAnd, &)\
	BINARY_OP(Ret, L, R, BitwiseOr, |)\
	BINARY_OP(Ret, L, R, BitwiseXor, ^)\
	BINARY_OP(Ret, L, R, LeftShift, <<)\
	BINARY_OP(Ret, L, R, RightShift, >>)

#pragma endregion

START_TYPE(Bool)
	CONSTRUCTOR_1(Bool, bool)
	CONSTRUCTOR_1(Bool, Bool)
	
	EQUALS_OP(bool, Bool)
	EQUALS_OP(Bool, bool)

	BINARY_OP(Bool, Bool, Bool, LogicalOr, ||)
	BINARY_OP(Bool, Bool, Bool, LogicalAnd, &&)
	UNARY_OP(Bool, Bool, LogicalNot, !)
END_TYPE

#pragma region Int Types

START_TYPE(Int)
	CONSTRUCTOR_1(Int, int32_t)
	CONSTRUCTOR_1(Int, Int)

	EQUALS_OP(int32_t, Int)
	EQUALS_OP(Int, int32_t)
	EQUALS_OP(Int, Int)
	COMPARISON_OP(int32_t, Int)
	COMPARISON_OP(Int, int32_t)
	COMPARISON_OP(Int, Int)

	NEGATION_OP(Int)
	ADD_SUB_OP(Int, Int, int32_t)
	ADD_SUB_OP(Int, int32_t, Int)
	ADD_SUB_OP(Int, Int, Int)
	
	MULTIPLY_OP(Int, Int, int32_t)
	MULTIPLY_OP(Int, int32_t, Int)
	MULTIPLY_OP(Int, Int, Int)

	DIVIDE_OP(Int, Int, int32_t)
	DIVIDE_OP(Int, int32_t, Int)
	DIVIDE_OP(Int, Int, Int)

	MODULO_OP(Int, Int, int32_t)
	MODULO_OP(Int, int32_t, Int)
	MODULO_OP(Int, Int, Int)

	BITWISE_NOT_OP(Int)
	BITWISE_OP(Int, Int, int32_t)
	BITWISE_OP(Int, int32_t, Int)
	BITWISE_OP(Int, Int, Int)

	CAST_OP(Int, Float)
	CAST_OP(Int, UInt)
END_TYPE

START_TYPE(Int2)
	CONSTRUCTOR_1(Int2, Int2)

	CONSTRUCTOR_2(Int2, int32_t, int32_t)
	CONSTRUCTOR_2(Int2, int32_t, Int)
	CONSTRUCTOR_2(Int2, Int, int32_t)
	CONSTRUCTOR_2(Int2, Int, Int)

	NEGATION_OP(Int2)
	ADD_SUB_OP(Int2, Int2, Int2)

	MULTIPLY_OP(Int2, Int2, Int)
	MULTIPLY_OP(Int2, Int, Int2)
	MULTIPLY_OP(Int2, Int2, int32_t)
	MULTIPLY_OP(Int2, int32_t, Int2)
	MULTIPLY_OP(Int2, Int2, Int2)
	MULTIPLY_OP(Float2, Int2, Float)
	MULTIPLY_OP(Float2, Float, Int2)
	MULTIPLY_OP(Float2, Int2, float)
	MULTIPLY_OP(Float2, float, Int2)

	DIVIDE_OP(Int2, Int2, Int)
	DIVIDE_OP(Int2, Int2, int32_t)
	DIVIDE_OP(Float2, Int2, Float)
	DIVIDE_OP(Float2, Int2, float)

	CAST_OP(Int2, Float2)
	CAST_OP(Int2, UInt2)

	START_MEMBERS(Int2)
		MEMBER_VAR(Int2, Int, X, 0);
		MEMBER_VAR(Int2, Int, Y, 1);
	END_MEMBERS
END_TYPE

START_TYPE(Int3)
	CONSTRUCTOR_1(Int3, Int3)

	CONSTRUCTOR_3(Int3, int32_t, int32_t, int32_t)
	CONSTRUCTOR_3(Int3, int32_t, int32_t, Int)
	CONSTRUCTOR_3(Int3, int32_t, Int, int32_t)
	CONSTRUCTOR_3(Int3, int32_t, Int, Int)
	CONSTRUCTOR_3(Int3, Int, int32_t, int32_t)
	CONSTRUCTOR_3(Int3, Int, int32_t, Int)
	CONSTRUCTOR_3(Int3, Int, Int, int32_t)
	CONSTRUCTOR_3(Int3, Int, Int, Int)

	NEGATION_OP(Int3)
	ADD_SUB_OP(Int3, Int3, Int3)
	
	MULTIPLY_OP(Int3, Int3, Int3)
	MULTIPLY_OP(Int3, Int3, Int)
	MULTIPLY_OP(Int3, Int, Int3)
	MULTIPLY_OP(Int3, Int3, int32_t)
	MULTIPLY_OP(Int3, int32_t, Int3)
	MULTIPLY_OP(Float3, Int3, Float)
	MULTIPLY_OP(Float3, Float, Int3)
	MULTIPLY_OP(Float3, Int3, float)
	MULTIPLY_OP(Float3, float, Int3)
	
	DIVIDE_OP(Int3, Int3, Int)
	DIVIDE_OP(Int3, Int3, int32_t)
	DIVIDE_OP(Float3, Int3, Float)
	DIVIDE_OP(Float3, Int3, float)

	CAST_OP(Int3, Float3)
	CAST_OP(Int3, UInt3)

	START_MEMBERS(Int3)
		MEMBER_VAR(Int3, Int, X, 0)
		MEMBER_VAR(Int3, Int, Y, 1)
		MEMBER_VAR(Int3, Int, Z, 2)
	END_MEMBERS
END_TYPE

START_TYPE(Int4)
	CONSTRUCTOR_1(Int4, Int4)

	CONSTRUCTOR_4(Int4, int32_t, int32_t, int32_t, int32_t)
	CONSTRUCTOR_4(Int4, int32_t, int32_t, int32_t, Int)
	CONSTRUCTOR_4(Int4, int32_t, int32_t, Int, int32_t)
	CONSTRUCTOR_4(Int4, int32_t, int32_t, Int, Int)
	CONSTRUCTOR_4(Int4, int32_t, Int, int32_t, int32_t)
	CONSTRUCTOR_4(Int4, int32_t, Int, int32_t, Int)
	CONSTRUCTOR_4(Int4, int32_t, Int, Int, int32_t)
	CONSTRUCTOR_4(Int4, int32_t, Int, Int, Int)
	CONSTRUCTOR_4(Int4, Int, int32_t, int32_t, int32_t)
	CONSTRUCTOR_4(Int4, Int, int32_t, int32_t, Int)
	CONSTRUCTOR_4(Int4, Int, int32_t, Int, int32_t)
	CONSTRUCTOR_4(Int4, Int, int32_t, Int, Int)
	CONSTRUCTOR_4(Int4, Int, Int, int32_t, int32_t)
	CONSTRUCTOR_4(Int4, Int, Int, int32_t, Int)
	CONSTRUCTOR_4(Int4, Int, Int, Int, int32_t)
	CONSTRUCTOR_4(Int4, Int, Int, Int, Int)

	NEGATION_OP(Int4)
	ADD_SUB_OP(Int4, Int4, Int4)
	
	MULTIPLY_OP(Int4, Int4, Int)
	MULTIPLY_OP(Int4, Int, Int4)
	MULTIPLY_OP(Int4, Int4, int32_t)
	MULTIPLY_OP(Int4, int32_t, Int4)
	MULTIPLY_OP(Int4, Int4, Int4)
	MULTIPLY_OP(Float4, Int4, Float)
	MULTIPLY_OP(Float4, Float, Int4)
	MULTIPLY_OP(Float4, Int4, float)
	MULTIPLY_OP(Float4, float, Int4)

	DIVIDE_OP(Int4, Int4, Int)
	DIVIDE_OP(Int4, Int4, int32_t)
	DIVIDE_OP(Float4, Int4, Float)
	DIVIDE_OP(Float4, Int4, float)

	CAST_OP(Int4, Float4)
	CAST_OP(Int4, UInt4)

	START_MEMBERS(Int4)
	MEMBER_VAR(Int4, Int, X, 0)
	MEMBER_VAR(Int4, Int, Y, 1)
	MEMBER_VAR(Int4, Int, Z, 2)
	MEMBER_VAR(Int4, Int, W, 3)
	END_MEMBERS
END_TYPE

#pragma endregion

#pragma region UInt Types

START_TYPE(UInt)
	CONSTRUCTOR_1(UInt, uint32_t)
	CONSTRUCTOR_1(UInt, UInt)

	EQUALS_OP(uint32_t, UInt)
	EQUALS_OP(UInt, uint32_t)
	EQUALS_OP(UInt, UInt)
	COMPARISON_OP(uint32_t, UInt)
	COMPARISON_OP(UInt, uint32_t)
	COMPARISON_OP(UInt, UInt)

	ADD_SUB_OP(UInt, UInt, uint32_t)
	ADD_SUB_OP(UInt, uint32_t, UInt)
	ADD_SUB_OP(UInt, UInt, UInt)

	MULTIPLY_OP(UInt, UInt, uint32_t)
	MULTIPLY_OP(UInt, uint32_t, UInt)
	MULTIPLY_OP(UInt, UInt, UInt)
	
	DIVIDE_OP(UInt, UInt, uint32_t)
	DIVIDE_OP(UInt, uint32_t, UInt)
	DIVIDE_OP(UInt, UInt, UInt)

	MODULO_OP(UInt, UInt, uint32_t)
	MODULO_OP(UInt, uint32_t, UInt)
	MODULO_OP(UInt, UInt, UInt)

	BITWISE_NOT_OP(UInt)
	BITWISE_OP(UInt, UInt, uint32_t)
	BITWISE_OP(UInt, uint32_t, UInt)
	BITWISE_OP(UInt, UInt, UInt)

	CAST_OP(UInt, Float)
	CAST_OP(UInt, Int)
END_TYPE

START_TYPE(UInt2)
	CONSTRUCTOR_1(UInt2, UInt2)

	CONSTRUCTOR_2(UInt2, uint32_t, uint32_t)
	CONSTRUCTOR_2(UInt2, uint32_t, UInt)
	CONSTRUCTOR_2(UInt2, UInt, uint32_t)
	CONSTRUCTOR_2(UInt2, UInt, UInt)

	ADD_SUB_OP(UInt2, UInt2, UInt2)
	
	MULTIPLY_OP(UInt2, UInt2, UInt)
	MULTIPLY_OP(UInt2, UInt, UInt2)
	MULTIPLY_OP(UInt2, UInt2, uint32_t)
	MULTIPLY_OP(UInt2, uint32_t, UInt2)
	MULTIPLY_OP(UInt2, UInt2, UInt2)
	MULTIPLY_OP(Float2, UInt2, Float)
	MULTIPLY_OP(Float2, Float, UInt2)
	MULTIPLY_OP(Float2, UInt2, float)
	MULTIPLY_OP(Float2, float, UInt2)

	CAST_OP(UInt2, Float2)
	CAST_OP(UInt2, Int2)

	START_MEMBERS(UInt2)
		MEMBER_VAR(UInt2, UInt, X, 0)
		MEMBER_VAR(UInt2, UInt, Y, 1)
	END_MEMBERS
END_TYPE

START_TYPE(UInt3)
	CONSTRUCTOR_1(UInt3, UInt3)

	CONSTRUCTOR_3(UInt3, uint32_t, uint32_t, uint32_t)
	CONSTRUCTOR_3(UInt3, uint32_t, uint32_t, UInt)
	CONSTRUCTOR_3(UInt3, uint32_t, UInt, uint32_t)
	CONSTRUCTOR_3(UInt3, uint32_t, UInt, UInt)
	CONSTRUCTOR_3(UInt3, UInt, uint32_t, uint32_t)
	CONSTRUCTOR_3(UInt3, UInt, uint32_t, UInt)
	CONSTRUCTOR_3(UInt3, UInt, UInt, uint32_t)
	CONSTRUCTOR_3(UInt3, UInt, UInt, UInt)

	ADD_SUB_OP(UInt3, UInt3, UInt3)
	
	MULTIPLY_OP(UInt3, UInt3, UInt3)
	MULTIPLY_OP(UInt3, UInt3, UInt)
	MULTIPLY_OP(UInt3, UInt, Int3)
	MULTIPLY_OP(UInt3, UInt3, uint32_t)
	MULTIPLY_OP(UInt3, uint32_t, UInt3)
	MULTIPLY_OP(Float3, UInt3, Float)
	MULTIPLY_OP(Float3, Float, UInt3)
	MULTIPLY_OP(Float3, UInt3, float)
	MULTIPLY_OP(Float3, float, UInt3)

	CAST_OP(UInt3, Float3)
	CAST_OP(UInt3, Int3)

	START_MEMBERS(UInt3)
		MEMBER_VAR(UInt3, UInt, X, 0)
		MEMBER_VAR(UInt3, UInt, Y, 1)
		MEMBER_VAR(UInt3, UInt, Z, 2)
	END_MEMBERS
END_TYPE

START_TYPE(UInt4)
	CONSTRUCTOR_1(UInt4, UInt4)

	CONSTRUCTOR_4(UInt4, uint32_t, uint32_t, uint32_t, uint32_t)
	CONSTRUCTOR_4(UInt4, uint32_t, uint32_t, uint32_t, UInt)
	CONSTRUCTOR_4(UInt4, uint32_t, uint32_t, UInt, uint32_t)
	CONSTRUCTOR_4(UInt4, uint32_t, uint32_t, UInt, UInt)
	CONSTRUCTOR_4(UInt4, uint32_t, UInt, uint32_t, uint32_t)
	CONSTRUCTOR_4(UInt4, uint32_t, UInt, uint32_t, UInt)
	CONSTRUCTOR_4(UInt4, uint32_t, UInt, UInt, uint32_t)
	CONSTRUCTOR_4(UInt4, uint32_t, UInt, UInt, UInt)
	CONSTRUCTOR_4(UInt4, UInt, uint32_t, uint32_t, uint32_t)
	CONSTRUCTOR_4(UInt4, UInt, uint32_t, uint32_t, UInt)
	CONSTRUCTOR_4(UInt4, UInt, uint32_t, UInt, uint32_t)
	CONSTRUCTOR_4(UInt4, UInt, uint32_t, UInt, UInt)
	CONSTRUCTOR_4(UInt4, UInt, UInt, uint32_t, uint32_t)
	CONSTRUCTOR_4(UInt4, UInt, UInt, uint32_t, UInt)
	CONSTRUCTOR_4(UInt4, UInt, UInt, UInt, uint32_t)
	CONSTRUCTOR_4(UInt4, UInt, UInt, UInt, UInt)

	ADD_SUB_OP(UInt4, UInt4, UInt4)

	MULTIPLY_OP(UInt4, UInt4, UInt)
	MULTIPLY_OP(UInt4, UInt, UInt4)
	MULTIPLY_OP(UInt4, UInt4, uint32_t)
	MULTIPLY_OP(UInt4, uint32_t, UInt4)
	MULTIPLY_OP(UInt4, UInt4, UInt4)
	MULTIPLY_OP(Float4, UInt4, Float)
	MULTIPLY_OP(Float4, Float, UInt4)
	MULTIPLY_OP(Float4, UInt4, float)
	MULTIPLY_OP(Float4, float, UInt4)

	DIVIDE_OP(UInt4, UInt4, UInt)
	DIVIDE_OP(UInt4, UInt4, uint32_t)
	DIVIDE_OP(Float4, UInt4, Float)
	DIVIDE_OP(Float4, UInt4, float)

	CAST_OP(UInt4, Float4)
	CAST_OP(UInt4, Int4)

	START_MEMBERS(UInt4)
	MEMBER_VAR(UInt4, UInt, X, 0)
	MEMBER_VAR(UInt4, UInt, Y, 1)
	MEMBER_VAR(UInt4, UInt, Z, 2)
	MEMBER_VAR(UInt4, UInt, W, 3)
	END_MEMBERS
END_TYPE

#pragma endregion

#pragma region Float Types

START_TYPE(Float)
	CONSTRUCTOR_1(Float, float)
	CONSTRUCTOR_1(Float, Float)

	EQUALS_OP(Float, float)
	EQUALS_OP(float, Float)
	EQUALS_OP( Float, Float)
	COMPARISON_OP(Float, float)
	COMPARISON_OP(float, Float)
	COMPARISON_OP(Float, Float)

	NEGATION_OP(Float)
	ADD_SUB_OP(Float, Float, float)
	ADD_SUB_OP(Float, float, Float)
	ADD_SUB_OP(Float, Float, Float)
	
	MULTIPLY_OP(Float, Float, Float)
	MULTIPLY_OP(Float, Float, float)
	MULTIPLY_OP(Float, float, Float)

	DIVIDE_OP(Float, Float, Float)
	DIVIDE_OP(Float, Float, float)
	DIVIDE_OP(Float, float, Float)

	CAST_OP(Float, Int)
	CAST_OP(Float, UInt)
END_TYPE

START_TYPE(Float2)
	CONSTRUCTOR_1(Float2, Float2)

	CONSTRUCTOR_2(Float2, float, float)
	CONSTRUCTOR_2(Float2, float, Float)
	CONSTRUCTOR_2(Float2, Float, float)
	CONSTRUCTOR_2(Float2, Float, Float)

	NEGATION_OP(Float2)
	ADD_SUB_OP(Float2, Float2, Float2)

	MULTIPLY_OP(Float2, Float2, Float2)
	MULTIPLY_OP(Float2, Float2, Float)
	MULTIPLY_OP(Float2, Float, Float2)
	MULTIPLY_OP(Float2, Float2, float)
	MULTIPLY_OP(Float2, float, Float2)
	
	DIVIDE_OP(Float2, Float2, Float)
	DIVIDE_OP(Float2, Float2, float)

	CAST_OP(Float2, Int2)
	CAST_OP(Float2, UInt2)

	START_MEMBERS(Float2)
	MEMBER_VAR(Float2, Float, X, 0);
	MEMBER_VAR(Float2, Float, Y, 1);
	END_MEMBERS
END_TYPE

START_TYPE(Float3)
	CONSTRUCTOR_1(Float3, Float3)

	CONSTRUCTOR_3(Float3, float, float, float)
	CONSTRUCTOR_3(Float3, float, float, Float)
	CONSTRUCTOR_3(Float3, float, Float, float)
	CONSTRUCTOR_3(Float3, float, Float, Float)
	CONSTRUCTOR_3(Float3, Float, float, float)
	CONSTRUCTOR_3(Float3, Float, float, Float)
	CONSTRUCTOR_3(Float3, Float, Float, float)
	CONSTRUCTOR_3(Float3, Float, Float, Float)

	NEGATION_OP(Float3)
	ADD_SUB_OP(Float3, Float3, Float3)
	
	MULTIPLY_OP(Float3, Float3, Float3)
	MULTIPLY_OP(Float3, Float3, Float)
	MULTIPLY_OP(Float3, Float, Float3)
	MULTIPLY_OP(Float3, Float3, float)
	MULTIPLY_OP(Float3, float, Float3)
	
	DIVIDE_OP(Float3, Float3, Float)
	DIVIDE_OP(Float3, Float3, float)

	CAST_OP(Float3, UInt3)
	CAST_OP(Float3, Int3)

	START_MEMBERS(Float3)
	MEMBER_VAR(Float3, Float, X, 0);
	MEMBER_VAR(Float3, Float, Y, 1);
	MEMBER_VAR(Float3, Float, Z, 2);
	END_MEMBERS
END_TYPE

START_TYPE(Float4)
	CONSTRUCTOR_1(Float4, Float4)

	CONSTRUCTOR_4(Float4, float, float, float, float)
	CONSTRUCTOR_4(Float4, float, float, float, Float)
	CONSTRUCTOR_4(Float4, float, float, Float, float)
	CONSTRUCTOR_4(Float4, float, float, Float, Float)
	CONSTRUCTOR_4(Float4, float, Float, float, float)
	CONSTRUCTOR_4(Float4, float, Float, float, Float)
	CONSTRUCTOR_4(Float4, float, Float, Float, float)
	CONSTRUCTOR_4(Float4, float, Float, Float, Float)
	CONSTRUCTOR_4(Float4, Float, float, float, float)
	CONSTRUCTOR_4(Float4, Float, float, float, Float)
	CONSTRUCTOR_4(Float4, Float, float, Float, float)
	CONSTRUCTOR_4(Float4, Float, float, Float, Float)
	CONSTRUCTOR_4(Float4, Float, Float, float, float)
	CONSTRUCTOR_4(Float4, Float, Float, float, Float)
	CONSTRUCTOR_4(Float4, Float, Float, Float, float)
	CONSTRUCTOR_4(Float4, Float, Float, Float, Float)

	NEGATION_OP(Float4)
	ADD_SUB_OP(Float4, Float4, Float4)
	
	MULTIPLY_OP(Float4, Float4, Float4)
	MULTIPLY_OP(Float4, Float4, Float)
	MULTIPLY_OP(Float4, Float4, float)

	DIVIDE_OP(Float4, Float4, Float)
	DIVIDE_OP(Float4, Float4, float)

	CAST_OP(Float4, Int4)
	CAST_OP(Float4, UInt4)

	START_MEMBERS(Float4)
	MEMBER_VAR(Float4, Float, X, 0)
	MEMBER_VAR(Float4, Float, Y, 1)
	MEMBER_VAR(Float4, Float, Z, 2)
	MEMBER_VAR(Float4, Float, W, 3)
	END_MEMBERS
END_TYPE

#pragma endregion