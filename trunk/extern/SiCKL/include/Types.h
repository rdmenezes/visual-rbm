struct Bool : public Data
{
	DEFINE_DATA(Bool, ReturnType::Bool)
	
	DEFINE_ASSIGN(bool)
	DEFINE_ASSIGN(Bool)
	DEFINE_EQUALS_OPERATORS(bool)
	DEFINE_EQUALS_OPERATORS(Bool)

	DEFINE_BINARY_OPERATOR(Bool, Bool, LogicalOr, ||)
	DEFINE_BINARY_OPERATOR(Bool, Bool, LogicalAnd, &&)
	DEFINE_UNARY_OPERATOR(Bool, LogicalNot, !)
};

#pragma region Int Types

struct Int : public Data
{
	DEFINE_DATA(Int, ReturnType::Int)

	DEFINE_ASSIGN(int32_t)
	DEFINE_ASSIGN(Int)

	DEFINE_EQUALS_OPERATORS(int32_t)
	DEFINE_EQUALS_OPERATORS(Int)
	DEFINE_COMPARISON_OPERATORS(int32_t)
	DEFINE_COMPARISON_OPERATORS(Int)

	DEFINE_ADDITION_SUBTRACTION(Int, int32_t)
	DEFINE_ADDITION_SUBTRACTION(Int, Int)
	DEFINE_ADDITION_SUBTRACTION(Float, float)
	DEFINE_ADDITION_SUBTRACTION(Float, Float)
	DEFINE_NEGATION
	DEFINE_MULTIPLICATION(Int, int32_t)
	DEFINE_MULTIPLICATION(Int, Int)
	DEFINE_MULTIPLICATION(Float, float)
	DEFINE_MULTIPLICATION(Float, Float)
	DEFINE_DIVISION(Int, int32_t)
	DEFINE_DIVISION(Int, Int)
	DEFINE_DIVISION(Float, float)
	DEFINE_DIVISION(Float, Float)
	DEFINE_MODULO(Int, int32_t)
	DEFINE_MODULO(Int, Int)

	DEFINE_BITWISE_NOT
	DEFINE_BITWISE_OPERATORS(Int, int32_t)
	DEFINE_BITWISE_OPERATORS(Int, Int)

	DEFINE_CAST(Float)
	DEFINE_CAST(UInt)
};

struct Int2 : public Data
{
	DEFINE_DATA(Int2, ReturnType::Int2)
	DEFINE_ASSIGN(Int2)

	DEFINE_CONSTRUCTOR_2(int32_t, int32_t)
	DEFINE_CONSTRUCTOR_2(int32_t, Int)
	DEFINE_CONSTRUCTOR_2(Int, int32_t)
	DEFINE_CONSTRUCTOR_2(Int, Int)

	DEFINE_ADDITION_SUBTRACTION(Int2, Int2)
	DEFINE_NEGATION
	DEFINE_MULTIPLICATION(Int2, Int2)
	DEFINE_MULTIPLICATION(Int2, Int)
	DEFINE_MULTIPLICATION(Int2, int32_t)
	DEFINE_MULTIPLICATION(Float2, Float)
	DEFINE_MULTIPLICATION(Float2, float)
	DEFINE_DIVISION(Int2, Int)
	DEFINE_DIVISION(Int2, int32_t)
	DEFINE_DIVISION(Float2, Float)
	DEFINE_DIVISION(Float2, float)

	DEFINE_CAST(Float2)
	DEFINE_CAST(UInt2)

	DECLARE_MEMBER_TABLE(Int2)
	DECLARE_MEMBER_VAR(Int, X);
	DECLARE_MEMBER_VAR(Int, Y);
};

struct Int3 : public Data
{
	DEFINE_DATA(Int3, ReturnType::Int3)
	DEFINE_ASSIGN(Int3)

