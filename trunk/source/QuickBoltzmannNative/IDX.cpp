#include <stdio.h>
#include <assert.h>

#include "IDX.h"


inline Endianness GetSystemEndianness()
{
	union
	{
		uint32_t u32;
		uint16_t u16[2];
	};
	u32 = 0x0000FFFF;
	return (Endianness)u16[0];
}



IDX::IDX(const char* in_filename)
{
	_idx_file = fopen(in_filename, "rb");
	// make sure we could open the file
	assert(_idx_file != 0);

	// get the systems endianness so that we can determine if we need to 
	// byte swap the data
	_system_endianness = GetSystemEndianness();	

	_idx_endianness = (Endianness)read<uint16_t>(_idx_file);
	
	// figure out our filetype
	_file_data = (DataType)fgetc(_idx_file);
	// read number of dimensions
	uint8_t dimensions = read<uint8_t>(_idx_file);
	// first number is always the number of rows
	_rows = read<uint32_t>(_idx_file);
	_row_length = 1;
	for(int k = 1; k < dimensions; k++)
		_row_length *= read<uint32_t>(_idx_file);

	_header_size = 4 + 4 * dimensions;
	switch(_file_data)
	{
	case UInt8:
	case SInt8:
		_row_length_bytes = _row_length;
		break;
	case SInt16:
		_row_length_bytes = _row_length * 2;
		break;
	case SInt32:
	case Single:
		_row_length_bytes = _row_length * 4;
		break;
	case Double:
		_row_length_bytes = _row_length * 8;
		break;
	}
}

IDX::~IDX()
{
	fclose(_idx_file);
	_idx_file = 0;
}

void IDX::ReadRow(uint32_t row, void* buffer)
{
	fseek(_idx_file, _header_size + row * _row_length_bytes, SEEK_SET);
	switch(_file_data)
	{
	case UInt8:
	case SInt8:
		fread(buffer, 1, _row_length, _idx_file);
		break;
	case SInt16:
		read_row<int16_t>(_idx_file, (int16_t*)buffer);
		break;
	case SInt32:
		read_row<int32_t>(_idx_file, (int32_t*)buffer);
		break;
	case Single:
		read_row<float>(_idx_file, (float*)buffer);
		break;
	case Double:
		read_row<double>(_idx_file, (double*)buffer);
		break;
	}
}