#pragma once

#include "Enums.h"


namespace SiCKL
{
	struct ASTNode
	{
		ASTNode();
		ASTNode(const ASTNode&);
		ASTNode(NodeType::Type, ReturnType::Type);
		ASTNode(NodeType::Type, ReturnType::Type, symbol_id_t);
		template<typename T>
		ASTNode(NodeType::Type node_type, ReturnType::Type return_type, const T* data_pointer)
			: _node_type(node_type)
			, _return_type(return_type)
			, _count(0)
			, _capacity(1)
			, _name(nullptr)
		{
			COMPUTE_ASSERT(node_type == NodeType::Literal);
			// copy in raw data
			_u.literal.data = malloc(sizeof(T));
			_u.literal.size = sizeof(T);
			memcpy(_u.literal.data, data_pointer, _u.literal.size);


			_children = new ASTNode*[_capacity];
			_children[0] = nullptr;
		}
		~ASTNode();
		void add_child(ASTNode*);

		NodeType::Type _node_type;
		uint32_t _count;
		uint32_t _capacity;
		ASTNode** _children;

		ReturnType::Type _return_type;

		union
		{
			
			symbol_id_t sid;
			struct
			{
				void* data;
				size_t size;
			} literal;
			struct
			{
				symbol_id_t parent_sid;
				member_id_t mid;
			} member;
		} _u;
		const char* _name;

		void Print() const;
		void PrintNode() const;
		void PrintDot() const;
	private:
		static void Print(const ASTNode*, uint32_t indent);
		static void PrintDot(const ASTNode*, uint32_t& id);
	};
}