	DEFINE_CONSTRUCTOR_3(int32_t, int32_t, int32_t)
	DEFINE_CONSTRUCTOR_3(int32_t, int32_t, Int)
	DEFINE_CONSTRUCTOR_3(int32_t, Int, int32_t)
	DEFINE_CONSTRUCTOR_3(int32_t, Int, Int)
	DEFINE_CONSTRUCTOR_3(Int, int32_t, int32_t)
	DEFINE_CONSTRUCTOR_3(Int, int32_t, Int)
	DEFINE_CONSTRUCTOR_3(Int, Int, int32_t)
	DEFINE_CONSTRUCTOR_3(Int, Int, Int)

	DEFINE_ADDITION_SUBTRACTION(Int3, Int3)
	DEFINE_NEGATION
	DEFINE_MULTIPLICATION(Int3, Int3)
	DEFINE_MULTIPLICATION(Int3, Int)
	DEFINE_MULTIPLICATION(Int3, int32_t)
	DEFINE_MULTIPLICATION(Float3, Float)
	DEFINE_MULTIPLICATION(Float3, float)
	DEFINE_DIVISION(Int3, Int)
	DEFINE_DIVISION(Int3, int32_t)
	DEFINE_DIVISION(Float3, Float)
	DEFINE_DIVISION(Float3, float)

	DEFINE_CAST(Float3)
	DEFINE_CAST(UInt3)

	DECLARE_MEMBER_TABLE(Int3)
	DECLARE_MEMBER_VAR(Int, X)
	DECLARE_MEMBER_VAR(Int, Y)
	DECLARE_MEMBER_VAR(Int, Z)
};

struct Int4 : public Data
{
	DEFINE_DATA(Int4, ReturnType::Int4)
	DEFINE_ASSIGN(Int4)

	DEFINE_CONSTRUCTOR_4(int32_t, int32_t, int32_t, int32_t)
	DEFINE_CONSTRUCTOR_4(int32_t, int32_t, int32_t, Int)
	DEFINE_CONSTRUCTOR_4(int32_t, int32_t, Int, int32_t)
	DEFINE_CONSTRUCTOR_4(int32_t, int32_t, Int, Int)
	DEFINE_CONSTRUCTOR_4(int32_t, Int, int32_t, int32_t)
	DEFINE_CONSTRUCTOR_4(int32_t, Int, int32_t, Int)
	DEFINE_CONSTRUCTOR_4(int32_t, Int, Int, int32_t)
	DEFINE_CONSTRUCTOR_4(int32_t, Int, Int, Int)

	DEFINE_CONSTRUCTOR_4(Int, int32_t, int32_t, int32_t)
	DEFINE_CONSTRUCTOR_4(Int, int32_t, int32_t, Int)
	DEFINE_CONSTRUCTOR_4(Int, int32_t, Int, int32_t)
	DEFINE_CONSTRUCTOR_4(Int, int32_t, Int, Int)
	DEFINE_CONSTRUCTOR_4(Int, Int, int32_t, int32_t)
	DEFINE_CONSTRUCTOR_4(Int, Int, int32_t, Int)
	DEFINE_CONSTRUCTOR_4(Int, Int, Int, int32_t)
	DEFINE_CONSTRUCTOR_4(Int, Int, Int, Int)

	DEFINE_ADDITION_SUBTRACTION(Int4, Int4)
	DEFINE_NEGATION
	DEFINE_MULTIPLICATION(Int4, Int4)
	DEFINE_MULTIPLICATION(Int4, Int)
	DEFINE_MULTIPLICATION(Int4, int32_t)
	DEFINE_MULTIPLICATION(Float4, Float)
	DEFINE_MULTIPLICATION(Float4, float)
	DEFINE_DIVISION(Int4, Int)
	DEFINE_DIVISION(Int4, int32_t)
	DEFINE_DIVISION(Float4, Float)
	DEFINE_DIVISION(Float4, float)

	DEFINE_CAST(Float4)
	DEFINE_CAST(UInt4)

	DECLARE_MEMBER_TABLE(Int4)
	DECLARE_MEMBER_VAR(Int, X)
	DECLARE_MEMBER_VAR(Int, Y)
	DECLARE_MEMBER_VAR(Int, Z)
	DECLARE_MEMBER_VAR(Int, W)
};

