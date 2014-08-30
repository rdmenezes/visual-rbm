#pragma once

// std
#include <string>
#include <sstream>
#include <vector>
#include <cstdint>

// tries to consume a token of type TOK from reader READER, returns RESULT if we get something else

#define SetReader(READER) cppJSONStream::Reader& __reader = READER
#define SetErrorResult(RESULT) auto __result = RESULT
#define TryGetToken(TOK) do { cppJSONStream::Token_t __tok = __reader.next(); if(__tok != TOK) {return __result;}} while(0,0)
#define TryGetNameValuePair(NAME, TOK) do { cppJSONStream::Token_t __tok = __reader.next(); if(__tok != cppJSONStream::Token::ValueName) {return __result;} if(__reader.readString() == NAME){ __tok = __reader.next(); if(__tok != TOK) {return __result;} } else{return __result;}} while(0,0)
#define VerifyEqual(A, B) do { if((A) != (B)) {return __result;}} while(0,0)

namespace cppJSONStream
{
	namespace Token
	{
		enum Enum
		{
			Invalid = -1,
			BeginObject,
			EndObject,
			BeginArray,
			EndArray,
			ValueName,
			String,
			Number,
			Bool,
			Null,
		};
	}
	typedef Token::Enum Token_t;

	class Reader
	{
	public:
		Reader(std::istream& stream);
		Token_t next();

		bool readBoolean();
		double readDouble();
		int64_t readInt();
		uint64_t readUInt();
		std::string readString();
		

	private:

		bool parse_bool();
		bool parse_number();
		bool parse_string();
		bool parse_null();
		Token_t parse_value();

		bool b;
		double d;
		int64_t i;
		uint64_t u;
		std::string str;
		nullptr_t null;

		struct scope
		{
			// in object
			bool need_name;
			bool need_value;
			uint32_t pair_count;
			// in array
			bool need_element;
			uint32_t element_count;
		};

		bool _error;

		std::istream& _stream;
		std::vector<scope> _scope_stack;
	};
	
	class Writer
	{
	public:
		Writer(std::ostream& stream, bool pretty_format);
		void begin_object();
		void write_name(const char* name);
		void end_object();
		void begin_array();
		void end_array();

		// write values
		void write(bool value);
		void write(int64_t value);
		void write(uint64_t value);
		void write(double value);
		void write(const char* value);
		void write(nullptr_t value);

		// name/value pair
		template<typename T>
		void write_namevalue (const char* name, const T& value)
		{
			write_name(name);
			write(value);
		}

		template<typename T>
		void write_array(const T* values, size_t count)
		{
			begin_array();
			for(size_t k = 0; k < count; k++)
			{
				write(values[k]);
			}
			end_array();
		}
	private:

		void begin_write();
		void end_write();

		bool _pretty_format;

		struct scope
		{
			// start of object or array
			bool begin_array;
			bool begin_object;
			// in object
			bool need_name;
			bool need_value;
			size_t pair_count;
			// in array
			bool need_element;
			size_t element_count;
		};

		size_t _depth;
		std::vector<scope> _scope_stack;
		std::ostream& _stream;
	};
}