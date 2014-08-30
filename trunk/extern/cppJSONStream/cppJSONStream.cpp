#include "cppJSONStream.hpp"

#include <cassert>
#include <sstream>

namespace cppJSONStream
{

/// Reader

	// skip whitespace
	static void skip(std::istream& stream)
	{
		while(true)
		{
			auto c = stream.peek();
			if(c == ' ' || c == '\t' || c == '\r' || c == '\n')
			{
				stream.get();
			}
			else
			{
				return;
			}
		}
	}

	bool Reader::parse_bool()
	{
		int c;
		char buf[5] = {0};
		for(size_t k = 0; k < 4; k++)
		{
			c = _stream.get();
			if(c == EOF)
			{
				return false;
			}
			buf[k] = (char)c;
		}
		// try for true
		if(strncmp(buf, "true", 4) == 0)
		{
			b = true;
			return true;
		}

		// get one more char
		c = _stream.get();
		if(c == EOF)
		{
			return false;
		}
		buf[4] = (char)c;

		// try for false
		if(strncmp(buf, "false", 5) == 0)
		{
			b = false;
			return true;
		}
		return false;
	}

	bool Reader::parse_number()
	{
		bool negative = false;
		uint64_t whole = 0u;
		double scale = 1.0;
		double fraction = 0.0;
		bool negative_exponent = false;
		int32_t exponent = 0;

		i = 0;
		u = 0u;
		d = 0.0;

		int c = _stream.peek();
		if(c == EOF) return false;

		if(c == '-')
		{
			negative = true;
			_stream.get();
		}
		
		// get the whole part
		for(c = _stream.peek(); c != EOF; c = _stream.peek())
		{
			if(c >= '0' && c <= '9')
			{
				whole = whole * 10 + uint64_t(c - '0');
				_stream.get();
			}
			else if (c == '.')
			{
				_stream.get();
				for(c = _stream.peek(); c != EOF; c = _stream.peek())
				{
					if(c >= '0' &&  c <= '9')
					{
						scale /= 10.0;
						fraction = fraction + double(c - '0') * scale;
						_stream.get();
					}
					else if(c == 'e' || c == 'E')
					{
						_stream.get();
						c = _stream.peek();
						if(c == EOF) return false;
						if(c == '-')
						{
							negative_exponent = true;
							_stream.get();
						}
						else if(c == '+')
						{
							_stream.get();
						}
						
						for(c = _stream.peek(); c != EOF; c = _stream.peek())
						{
							if(c >= '0' && c <= '9')
							{
								exponent = exponent * 10 + int32_t(c - '0');
								_stream.get();
							}
							else
							{
								break;
							}
						}
						break;
					}
					else
					{
						break;
					}
				}
				break;
			}
			else
			{
				break;
			}
		}

		if(c == EOF)
		{
			return false;
		}

		if(negative_exponent) exponent *= -1;
		d = (double(whole) + fraction) * std::pow(10.0, exponent);
		u = whole;
		i = (int64_t)u;

		if(negative)
		{
			d *= -1.0;
			i *= -1;
			u = 0;
		}

		return true;
	}

	// no unicode support
	bool Reader::parse_string()
	{
		int c = _stream.peek();
		if(c == '\"')
		{
			_stream.get();

			std::stringstream ss;
			for(c = _stream.peek(); c != EOF; c = _stream.peek())
			{
				switch(c)
				{
				case '\"':
					str = ss.str();
					_stream.get();
					return true;
				case '\\':
					_stream.get();
					c = _stream.peek();
					switch(c)
					{
					case '\"' : ss << '\"'; break;
					case '\\' : ss << '\\'; break;
					case '/' : ss << '/'; break;
					case 'b': ss << '\b'; break;
					case 'f': ss << '\f'; break;
					case 'n': ss << '\n'; break;
					case 'r': ss << '\r'; break;
					case 't': ss << '\t'; break;
					default: return false;
					}
					_stream.get();
					break;
				default:
					ss << (char)c;
					_stream.get();
					break;
				}
			}
		}
		return false;
	}

	bool Reader::parse_null()
	{
		char buff[4] = {0};
		for(size_t k = 0; k < 4; k++)
		{
			int c = _stream.get();
			if(c == EOF)
			{
				return false;
			}
			else
			{
				buff[k] = (char)c;
			}
		}
		return strncmp(buff, "null", 4) == 0;
	}

	Token_t Reader::parse_value()
	{
		char c = _stream.peek();
		
		if(c == '{')
		{
			_stream.get();
			scope s = {0};
			s.need_name = true;
			_scope_stack.push_back(s);
			return Token::BeginObject;
		}
		else if(c == '[')
		{
			_stream.get();
			scope s = {0};
			s.need_element = true;
			_scope_stack.push_back(s);

			return Token::BeginArray;
		}
		else if(c == 't' || c == 'f')
		{
			return parse_bool() ? Token::Bool : Token::Invalid;
		}
		else if(c == '-' || (c >= '0' && c <= '9'))
		{
			return parse_number() ? Token::Number : Token::Invalid;
		}
		else if(c == '\"')
		{
			return parse_string() ? Token::String : Token::Invalid;
		}
		else if(c == 'n')
		{
			return parse_null() ? Token::Null : Token::Invalid;
		}

		return Token::Invalid;
	}