#pragma endregion

#pragma region UInt Types

struct UInt : public Data
{
	DEFINE_DATA(UInt, ReturnType::UInt)

	DEFINE_ASSIGN(uint32_t)
	DEFINE_ASSIGN(UInt)

	DEFINE_EQUALS_OPERATORS(uint32_t)
	DEFINE_EQUALS_OPERATORS(UInt)
	DEFINE_COMPARISON_OPERATORS(uint32_t)
	DEFINE_COMPARISON_OPERATORS(UInt)

	DEFINE_ADDITION_SUBTRACTION(UInt, uint32_t)
	DEFINE_ADDITION_SUBTRACTION(UInt, UInt)
	DEFINE_ADDITION_SUBTRACTION(Float, float)
	DEFINE_ADDITION_SUBTRACTION(Float, Float)
	DEFINE_MULTIPLICATION(UInt, uint32_t)
	DEFINE_MULTIPLICATION(UInt, UInt)
	DEFINE_MULTIPLICATION(Float, float)
	DEFINE_MULTIPLICATION(Float, Float)
	DEFINE_DIVISION(UInt, uint32_t)
	DEFINE_DIVISION(UInt, UInt)
	DEFINE_DIVISION(Float, float)
	DEFINE_DIVISION(Float, Float)
	DEFINE_MODULO(UInt, uint32_t)
	DEFINE_MODULO(UInt, UInt)

	DEFINE_BITWISE_NOT
	DEFINE_BITWISE_OPERATORS(UInt, uint32_t)
	DEFINE_BITWISE_OPERATORS(UInt, UInt)

	DEFINE_CAST(Float)
	DEFINE_CAST(Int)
};

struct UInt2 : public Data
{
	DEFINE_DATA(UInt2, ReturnType::UInt2)
	DEFINE_ASSIGN(UInt2)

	DEFINE_CONSTRUCTOR_2(uint32_t, uint32_t)
	DEFINE_CONSTRUCTOR_2(uint32_t, UInt)
	DEFINE_CONSTRUCTOR_2(UInt, uint32_t)
	DEFINE_CONSTRUCTOR_2(UInt, UInt)

	DEFINE_ADDITION_SUBTRACTION(UInt2, UInt2)
	DEFINE_MULTIPLICATION(UInt2, UInt2)
	DEFINE_MULTIPLICATION(UInt2, UInt)
	DEFINE_MULTIPLICATION(UInt2, uint32_t)
	DEFINE_MULTIPLICATION(Float2, Float)
	DEFINE_MULTIPLICATION(Float2, float)
	DEFINE_DIVISION(UInt2, UInt)
	DEFINE_DIVISION(UInt2, uint32_t)
	DEFINE_DIVISION(Float2, Float)
	DEFINE_DIVISION(Float2, float)

	DEFINE_CAST(Float2)
	DEFINE_CAST(Int2)

	DECLARE_MEMBER_TABLE(UInt2)
	DECLARE_MEMBER_VAR(UInt, X)
	DECLARE_MEMBER_VAR(UInt, Y)
};

struct UInt3 : public Data
{
	DEFINE_DATA(UInt3, ReturnType::UInt3)
	DEFINE_ASSIGN(UInt3)

	DEFINE_CONSTRUCTOR_3(uint32_t, uint32_t, uint32_t)
	DEFINE_CONSTRUCTOR_3(uint32_t, uint32_t, UInt)
	DEFINE_CONSTRUCTOR_3(uint32_t, UInt, uint32_t)
	DEFINE_CONSTRUCTOR_3(uint32_t, UInt, UInt)
	DEFINE_CONSTRUCTOR_3(UInt, uint32_t, uint32_t)
	DEFINE_CONSTRUCTOR_3(UInt, uint32_t, UInt)
	DEFINE_CONSTRUCTOR_3(UInt, UInt, uint32_t)
	DEFINE_CONSTRUCTOR_3(UInt, UInt, UInt)

