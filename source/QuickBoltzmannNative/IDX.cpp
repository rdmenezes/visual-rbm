#include <stdio.h>
#include <assert.h>

#include "IDX.h"




IDX::IDX()
: _rows(0)
, _row_length(0)
, _idx_endianness(Endianness::BigEndian)
, _file_data(DataFormat::Invalid)
, _idx_file(NULL)
, _header_size(0)
, _row_length_bytes(0)
{ }

IDX* IDX::Load(const char* in_filename)
{
	FILE* file = fopen(in_filename, "rb");

	// make sure we could open the file
	if(file == NULL)
	{
		return NULL;
	}

	IDX* result = new IDX();
	IDX& idx = *result;

	idx._idx_file = file;	

	idx._idx_endianness = (Endianness)idx.read<uint16_t>();
	
	// figure out our filetype
	idx._file_data = (DataFormat)idx.read<uint8_t>();
	// read number of dimensions
	uint8_t dimensions = idx.read<uint8_t>();
	// first number is always the number of rows
	idx._rows = idx.read<uint32_t>();
	idx._row_length = 1;
	for(int k = 1; k < dimensions; k++)
		idx._row_length *= idx.read<uint32_t>();

	idx._header_size = 4 + 4 * dimensions;
	switch(idx._file_data)
	{
	case UInt8:
	case SInt8:
		idx._row_length_bytes = idx._row_length;
		break;
	case SInt16:
		idx._row_length_bytes = idx._row_length * 2;
		break;
	case SInt32:
	case Single:
		idx._row_length_bytes = idx._row_length * 4;
		break;
	case Double:
		idx._row_length_bytes = idx._row_length * 8;
		break;
	}

	return result;
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
		read_row<int16_t>((int16_t*)buffer);
		break;
	case SInt32:
		read_row<int32_t>((int32_t*)buffer);
		break;
	case Single:
		read_row<float>((float*)buffer);
		break;
	case Double:
		read_row<double>((double*)buffer);
		break;
	}
}