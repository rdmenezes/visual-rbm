#include <math.h>

#include <IDX.hpp>

const char Usage[] = 
	"Parses the given CSV file as single precision floats into an IDX file.\n"
	"\n"
	"Usage: csv2idx [INPUT] [OUTPUT]\n"
	"  INPUT   A CSV file (with no header) delimitted with commas\n"
	"  OUTPUT  Desintation to save IDX file\n";

enum
{
	NoError = 0,
	InvalidCharacter,
	InconsistentRowLength,
	InvalidNumberParse,
} error;
size_t line_length = 1024;
uint8_t* line_buffer = NULL;
int line_number = 0;


// returns true if line read (no invalid chars)
// false on error or end of file

bool read_line(FILE* file, uint8_t*& line, size_t& length)
{
	if(line_buffer == NULL)
	{
		line_buffer = (uint8_t*)malloc(line_length);
	}

	// update the line we're on
	++line_number;
	int column_number = 0;

	length = 0;
	for(int32_t b = fgetc(file); b >= 0; b = fgetc(file))
	{
		// update the column we're on
		++column_number;

		// double buffer size if we need more memory
		if(length == line_length)
		{
			line_length *= 2;
			uint8_t* new_line_buffer = (uint8_t*)malloc(line_length);
			memcpy(new_line_buffer, line_buffer, length);
			free(line_buffer);
			line_buffer = new_line_buffer;
		}

		// end of line get out
		if(b == '\n')
		{
			line_buffer[length++] = 0;
			break;
		}
		// ignore other whitespace
		else if(b == '\r' || b == '\t' || b == ' ')
		{
			continue;
		}
		else
		{
			if( b == '-' || b == '+' ||
				(b >= '0' && b <= '9') ||
				b == '.' || 
				b == 'e' || b == 'E' ||
				b == ',')
			{
				line_buffer[length++] = (uint8_t)b;
			}
			else
			{
				error = InvalidCharacter;
				// printable character
				if(b >= 0x20 && b < 0x7F)
				{
					printf("Error: Problem reading line, found invalid character on line %i, column %i: \"%c\"\n", line_number, column_number, (char)b);
				}
				else
				{
					printf("Error: Problem reading line, found invalid byte on line %i, column %i: %02x\n", line_number, column_number,  b);
				}
				return false;
			}
			
		}
	}

	// end of file found
	if(length == 0)
	{
		line = NULL;
		return false;
	}
	
	// set the buffer
	line = line_buffer;
	// replace last printable character with \0 if it is a comma
	if(line[length - 1 - 1] == ',')
	{
		line[length - 1 - 1] = 0;
		length--;
	}
	return true;
}

// [+-]?[0-9]*(.[0-9]+)?([eE][+-]?[0-9]+)?
// this is a constrained float parsing method, no infinity/nan/etc allowed
bool parse_float(const char* buf, float& result)
{
	enum
	{
		Sign,
		Integer,
		Fraction,
		ExponentSign,
		Exponent,
	} state = Sign;

	int sign = 1;
	double integer = 0.0;
	int exponent_sign = 1;
	int exponent = 0;

	while(*buf != 0)
	{
		switch(state)
		{
		case Sign:
			if(*buf == '+')
			{
				sign = 1;
			}
			else if(*buf == '-')
			{
				sign = -1;
			}
			else if(*buf >= '0' && *buf <= '9')
			{
				integer = (*buf - '0');
			}
			else
			{
				return false;
			}
			state = Integer;
			break;
		case Integer:
			if(*buf == '.')
			{
				state = Fraction;
			}
			else if(*buf == 'e' || *buf == 'E')
			{
				state = ExponentSign;
			}
			else if(*buf >= '0' && *buf <= '9')
			{
				integer = integer * 10.0 + (double)(*buf - '0');
			}
			else
			{
				return false;
			}
			break;
		case Fraction:
			if(*buf == 'e' || *buf == 'E')
			{
				state = ExponentSign;
			}
			else if(*buf >= '0' && *buf <= '9')
			{
				integer = integer * 10.0 + (double)(*buf - '0');
				--exponent;
			}
			else
			{
				return false;
			}
			break;
		case ExponentSign:
			if(*buf == '+')
			{
				state = Exponent;
				exponent_sign = 1;
			}
			else if(*buf == '-')
			{
				state = Exponent;
				exponent_sign = -1;
			}
			else if(*buf >= '0' && *buf <= '9')
			{
				state = Exponent;
				exponent += exponent_sign * (int)(*buf - '0');
			}
			else
			{
				return false;
			}
			break;
		case Exponent:
			if(*buf >= '0' && *buf <= '9')
			{
				exponent += exponent_sign * (int)(*buf - '0');
			}
			else
			{
				return false;
			}
			break;
		}

		++buf;
	}

	result = (float)(sign * integer * pow(10.0, exponent));

	return true;
}

bool parse_line(uint8_t* line_buffer, size_t line_length, float*& row_buffer, int& count)
{
	// count the number of items in this row
	int new_count = 0;
	for(size_t i = 0; i < line_length; i++)
	{
		new_count += line_buffer[i] == ',' ? 1 : 0;
	}
	// one less comma than items
	++new_count;

	// allocate memory
	if(row_buffer == NULL && count == -1)
	{
		count = new_count;
		row_buffer = new float[count];
	}
	// or error
	else if(count != new_count)
	{
		printf("Error: Inconsistent number of cells on line %i; found %i cells but the first row had %i cells\n", line_number, new_count, count);
		error = InconsistentRowLength;
		return false;
	}

	int begin_index = -1;
	int end_index;

	for(int k = 0; k < count; k++)
	{
		for(end_index = begin_index + 1; line_buffer[end_index] != ',' && line_buffer[end_index] != 0; ++end_index);
		line_buffer[end_index] = 0;	// null teriminate this bit
		// parse
		float val = -1;
		const char* str = (char*)(line_buffer + begin_index + 1);

		if(parse_float(str, val) == false)
		{
			printf("Error: Problem parsing value \"%s\" as number on line %i, column %i\n", str, line_number, (begin_index + 2));
			error = InvalidNumberParse;
			return false;
		}

		row_buffer[k] = (float)val;

		begin_index = end_index;
	}

	return true;
}

int main(int argc, char** argv)
{
	int result = -1;

	float* row_buffer = NULL;
	int columns = -1;
	FILE* file = NULL;
	IDX* idx = NULL;

	if(argc != 3)
	{
		printf(Usage);
		goto ERROR;
	}

	uint8_t* line = NULL;
	size_t line_length = 0;

	file = fopen(argv[1], "rb");
	
	printf("Reading rows and writing IDX ...\n");
	while(read_line(file, line, line_length) && parse_line(line, line_length, row_buffer, columns))
	{
		if(idx == NULL)
		{
			idx = IDX::Create(argv[2], LittleEndian, Single, columns);
		}
		idx->AddRow(row_buffer);
	}
	
	if(error != NoError)
	{
		goto ERROR;
	}
	
	result = 0;
	printf("Done!\n");
ERROR:

	if(file)
	{
		fclose(file);
		file = NULL;
	}

	if(idx)
	{
		idx->Close();
		delete idx;
		idx = NULL;
	}


	return result;
}