	DEFINE_ADDITION_SUBTRACTION(UInt3, UInt3)
	DEFINE_MULTIPLICATION(UInt3, UInt3)
	DEFINE_MULTIPLICATION(UInt3, UInt)
	DEFINE_MULTIPLICATION(UInt3, uint32_t)
	DEFINE_MULTIPLICATION(Float3, Float)
	DEFINE_MULTIPLICATION(Float3, float)
	DEFINE_DIVISION(UInt3, UInt)
	DEFINE_DIVISION(UInt3, uint32_t)
	DEFINE_DIVISION(Float3, Float)
	DEFINE_DIVISION(Float3, float)

	DEFINE_CAST(Float3)
	DEFINE_CAST(Int3)

	DECLARE_MEMBER_TABLE(UInt3)
	DECLARE_MEMBER_VAR(UInt, X)
	DECLARE_MEMBER_VAR(UInt, Y)
	DECLARE_MEMBER_VAR(UInt, Z)
};

struct UInt4 : public Data
{
	DEFINE_DATA(UInt4, ReturnType::UInt4)
	DEFINE_ASSIGN(UInt4)

	DEFINE_CONSTRUCTOR_4(uint32_t, uint32_t, uint32_t, uint32_t)
	DEFINE_CONSTRUCTOR_4(uint32_t, uint32_t, uint32_t, UInt)
	DEFINE_CONSTRUCTOR_4(uint32_t, uint32_t, UInt, uint32_t)
	DEFINE_CONSTRUCTOR_4(uint32_t, uint32_t, UInt, UInt)
	DEFINE_CONSTRUCTOR_4(uint32_t, UInt, uint32_t, uint32_t)
	DEFINE_CONSTRUCTOR_4(uint32_t, UInt, uint32_t, UInt)
	DEFINE_CONSTRUCTOR_4(uint32_t, UInt, UInt, uint32_t)
	DEFINE_CONSTRUCTOR_4(uint32_t, UInt, UInt, UInt)
	
	DEFINE_CONSTRUCTOR_4(UInt, uint32_t, uint32_t, uint32_t)
	DEFINE_CONSTRUCTOR_4(UInt, uint32_t, uint32_t, UInt)
	DEFINE_CONSTRUCTOR_4(UInt, uint32_t, UInt, uint32_t)
	DEFINE_CONSTRUCTOR_4(UInt, uint32_t, UInt, UInt)
	DEFINE_CONSTRUCTOR_4(UInt, UInt, uint32_t, uint32_t)
	DEFINE_CONSTRUCTOR_4(UInt, UInt, uint32_t, UInt)
	DEFINE_CONSTRUCTOR_4(UInt, UInt, UInt, uint32_t)
	DEFINE_CONSTRUCTOR_4(UInt, UInt, UInt, UInt)

	DEFINE_ADDITION_SUBTRACTION(UInt4, UInt4)
	DEFINE_MULTIPLICATION(UInt4, UInt4)
	DEFINE_MULTIPLICATION(UInt4, UInt)
	DEFINE_MULTIPLICATION(UInt4, uint32_t)
	DEFINE_MULTIPLICATION(Float4, Float)
	DEFINE_MULTIPLICATION(Float4, float)
	DEFINE_DIVISION(UInt4, UInt)
	DEFINE_DIVISION(UInt4, uint32_t)
	DEFINE_DIVISION(Float4, Float)
	DEFINE_DIVISION(Float4, float)

	DEFINE_CAST(Float4)
	DEFINE_CAST(Int4)

	DECLARE_MEMBER_TABLE(UInt4)
	DECLARE_MEMBER_VAR(UInt, X)
	DECLARE_MEMBER_VAR(UInt, Y)
	DECLARE_MEMBER_VAR(UInt, Z)
	DECLARE_MEMBER_VAR(UInt, W)
};

#pragma endregion

#pragma region Float Types

struct Float : public Data
{
	DEFINE_DATA(Float, ReturnType::Float)

	DEFINE_ASSIGN(float)
	DEFINE_ASSIGN(Float)

