START_BUFFER_TYPE(Buffer1D)
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

#define MAKE_VALID(T) template<> struct valid_type<##T##> { enum { is_valid=1 }; };
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
	/// sampling operators
	Temp<TYPE> operator()(int32_t A) const
	{
		COMPUTE_ASSERT(A >= 0);
		ASTNode* sample  = new ASTNode(NodeType::Sample1D, get_return_type<TYPE>());
		sample->add_child(new ASTNode(NodeType::ConstVar, get_return_type<Buffer1D<TYPE>>(), _id));
		sample->add_child(create_literal_node(A));
		return Temp<TYPE>(sample);
	}

	Temp<TYPE> operator()(const Int& A) const
	{
		ASTNode* sample  = new ASTNode(NodeType::Sample1D, get_return_type<TYPE>());
		sample->add_child(new ASTNode(NodeType::ConstVar, get_return_type<Buffer1D<TYPE>>(), _id));
		sample->add_child(create_data_node(A));
		return Temp<TYPE>(sample);
	}
END_TYPE

START_BUFFER_TYPE(Buffer2D)
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

#       define MAKE_VALID(T) template<> struct valid_type<##T##> { enum { is_valid=1 }; };
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
	
	/// sampling operators  
	Temp<TYPE>  operator()(int32_t A, int32_t B) const
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

	Temp<TYPE>  operator()(int A, const Int& B) const 
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

	Temp<TYPE> operator()(const Int2& VEC) const
	{
		ASTNode* sample  = new ASTNode(NodeType::Sample2D, get_return_type<TYPE>());
		sample->add_child(new ASTNode(NodeType::ConstVar, get_return_type<Buffer2D<TYPE>>(), _id));
		sample->add_child(create_data_node(VEC));
		return Temp<TYPE>(sample);
	}
END_TYPE