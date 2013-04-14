#pragma once

#include <stddef.h>
#include "SiCKL.h"
#include <map>
#include <utility>
#include <string>

namespace SiCKL
{
	struct ASTNode;
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

		// code generation for different operators
#		include "Interfaces.h"
		// our datatypes
#		include "Types.h"
		// and our functions
#		include "Functions.h"

		// control functions
		void _If(const Bool&);
		void _ElseIf(const Bool&);
		void _Else();
		void _While(const Bool&);

		void _StartBlock();		
		void _EndBlock();

		struct _ForInRange
		{
			_ForInRange(Int& i) : _counter(i) {}
			~_ForInRange() { _counter += 1;	}
			Int& _counter;
		};

	private:
		// these methods exist to prevent passing
		// bool literals into control flow statements
		// and creating temporary Bool objects
		void _If(bool) {}
		void _ElseIf(bool) {}
		void _While(bool) {}
	public:


#define If( A ) _If(A); {
#define ElseIf( A ) } _ElseIf(A); {
#define Else } _Else(); {
#define EndIf } _EndBlock();
#define While( A ) _While(A); {
#define EndWhile } _EndBlock();
#define ForInRange(I, START, STOP)\
		{\
			_StartBlock();\
			Int I = START;\
			While(I < STOP)\
				_ForInRange I##FOR(I);
#define EndFor\
			EndWhile\
			_EndBlock();\
		}

		
		// override Main for new program
		virtual void Parse() = 0;
	};
}