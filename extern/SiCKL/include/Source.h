#pragma once

#include <stddef.h>
#include <map>
#include <utility>
#include <string>
#include <stdint.h>
#include <vector>

#include "Enums.h"
#include "AST.h"

namespace SiCKL
{
	struct ASTNode;
	
	// forward declare all of our types
	struct Bool;
	struct Int;
	struct Int2;
	struct Int3;
	struct Int4;
	struct UInt;
	struct UInt2;
	struct UInt3;
	struct UInt4;
	struct Float;
	struct Float2;
	struct Float3;
	struct Float4;

	template<typename T> struct Buffer1D;
	template<typename T> struct Buffer2D;

	struct Source
	{
		Source();
		~Source();

		const ASTNode& GetRoot() const {return *_my_ast;}
		uint32_t GetSymbolCount() const {return _symbol_count;}
		//ASTSymbol* GetSymbolTable() const;

	private:
		static ASTNode* _root;
		static ASTNode* _current_block;
		static std::vector<ASTNode*> _block_stack;
		static symbol_id_t _next_symbol;

		static symbol_id_t next_symbol() {return _next_symbol++;};

		// the AST for this program's Source
		ASTNode* _my_ast;
		uint32_t _symbol_count;
	protected:
		void initialize();
		void finalize();
	public:
		// friend all these types
		friend struct Bool;
		friend struct Int;
		friend struct Int2;
		friend struct Int3;
		friend struct Int4;
		friend struct UInt;
		friend struct UInt2;
		friend struct UInt3;
		friend struct UInt4;
		friend struct Float;
		friend struct Float2;
		friend struct Float3;
		friend struct Float4;

		// friend types
		template<typename BASE>
		friend struct Out;
		template<typename BASE>
		friend struct Const;
		template<typename BASE, typename PARENT>
		friend struct Member;

		// control functions
		static void _If(const Bool&);
		static void _ElseIf(const Bool&);
		static void _Else();
		static void _While(const Bool&);
		static void _ForInRange(const Int&, int32_t from, int32_t to);

		// use for beginning and endnig scope nodes
		template<NodeType::Type TYPE>
		static void _StartBlock()
		{
			ASTNode* begin = new ASTNode(TYPE, ReturnType::Void);
			_current_block->add_child(begin);

			_block_stack.push_back(begin);
			_current_block = begin;
		}
		static void _EndBlock()
		{
			_block_stack.pop_back(); 
			_current_block = _block_stack.back();
		}
	private:
		// these methods exist to prevent passing
		// bool literals into control flow statements
		// and creating temporary Bool objects
		static void _If(bool) {}
		static void _ElseIf(bool) {}
		static void _While(bool) {}
	public:


#define If( A ) Source::_If(A); {
#define ElseIf( A ) } Source::_ElseIf(A); {
#define Else } Source::_Else(); {
#define EndIf } Source::_EndBlock();
#define While( A ) Source::_While(A); {
#define EndWhile } Source::_EndBlock();
#define ForInRange(I, START, STOP) { const Int I; Source::_ForInRange(I, START, STOP); {
#define EndFor } Source::_EndBlock(); }

		
		// override Main for new program
		virtual void Parse() = 0;
	};

	// defines various struct types used for creating new programs
#	include "Interfaces.h"
	// fills in operator DEFINES as declarations (implemented in Types.cpp)
#	include "Declares.h"
	// Declare our datatypes
#	include "Types.h"
	// Declare our functions
#	include "Functions.h"
	// and Declre our buffer types
#	include "Buffers.h"
}