#pragma once

#include <stdio.h>
#include <stdint.h>

#pragma warning ( push )
#pragma warning (disable : 4480)

enum Endianness : uint16_t
{ 
	BigEndian = 0x0000,
	LittleEndian = 0xFFFF
};
enum DataType : uint8_t
{
	UInt8 = 0x08,
	SInt8 = 0x09,
	SInt16 = 0x0B,
	SInt32 = 0x0C,
	Single = 0x0D,
	Double = 0x0E
};


#pragma warning (pop)

class IDX
{
private:
	// info about the file
	uint32_t _rows;
	uint32_t _row_length;
	Endianness _idx_endianness;
	Endianness _system_endianness;
	DataType _file_data;

	// bookkeeping data
	FILE* _idx_file;
	uint32_t _header_size;
	uint32_t _row_length_bytes;
public:
	IDX(const char* filename);
	~IDX();
	
	uint32_t Rows() const {return _rows;};
	void ReadRow(uint32_t row, void* dest);
	inline uint32_t RowLength() const {return _row_length;};
	inline DataType GetDataType() const {return _file_data;};
private:
	
	// binary reading methods
	template <typename T>
	T read(FILE* in_file)
	{
		T result;
		if(sizeof(T) != 1 && _idx_endianness != _system_endianness)
		{
			uint8_t* buffer = (uint8_t*)&result;
			for(int i = sizeof(T) - 1; i >= 0; i--)
				buffer[i] = fgetc(in_file);
		}
		else
			fread(&result, sizeof(T), 1, in_file);

		return result;
	}

	template<typename T>
	T* read_row(FILE* in_file, T* row)
	{
		if(row)
		{

			if(sizeof(T) != 1 && _idx_endianness != _system_endianness)
			{
				T val;
				uint8_t* byte_buffer = (uint8_t*)&val;
				for(uint32_t k = 0; k < _row_length; k++)
				{
					for(int i = sizeof(T) - 1; i >= 0; i--)
						byte_buffer[i] = fgetc(in_file);
					row[k] = val;
				}
			}
			else
				fread(row, sizeof(T), _row_length, in_file);
		}

		return row;
	}
};