	Reader::Reader(std::istream& stream)
	: _stream(stream)
	, _error(false)
	{ }

	Token_t Reader::next()
	{
		if(_error) return Token::Invalid;

		skip(_stream);
		char c = _stream.peek();
		
		// we're in an array or object
		if(_scope_stack.size() > 0)
		{
			size_t index = _scope_stack.size() - 1;
			// need a name token
			if(_scope_stack[index].need_name)
			{
				if(c == ',')
				{
					if(_scope_stack[index].pair_count == 0)
					{
						_error = true;
						return Token::Invalid;
					}
					_stream.get();
					skip(_stream);
					c = _stream.peek();
				}
				else if(c == '}')
				{
					_stream.get();
					_scope_stack.pop_back();
					return Token::EndObject;
				}
				else if(_scope_stack[index].pair_count > 0)
				{
					_error = true;
					return Token::Invalid;
				}

				if(c == '\"')
				{
					if(parse_string())
					{
						_scope_stack[index].need_name = false;
						_scope_stack[index].need_value = true;

						skip(_stream);
						c = _stream.peek();
						if(c== ':')
						{
							_stream.get();
							return Token::ValueName;
						}
					}
					_error = true;
					return Token::Invalid;
				}
			}
			// need a value token
			else if(_scope_stack[index].need_value)
			{
				Token_t tok = parse_value();
				if(tok != Token::Invalid)
				{
					_scope_stack[index].need_name = true;
					_scope_stack[index].need_value = false;
					_scope_stack[index].pair_count++;
				}
				else
				{
					_error = true;
				}
				return tok;
			}
			// need the next array element
			else if(_scope_stack[index].need_element)
			{
				if(c == ',')
				{
					// comma only appears if element count is positive
					if(_scope_stack[index].element_count == 0)
					{
						_error = true;
						return Token::Invalid;
					}
					_stream.get();
				}
				else if(c == ']')
				{
					_stream.get();
					_scope_stack.pop_back();
					return Token::EndArray;
				}
				else if(_scope_stack[index].element_count > 0)
				{
					_error = true;
					return Token::Invalid;
				}

				skip(_stream);
				Token_t tok = parse_value();
				if(tok == Token::Invalid)
				{
					_error = true;
				}
				_scope_stack[index].element_count++;

				return tok;
			}
		}
		Token_t tok = parse_value();
		if(tok == Token::Invalid)
		{
			_error = true;
		}
		return tok;
	}

	bool Reader::readBoolean()
	{
		return b;
	}
	double Reader::readDouble()
	{
		return d;
	}
	int64_t Reader::readInt()
	{
		return i;
	}
	uint64_t Reader::readUInt()
	{
		return u;
	}
	std::string Reader::readString()
	{
		return str;
	}

/// Writer

	// no unicode support
	static void write_string(const char* name, std::ostream& stream)
	{
		stream << '\"';
		while(*name)
		{
			// regular printable character
			if(*name > 31 && *name!='\"' && *name != '\\')
			{
				stream << *name;
			}
			// escaped
			else
			{
				stream << '\\';
				switch(*name)
				{
				case '\"': stream << '\"'; break;	
				case '\\': stream << '\\';  break;
				case '/': stream << '/';  break;
				case '\b': stream << 'b'; break;
				case '\f': stream << 'f'; break;
				case '\n': stream << 'n'; break;
				case '\r': stream << 'r'; break;
				case '\t': stream << 't'; break;
				}
			}
			name++;
		}
		stream << '\"';
	}

	Writer::Writer(std::ostream& stream, bool pretty_format)
	 : _pretty_format(pretty_format)
	 , _stream(stream)
	 , _depth(0)
	{ }

	void Writer::begin_object()
	{
		if(_scope_stack.size() > 0)
		{
			const scope& s = _scope_stack.back();
			assert(s.need_value || s.need_element);
		}

		begin_write();

		if(_pretty_format && _scope_stack.size() > 0)
		{
			const scope& prev_s = _scope_stack.back();
			if(prev_s.need_value)
			{
				_stream << '\n';
				for(size_t k = 0; k < _depth; k++)
				{
					_stream << '\t';
				}
			}
			else if(prev_s.need_element)
			{
				if(prev_s.element_count > 0)
				{
					_stream << '\n';
					for(size_t k = 0; k < _depth; k++)
					{
						_stream << '\t';
					}
				}
			}
		}

		_stream << '{';

		scope s = {0};
		s.begin_object = true;

		_scope_stack.push_back(s);
		_depth++;

		end_write();
	}

	void Writer::end_object()
	{
		assert(_scope_stack.size() > 0);
		const scope& s = _scope_stack.back();
		assert(s.need_name == true);

		_depth--;

		if(_pretty_format)
		{
			_stream << '\n';
			for(size_t k = 0; k < _depth; k++)
			{
				_stream << '\t';
			}
		}
		
		_stream << '}';

		_scope_stack.pop_back();
		end_write();
	}

