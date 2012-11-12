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
enum DataFormat : uint8_t
{
	Invalid = 0x00,
	UInt8 = 0x08,
	SInt8 = 0x09,
	SInt16 = 0x0B,
	SInt32 = 0x0C,
	Single = 0x0D,
	Double = 0x0E
};

inline const Endianness SystemEndianness()
{
	union
	{
		uint32_t u32;
		uint16_t u16[2];
	};
	u32 = 0x0000FFFF;
	return (Endianness)u16[0];
}

#pragma warning (pop)

class IDX
{
private:
	// info about the file
	uint32_t _rows;
	uint32_t _row_length;
	Endianness _idx_endianness;
	DataFormat _file_data;

	// bookkeeping data
	FILE* _idx_file;
	uint32_t _header_size;
	uint32_t _row_length_bytes;
public:
	static IDX* Load(const char* filename);
	~IDX();
	
	uint32_t Rows() const {return _rows;};
	void ReadRow(uint32_t row, void* dest);
	inline uint32_t RowLength() const {return _row_length;};
	inline DataFormat GetDataType() const {return _file_data;};
private:
	
	IDX();

	// binary reading methods
	template <typename T>
	T read()
	{
		T result;
		if(sizeof(T) != 1 && _idx_endianness != SystemEndianness())
		{
			uint8_t* buffer = (uint8_t*)&result;
			for(int i = sizeof(T) - 1; i >= 0; i--)
				buffer[i] = fgetc(_idx_file);
		}
		else
			fread(&result, sizeof(T), 1, _idx_file);

		return result;
	}

	template<typename T>
	T* read_row(T* row)
	{
		if(row)
		{
			if(sizeof(T) != 1 && _idx_endianness != SystemEndianness())
			{
				T val;
				uint8_t* byte_buffer = (uint8_t*)&val;
				for(uint32_t k = 0; k < _row_length; k++)
				{
					for(int i = sizeof(T) - 1; i >= 0; i--)
						byte_buffer[i] = fgetc(_idx_file);
					row[k] = val;
				}
			}
			else
				fread(row, sizeof(T), _row_length, _idx_file);
		}

		return row;
	}
};