	DEFINE_EQUALS_OPERATORS(float)
	DEFINE_EQUALS_OPERATORS(Float)
	DEFINE_COMPARISON_OPERATORS(float)
	DEFINE_COMPARISON_OPERATORS(Float)

	DEFINE_ADDITION_SUBTRACTION(Float, float)
	DEFINE_ADDITION_SUBTRACTION(Float, Float)
	DEFINE_NEGATION
	DEFINE_MULTIPLICATION(Float, float)
	DEFINE_MULTIPLICATION(Float, Float)
	DEFINE_DIVISION(Float, float)
	DEFINE_DIVISION(Float, Float)

	DEFINE_CAST(Int)
	DEFINE_CAST(UInt)
};

struct Float2 : public Data
{
	DEFINE_DATA(Float2, ReturnType::Float2)
	DEFINE_ASSIGN(Float2)

	DEFINE_CONSTRUCTOR_2(float, float)
	DEFINE_CONSTRUCTOR_2(float, Float)
	DEFINE_CONSTRUCTOR_2(Float, float)
	DEFINE_CONSTRUCTOR_2(Float, Float)

	DEFINE_ADDITION_SUBTRACTION(Float2, Float2)
	DEFINE_NEGATION
	DEFINE_MULTIPLICATION(Float2, Float2)
	DEFINE_MULTIPLICATION(Float2, Float)
	DEFINE_MULTIPLICATION(Float2, float)
	DEFINE_MULTIPLICATION(Float2, Int)
	DEFINE_MULTIPLICATION(Float2, int32_t)
	DEFINE_MULTIPLICATION(Float2, UInt)
	DEFINE_MULTIPLICATION(Float2, uint32_t)
	DEFINE_DIVISION(Float2, Float)
	DEFINE_DIVISION(Float2, float)
	DEFINE_DIVISION(Float2, Int)
	DEFINE_DIVISION(Float2, int32_t)
	DEFINE_DIVISION(Float2, UInt)
	DEFINE_DIVISION(Float2, uint32_t)

	DEFINE_CAST(Int2)
	DEFINE_CAST(UInt2)

	DECLARE_MEMBER_TABLE(Float2)
	DECLARE_MEMBER_VAR(Float, X);
	DECLARE_MEMBER_VAR(Float, Y);
};

struct Float3 : public Data
{
	DEFINE_DATA(Float3, ReturnType::Float3)
	DEFINE_ASSIGN(Float3)

	DEFINE_CONSTRUCTOR_3(float, float, float)
	DEFINE_CONSTRUCTOR_3(float, float, Float)
	DEFINE_CONSTRUCTOR_3(float, Float, float)
	DEFINE_CONSTRUCTOR_3(float, Float, Float)
	DEFINE_CONSTRUCTOR_3(Float, float, float)
	DEFINE_CONSTRUCTOR_3(Float, float, Float)
	DEFINE_CONSTRUCTOR_3(Float, Float, float)
	DEFINE_CONSTRUCTOR_3(Float, Float, Float)

	DEFINE_ADDITION_SUBTRACTION(Float3, Float3)
	DEFINE_NEGATION
	DEFINE_MULTIPLICATION(Float3, Float3)
	DEFINE_MULTIPLICATION(Float3, Float)
	DEFINE_MULTIPLICATION(Float3, float)
	DEFINE_MULTIPLICATION(Float3, Int)
	DEFINE_MULTIPLICATION(Float3, int32_t)
	DEFINE_MULTIPLICATION(Float3, UInt)
	DEFINE_MULTIPLICATION(Float3, uint32_t)
	DEFINE_DIVISION(Float3, Float)
	DEFINE_DIVISION(Float3, float)
	DEFINE_DIVISION(Float3, Int)
	DEFINE_DIVISION(Float3, int32_t)
	DEFINE_DIVISION(Float3, UInt)
	DEFINE_DIVISION(Float3, uint32_t)

	DEFINE_CAST(UInt3)
	DEFINE_CAST(Int3)

