template<typename T> static bool is_primitive()
{
	//printf("type %s is not a primitive\n", typeid(T).name());
	return false;
}
#pragma region is primitive specializations
#define IS_PRIMITIVE(A) template<> static bool is_primitive<##A##>() { return true; }
IS_PRIMITIVE(bool)
IS_PRIMITIVE(uint8_t)
IS_PRIMITIVE(int8_t)
IS_PRIMITIVE(uint16_t)
IS_PRIMITIVE(int16_t)
IS_PRIMITIVE(uint32_t)
IS_PRIMITIVE(int32_t)
IS_PRIMITIVE(uint64_t)
IS_PRIMITIVE(int64_t)
IS_PRIMITIVE(float)
IS_PRIMITIVE(double)
#pragma endregion

template<typename T> static ReturnType::Type get_return_type()
{
	return T::_return_type;
}
#pragma region primitive to returntype conversion specialization

#define PRIMITIVE_TO_RETURNTYPE(P, RT) template<> static ReturnType::Type get_return_type<##P##>() {return (RT);}
PRIMITIVE_TO_RETURNTYPE(bool, ReturnType::Bool)
PRIMITIVE_TO_RETURNTYPE(int32_t, ReturnType::Int)
PRIMITIVE_TO_RETURNTYPE(uint32_t, ReturnType::UInt)
PRIMITIVE_TO_RETURNTYPE(float, ReturnType::Float)
#pragma endregion

#define BEGIN_SOURCE void Parse() { initialize();
#define END_SOURCE finalize(); }

#define BEGIN_OUT_DATA BeginBlock<SiCKL::NodeType::OutData> begin_out_data;
#define OUT_DATA(TYPE, NAME) Out<##TYPE##> NAME( #NAME );
#define END_OUT_DATA EndBlock end_out_data;

#define BEGIN_CONST_DATA BeginBlock<SiCKL::NodeType::ConstData> begin_const_data;
#define CONST_DATA(TYPE, NAME) const Const<##TYPE##> NAME( #NAME );
#define END_CONST_DATA EndBlock end_const_data;

#define BEGIN_MAIN BeginBlock<SiCKL::NodeType::Main> begin_main;
#define END_MAIN EndBlock end_main;

template<NodeType::Type TYPE>
struct BeginBlock
{
	BeginBlock() 
	{
		ASTNode* begin = new ASTNode(TYPE, ReturnType::Void);
		_current_block->add_child(begin);

		_block_stack.push_back(begin);
		_current_block = begin;
	};
};

struct EndBlock
{
	EndBlock()
	{
		_block_stack.pop_back();
		_current_block = _block_stack.back();
	}
};

struct Data
{
	Data()
		: _id(invalid_symbol)
		, _node(nullptr)
		, _name(nullptr)
	{ }
	~Data()
	{
		if(_node)
		{
			delete _node;
			_node = nullptr;
		}
	}
	Data(symbol_id_t in_id, ASTNode* in_node, const char* in_name)
		: _id(in_id)
		, _node(in_node)
		, _name(in_name)
	{ }
	symbol_id_t _id;
	ASTNode* _node;
	const char* _name;
};

#define DEFINE_DATA(BASE,RETURN)\
	BASE() : Data() {}\
	BASE(symbol_id_t in_id, ASTNode* in_node, const char* in_name) : Data(in_id, in_node, in_name) {}\
	static const ReturnType::Type _return_type = RETURN;\
	private:\
	typedef BASE base_type;\
	public:


// Temp template is used internally as a return type from operators denoting
// 'value' types 
template<class BASE>
struct Temp : public BASE
{
private:
	// private constructor to prevent instantiation by users
	Temp() {};

	// this constructor will call the base constructor
	// which should in turn call the IData constructor, prevent
	// another symbol_id being allocated to the Temp<BASE> object
	Temp(ASTNode* in_node) : BASE(temp_symbol, in_node, nullptr) {};

	BASE& operator=(const BASE&)
	{
		COMPUTE_ASSERT(0);
		return *this;
	}

	// friend specific operators that need Temp's private constructor
	friend struct Source;
	template<typename BASE,typename>
	friend struct IEquals;
	template<typename  BASE,typename>
	friend struct ICompare;
	template<typename BASE,typename>
	friend struct IAddSub;
	template<typename BASE>
	friend struct Buffer2D;
};

// Out template is used for data locations written to by a
// shader program
template<class BASE>
struct Out : public BASE
{
	using BASE::operator=;