	void Writer::begin_array()
	{
		if(_scope_stack.size() > 0)
		{
			const scope& s = _scope_stack.back();
			assert(s.need_value || s.need_element);
		}

		begin_write();

		if(_pretty_format && _scope_stack.size() > 0)
		{
			const scope& prev_s = _scope_stack[_scope_stack.size() - 1];
			if(prev_s.need_element)
			{
				if(prev_s.element_count > 0)
				{
					_stream << '\n';
					for(size_t k = 0; k < _depth; k++)
					{
						_stream << '\t';
					}
				}
			}
		}

		_depth++;
		_stream << '[';

		scope s = {0};
		s.begin_array = true;

		_scope_stack.push_back(s);

		end_write();
	}

	void Writer::end_array()
	{
		assert(_scope_stack.size() > 0);
		const scope& s = _scope_stack.back();
		assert(s.need_element);

		_depth--;
		_stream << ']';

		_scope_stack.pop_back();

		end_write();
	}

	void Writer::write_name(const char* name)
	{
		assert(name != nullptr);

		assert(_scope_stack.size() > 0);
		scope& s = _scope_stack.back();
		assert(s.need_name == true);
		assert(s.need_value == false);
		assert(s.need_element == false);
		assert(s.element_count == 0);

		begin_write();

		write_string(name, _stream); 

		end_write();
	}

	void Writer::write(bool value)
	{
		begin_write();

		if(value)
		{
			_stream << "true";
		}
		else
		{
			_stream << "false";
		}

		end_write();
	}

	void Writer::write(int64_t value)
	{
		begin_write();

		_stream << value;

		end_write();
	}

	void Writer::write(uint64_t value)
	{
		begin_write();

		_stream << value;

		end_write();
	}

	void Writer::write(double value)
	{
		begin_write();

		char buff[64];

		if (fabs(floor(value)-value)<=DBL_EPSILON && fabs(value)<1.0e60) sprintf(buff,"%.0f",value);
		else if (fabs(value)<1.0e-6 || fabs(value)>1.0e9)                sprintf(buff,"%e",value);
		else                                                             sprintf(buff,"%f",value);

		_stream << buff;

		end_write();

	}

	void Writer::write(const char* value)
	{
		begin_write();

		write_string(value, _stream);

		end_write();
	}

	void Writer::write(nullptr_t value)
	{
		begin_write();

		_stream << "null";

		end_write();
	}

	void Writer::begin_write()
	{
		if(_scope_stack.size() > 0)
		{
			const scope& s = _scope_stack.back();
			if(s.need_element)
			{
				if(s.element_count > 0)
				{
					if(_pretty_format)
					{
						_stream << ", ";
					}
					else
					{
						_stream << ',';
					}
				}
			}
			else if(s.need_name)
			{
				if(s.pair_count > 0)
				{
					if(_pretty_format)
					{
						_stream << ",\n";
						for(size_t k = 0; k < _depth; k++)
						{
							_stream << '\t';
						}
					}
					else
					{
						_stream << ',';
					}
				}
				else
				{
					if(_pretty_format)
					{
						_stream << "\n";
						for(size_t k = 0; k < _depth; k++)
						{
							_stream << '\t';
						}
					}
				}
			}
			else if(s.need_value)
			{
				if(_pretty_format)
				{
					_stream << " : ";
				}
				else
				{
					_stream << ':';
				}
			}
			else
			{
				assert(false);
			}
		}
	}

	void Writer::end_write()
	{
		if(_scope_stack.size() > 0)
		{
			scope& s = _scope_stack.back();
			
			if(s.begin_object)
			{
				assert(s.begin_array == false);
				assert(s.need_name == false);
				assert(s.need_value == false);
				assert(s.pair_count == 0);
				assert(s.need_element == false);
				assert(s.element_count == 0);

				s.need_name = true;
				s.begin_object = false;
			}
			else if(s.begin_array)
			{
				assert(s.begin_object == false);
				assert(s.need_name == false);
				assert(s.need_value == false);
				assert(s.pair_count == 0);
				assert(s.need_element == false);
				assert(s.element_count == 0);

				s.need_element = true;
				s.begin_array = false;
			}
			else if(s.need_name)
			{
				assert(s.begin_object == false);
				assert(s.begin_array == false);
				assert(s.need_value == false);
				assert(s.need_element == false);
				assert(s.element_count == 0);

				s.need_value = true;
				s.need_name = false;
			}
			else if(s.need_value)
			{
				assert(s.begin_object == false);
				assert(s.begin_array == false);
				assert(s.need_name == false);
				assert(s.need_element == false);
				assert(s.element_count == 0);

				s.need_value = false;
				s.need_name = true;
				s.pair_count++;
			}
			else if(s.need_element)
			{
				assert(s.begin_object == false);
				assert(s.begin_array == false);
				assert(s.need_name == false);
				assert(s.need_value == false);
				assert(s.pair_count == 0);

				s.element_count++;
			}
		}
	}
}