	DECLARE_MEMBER_TABLE(Float3)
	DECLARE_MEMBER_VAR(Float, X)
	DECLARE_MEMBER_VAR(Float, Y)
	DECLARE_MEMBER_VAR(Float, Z)
};

struct Float4 : public Data
{
	DEFINE_DATA(Float4, ReturnType::Float4)
	DEFINE_ASSIGN(Float4)

	DEFINE_CONSTRUCTOR_4(float, float, float, float)
	DEFINE_CONSTRUCTOR_4(float, float, float, Float)
	DEFINE_CONSTRUCTOR_4(float, float, Float, float)
	DEFINE_CONSTRUCTOR_4(float, float, Float, Float)
	DEFINE_CONSTRUCTOR_4(float, Float, float, float)
	DEFINE_CONSTRUCTOR_4(float, Float, float, Float)
	DEFINE_CONSTRUCTOR_4(float, Float, Float, float)
	DEFINE_CONSTRUCTOR_4(float, Float, Float, Float)

	DEFINE_CONSTRUCTOR_4(Float, float, float, float)
	DEFINE_CONSTRUCTOR_4(Float, float, float, Float)
	DEFINE_CONSTRUCTOR_4(Float, float, Float, float)
	DEFINE_CONSTRUCTOR_4(Float, float, Float, Float)
	DEFINE_CONSTRUCTOR_4(Float, Float, float, float)
	DEFINE_CONSTRUCTOR_4(Float, Float, float, Float)
	DEFINE_CONSTRUCTOR_4(Float, Float, Float, float)
	DEFINE_CONSTRUCTOR_4(Float, Float, Float, Float)

	DEFINE_ADDITION_SUBTRACTION(Float4, Float4)
	DEFINE_NEGATION
	DEFINE_MULTIPLICATION(Float4, Float4)
	DEFINE_MULTIPLICATION(Float4, Float)
	DEFINE_MULTIPLICATION(Float4, float)
	DEFINE_MULTIPLICATION(Float4, Int)
	DEFINE_MULTIPLICATION(Float4, int32_t)
	DEFINE_MULTIPLICATION(Float4, UInt)
	DEFINE_MULTIPLICATION(Float4, uint32_t)

	DEFINE_DIVISION(Float4, Float)
	DEFINE_DIVISION(Float4, float)
	DEFINE_DIVISION(Float4, Int)
	DEFINE_DIVISION(Float4, int32_t)
	DEFINE_DIVISION(Float4, UInt)
	DEFINE_DIVISION(Float4, uint32_t)

	DEFINE_CAST(Int4)
	DEFINE_CAST(UInt4)

	DECLARE_MEMBER_TABLE(Float4)
	DECLARE_MEMBER_VAR(Float, X)
	DECLARE_MEMBER_VAR(Float, Y)
	DECLARE_MEMBER_VAR(Float, Z)
	DECLARE_MEMBER_VAR(Float, W)
};

#pragma endregion

#pragma region Buffers

template<typename TYPE>
struct Buffer1D : public Data
{
	/// Prevent instantions of Buffer2Ds with invalid TYPE
#pragma region
	template<typename T>
	struct valid_type
	{
		enum
		{
			is_valid = 0,
		};
	};

#	define MAKE_VALID(T) template<> struct valid_type<##T##> { enum { is_valid=1 }; };
	MAKE_VALID(Int)
		MAKE_VALID(Int2)
		MAKE_VALID(Int3)	
		MAKE_VALID(Int4)
		MAKE_VALID(UInt)
		MAKE_VALID(UInt2)
		MAKE_VALID(UInt3)	
		MAKE_VALID(UInt4)
		MAKE_VALID(Float)
		MAKE_VALID(Float2)
		MAKE_VALID(Float3)	
		MAKE_VALID(Float4)
		static_assert(valid_type<TYPE>::is_valid, "Invalid type used in Buffer1D");
#pragma endregion
	DEFINE_DATA(Buffer1D<TYPE>, (ReturnType::Type)(TYPE::_return_type | ReturnType::Buffer1D))