	Out(const char* in_name) : BASE(next_symbol(), nullptr, in_name)
	{
		COMPUTE_ASSERT(_current_block->_node_type == NodeType::OutData);

		ASTNode* out = new ASTNode(NodeType::OutVar, get_return_type<BASE>(), _id);
		out->_name = in_name;
		_current_block->add_child(out);
	}
};

// Const template is used in parameter list in programs, adds self to the AST
// as a declaration inside a ConstBuffer table
template<class BASE>
struct Const : public BASE
{
	Const(const char* in_name) : BASE(next_symbol(), nullptr, in_name)
	{
		COMPUTE_ASSERT(_current_block->_node_type == NodeType::ConstData);

		ASTNode* decl = new ASTNode(NodeType::ConstVar, get_return_type<BASE>(), _id);
		decl->_name = in_name;
		_current_block->add_child(decl);
	}
};

template<typename BASE, typename PARENT>
struct Member : public BASE
{
	using BASE::operator=;
	Member(uint32_t member_offset, member_id_t mid) : BASE(temp_symbol, nullptr, nullptr)
	{
		// accesses to a Member type 
		_id = member_symbol;

		// get our owner
		PARENT* parent = (PARENT*)(((uint8_t*)this) - member_offset);

		// in the case where our parent did not get anything assigned to it, give it a valid symbol
		if(parent->_id == invalid_symbol)
		{
			parent->_id = next_symbol();
		}

		// construct our ASTnode that represents us
		ASTNode* member = new ASTNode(NodeType::Member, get_return_type<BASE>());
		binary_operator(member, *parent, mid);

		_node = member;
	}
};

#define DECLARE_MEMBER_TABLE(PARENT)\
	private:\
	typedef PARENT parent_type;\
	public:
#define DECLARE_MEMBER_VAR(TYPE, NAME)\
struct Member_##NAME : public Member<TYPE, parent_type>\
{\
	using Member<TYPE, parent_type>::operator=;\
	Member_##NAME() : Member<TYPE, parent_type>(_member_offset, _mid) { }\
private:\
	const static uint32_t _member_offset;\
	const static member_id_t _mid;\
} NAME;\

#define DEFINE_MEMBER_VAR(PARENT, TYPE, NAME, ID)\
	const uint32_t SiCKL::Source::##PARENT##::Member_##NAME::_member_offset = (uint32_t)&((SiCKL::Source::##PARENT##*)(void*)0)->##NAME;\
	const SiCKL::member_id_t SiCKL::Source::##PARENT##::Member_##NAME::_mid = ID;


/// Operator Methods
template<typename BASE>
static void unary_operator(ASTNode* root, const BASE& in_base)
{
	COMPUTE_ASSERT(is_primitive<BASE>() == false);

	ASTNode* child = in_base._id >= 0 ? new ASTNode(NodeType::Var, get_return_type<BASE>(), in_base._id) : new ASTNode(*in_base._node);
	root->add_child(child);
}

template<typename TYPE>
static ASTNode* create_literal_node(const TYPE& val)
{
	COMPUTE_ASSERT(is_primitive<TYPE>() == true);
	return new ASTNode(NodeType::Literal, get_return_type<TYPE>(), &val);
}

template<typename TYPE>
static ASTNode* create_data_node(const TYPE& val)
{
	COMPUTE_ASSERT(is_primitive<TYPE>() == false);
	Data& data = *(Data*)&val;
	if(data._id >= 0)
	{
		// a symbol
		return new ASTNode(NodeType::Var, get_return_type<TYPE>(), data._id);
	}
	else
	{
		// make sure the temp data is initialized
		COMPUTE_ASSERT(data._node != nullptr);
		return new ASTNode(*data._node);
	}
	return nullptr;
}

template<typename TYPE>
static ASTNode* create_value_node(const TYPE& val)
{
	ASTNode* result = nullptr;
	if(is_primitive<TYPE>())
	{
		//make a literal node
		result = create_literal_node<TYPE>(val);
	}
	else
	{
		// if it's not a primitive, it must be a Data
		result = create_data_node<TYPE>(val);

	}
	return result;
}

template<typename BASE, typename FRIEND>
static void binary_operator(ASTNode* root, const BASE& in_base, const FRIEND& in_friend)
{
	ASTNode* left = create_value_node(in_base);
	ASTNode* right = create_value_node(in_friend);

	root->add_child(left);
	root->add_child(right);
}