	/// sampling operators
	Temp<TYPE> operator()(int32_t A) const
	{
		COMPUTE_ASSERT(A >= 0);
		ASTNode* sample  = new ASTNode(NodeType::Sample1D, get_return_type<TYPE>());
		sample->add_child(new ASTNode(NodeType::ConstVar, get_return_type<Buffer1D<TYPE>>(), _id));
		sample->add_child(create_literal_node(A));

		return Temp<TYPE>(sample);
	}

	Temp<TYPE> operator()(Int A) const
	{
		ASTNode* sample  = new ASTNode(NodeType::Sample1D, get_return_type<TYPE>());
		sample->add_child(new ASTNode(NodeType::ConstVar, get_return_type<Buffer1D<TYPE>>(), _id));
		sample->add_child(create_data_node(A));

		return Temp<TYPE>(sample);
	}
};

template <typename TYPE>
struct Buffer2D : public Data
{
	/// Prevent instantions of Buffer2Ds with invalid TYPE
#pragma region
	template<typename T>
	struct valid_type
	{
		enum
		{
			is_valid = 0,
		};
	};

#	define MAKE_VALID(T) template<> struct valid_type<##T##> { enum { is_valid=1 }; };
	MAKE_VALID(Int)
	MAKE_VALID(Int2)
	MAKE_VALID(Int3)	
	MAKE_VALID(Int4)
	MAKE_VALID(UInt)
	MAKE_VALID(UInt2)
	MAKE_VALID(UInt3)	
	MAKE_VALID(UInt4)
	MAKE_VALID(Float)
	MAKE_VALID(Float2)
	MAKE_VALID(Float3)	
	MAKE_VALID(Float4)
	static_assert(valid_type<TYPE>::is_valid, "Invalid type used in Buffer2D");
#pragma endregion
	DEFINE_DATA(Buffer2D<TYPE>, (ReturnType::Type)(TYPE::_return_type | ReturnType::Buffer2D))


	/// sampling operators	
	Temp<TYPE> operator()(int32_t A, int32_t B) const
	{
		COMPUTE_ASSERT(A >= 0 && B >= 0);

		ASTNode* sample  = new ASTNode(NodeType::Sample2D, get_return_type<TYPE>());
		sample->add_child(new ASTNode(NodeType::ConstVar, get_return_type<Buffer2D<TYPE>>(), _id));
		sample->add_child(create_literal_node(A));
		sample->add_child(create_literal_node(B));
		return Temp<TYPE>(sample);
	}

	Temp<TYPE> operator()(const Int& A, int B) const
	{
		COMPUTE_ASSERT(B >= 0);

		ASTNode* sample  = new ASTNode(NodeType::Sample2D, get_return_type<TYPE>());
		sample->add_child(new ASTNode(NodeType::ConstVar, get_return_type<Buffer2D<TYPE>>(), _id));
		sample->add_child(create_data_node(A));
		sample->add_child(create_literal_node(B));
		return Temp<TYPE>(sample);
	}

	Temp<TYPE> operator()(int A, const Int& B) const 
	{
		COMPUTE_ASSERT(A >= 0);

		ASTNode* sample  = new ASTNode(NodeType::Sample2D, get_return_type<TYPE>());
		sample->add_child(new ASTNode(NodeType::ConstVar, get_return_type<Buffer2D<TYPE>>(), _id));
		sample->add_child(create_literal_node(A));
		sample->add_child(create_data_node(B));
		return Temp<TYPE>(sample);
	}

	Temp<TYPE> operator()(const Int& A, const Int& B) const
	{
		ASTNode* sample  = new ASTNode(NodeType::Sample2D, get_return_type<TYPE>());
		sample->add_child(new ASTNode(NodeType::ConstVar, get_return_type<Buffer2D<TYPE>>(), _id));
		sample->add_child(create_data_node(A));
		sample->add_child(create_data_node(B));
		return Temp<TYPE>(sample);
	}
};

#pragma endregion