#define DEFINE_ASSIGN(FRIEND)\
base_type(const FRIEND& F)\
{\
	*this = F;\
}\
base_type& operator=(const FRIEND& F)\
{\
	if(this->_id == invalid_symbol)\
	{\
		this->_id = next_symbol();\
	}\
	ASTNode* assign = new ASTNode(NodeType::Assignment, base_type::_return_type);\
	binary_operator(assign, *this, F);\
	_current_block->add_child(assign);\
	return *this;\
}

#define DEFINE_UNARY_OPERATOR(RETURNTYPE, NODETYPE, OP)\
Temp<RETURNTYPE> operator##OP() const\
{\
	ASTNode* node = new ASTNode(NodeType::##NODETYPE, get_return_type<RETURNTYPE>());\
	unary_operator(node, *this);\
	return Temp<RETURNTYPE>(node);\
}

#define DEFINE_BINARY_OPERATOR(RETURNTYPE, FRIENDTYPE, NODETYPE, OP)\
Temp<RETURNTYPE> operator##OP##(const FRIENDTYPE& F) const\
{\
	ASTNode* node = new ASTNode(NodeType::##NODETYPE, get_return_type<RETURNTYPE>());\
	binary_operator(node, *this, F);\
	return Temp<RETURNTYPE>(node);\
}

#define DEFINE_BINARY_ASSIGNMENT_OPERATOR(RETURNTYPE, FRIENDTYPE, NODETYPE, OP)\
base_type& operator##OP##=(const FRIENDTYPE& F)\
{\
	*this = (const base_type&)(*this OP F);\
	return *this;\
}

#define DEFINE_BINARY_OPERATOR_FULL(RETURNTYPE, FRIENDTYPE, NODETYPE, OP)\
DEFINE_BINARY_OPERATOR(RETURNTYPE, FRIENDTYPE, NODETYPE, OP)\
DEFINE_BINARY_ASSIGNMENT_OPERATOR(RETURNTYPE, FRIENDTYPE, NODETYPE, OP)

#define DEFINE_EQUALS_OPERATORS(FRIEND)\
Temp<Bool> operator==(const FRIEND& F) const\
{\
	ASTNode* equals = new ASTNode(NodeType::Equal, ReturnType::Bool);\
	binary_operator(equals, *this, F);\
	return Temp<Bool>(equals);\
}\
Temp<Bool> operator!=(const FRIEND& F) const\
{\
	ASTNode* notequal = new ASTNode(NodeType::NotEqual, ReturnType::Bool);\
	binary_operator(notequal, *this, F);\
	return Temp<Bool>(notequal);\
}

#define DEFINE_COMPARISON_OPERATORS(FRIEND)\
Temp<Bool> operator>(const FRIEND& F) const\
{\
	ASTNode* node = new ASTNode(NodeType::Greater, ReturnType::Bool);\
	binary_operator(node, *this, F);\
	return Temp<Bool>(node);\
}\
Temp<Bool> operator>=(const FRIEND& F) const\
{\
	ASTNode* node = new ASTNode(NodeType::GreaterEqual, ReturnType::Bool);\
	binary_operator(node, *this, F);\
	return Temp<Bool>(node);\
}\
Temp<Bool> operator<(const FRIEND& F) const\
{\
	ASTNode* node = new ASTNode(NodeType::Less, ReturnType::Bool);\
	binary_operator(node, *this, F);\
	return Temp<Bool>(node);\
}\
Temp<Bool> operator<=(const FRIEND& F) const\
{\
	ASTNode* node = new ASTNode(NodeType::LessEqual, ReturnType::Bool);\
	binary_operator(node, *this, F);\
	return Temp<Bool>(node);\
}

#define DEFINE_ADDITION_SUBTRACTION(RETURN, FRIEND)\
	DEFINE_BINARY_OPERATOR_FULL(RETURN, FRIEND, Add, +)\
	DEFINE_BINARY_OPERATOR_FULL(RETURN, FRIEND, Subtract, -)

#define DEFINE_NEGATION \
DEFINE_UNARY_OPERATOR(base_type, UnaryMinus, -)

#define DEFINE_MULTIPLICATION(RETURN, FRIEND)\
	DEFINE_BINARY_OPERATOR_FULL(RETURN, FRIEND, Multiply, *)

#define DEFINE_DIVISION(RETURN, FRIEND)\
	DEFINE_BINARY_OPERATOR_FULL(RETURN, FRIEND, Divide, /)

#define DEFINE_MODULO(RETURN, FRIEND)\
	DEFINE_BINARY_OPERATOR_FULL(RETURN, FRIEND, Modulo, %)

#define DEFINE_BITWISE_NOT \
Temp<base_type> operator~() const\
{\
	ASTNode* node = new ASTNode(NodeType::BitwiseNot, get_return_type<base_type>());\
	unary_operator(node, *(base_type*)this);\
	return Temp<base_type>(node);\
}\

#define DEFINE_BITWISE_OPERATORS(RETURN, FRIEND)\
	DEFINE_BINARY_OPERATOR_FULL(RETURN, FRIEND, BitwiseAnd, &)\
	DEFINE_BINARY_OPERATOR_FULL(RETURN, FRIEND, BitwiseOr, |)\
	DEFINE_BINARY_OPERATOR_FULL(RETURN, FRIEND, BitwiseXor, ^)

/// Functions

#define DEFINE_CONSTRUCTOR_1(T1)\
base_type(const T1& A)\
{\
	ASTNode* node = new ASTNode(NodeType::Constructor, get_return_type<base_type>());\
	node->add_child(create_value_node(A));\
	*this = Temp<base_type>(node);\
}

#define DEFINE_CONSTRUCTOR_2(T1, T2)\
base_type(const T1& A, const T2& B)\
{\
	ASTNode* node = new ASTNode(NodeType::Constructor, get_return_type<base_type>());\
	node->add_child(create_value_node(A));\
	node->add_child(create_value_node(B));\
	*this = Temp<base_type>(node);\
}

#define DEFINE_CONSTRUCTOR_3(T1, T2, T3)\
base_type(const T1& A, const T2& B, const T3& C)\
{\
	ASTNode* node = new ASTNode(NodeType::Constructor, get_return_type<base_type>());\
	node->add_child(create_value_node(A));\
	node->add_child(create_value_node(B));\
	node->add_child(create_value_node(C));\
	*this = Temp<base_type>(node);\
}

#define DEFINE_CONSTRUCTOR_4(T1, T2, T3, T4)\
base_type(const T1& A, const T2& B, const T3& C, const T4& D)\
{\
	ASTNode* node = new ASTNode(NodeType::Constructor, get_return_type<base_type>());\
	node->add_child(create_value_node(A));\
	node->add_child(create_value_node(B));\
	node->add_child(create_value_node(C));\
	node->add_child(create_value_node(D));\
	*this = Temp<base_type>(node);\
}

#define DEFINE_CAST(TO)\
operator TO() const\
{\
	ASTNode* node = new ASTNode(NodeType::Cast, get_return_type<TO>());\
	node->add_child(create_value_node(*this));\
	return TO(temp_symbol, node, nullptr);\
}

#define FUNC_HEADER(RETURNTYPE, NAME)\
	ASTNode* node = new ASTNode(NodeType::Function, get_return_type<RETURNTYPE>());\
	int32_t id = BuiltinFunction::NAME;\
	ASTNode* ident = new ASTNode(NodeType::Literal, ReturnType::Int, &id);\
	node->add_child(ident);

#define DEFINE_FUNC_0(RETURNTYPE, NAME)\
Temp<RETURNTYPE> NAME()\
{\
	FUNC_HEADER(RETURNTYPE, NAME)\
	return Temp<RETURNTYPE>(node);\
}

#define DEFINE_FUNC_1(RETURNTYPE, NAME, T1)\
Temp<RETURNTYPE> NAME(const T1& A)\
{\
	FUNC_HEADER(RETURNTYPE, NAME)\
	node->add_child(create_value_node(A));\
	return Temp<RETURNTYPE>(node);\
}

#define DEFINE_FUNC_2(RETURNTYPE, NAME, T1, T2)\
	Temp<RETURNTYPE> NAME(const T1& A, const T2& B)\
{\
	FUNC_HEADER(RETURNTYPE, NAME)\
	node->add_child(create_value_node(A));\
	node->add_child(create_value_node(B));\
	return Temp<RETURNTYPE>(node);\
}

#define DEFINE_FUNC_3(RETURNTYPE, NAME, T1, T2, T3)\
	Temp<RETURNTYPE> NAME(const T1& A, const T2& B, const T3& C)\
{\
	FUNC_HEADER(RETURNTYPE, NAME)\
	node->add_child(create_value_node(A));\
	node->add_child(create_value_node(B));\
	node->add_child(create_value_node(C));\
	return Temp<RETURNTYPE>(node);\
}