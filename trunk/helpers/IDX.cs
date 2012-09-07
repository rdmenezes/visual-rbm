using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Diagnostics;

public enum Endianness : ushort
{
	BigEndian = 0x0000,
	LittleEndian = 0xFFFF
}

public enum DataFormat : byte
{
	UInt8 = 0x08,
	SInt8 = 0x09,
	SInt16 = 0x0B,
	SInt32 = 0x0C,
	Single = 0x0D,
	Double = 0x0E
}

public class IDX
{
	// file data
	uint _rows;
	uint _row_length;
	Endianness _idx_endianness;
	static Endianness _system_endianness = BitConverter.IsLittleEndian ? Endianness.LittleEndian : Endianness.BigEndian;
	DataFormat _data_format;

	// bookkeeping
	Stream _idx_stream;
	uint _header_size;
	uint _row_length_bytes;
	bool _writing;

	#region READS

	byte ReadUInt8()
	{
		int result = _idx_stream.ReadByte();
		Debug.Assert(result >= 0 && result <= 255);
		return (byte)result;
	}

	unsafe uint ReadUInt32()
	{
		const int ALLOC = 4;

		byte* buff = stackalloc byte[ALLOC];

		if (_idx_endianness == _system_endianness)
		{
			buff[0] = ReadUInt8();
			buff[1] = ReadUInt8();
			buff[2] = ReadUInt8();
			buff[3] = ReadUInt8();
		}
		else
		{
			buff[3] = ReadUInt8();
			buff[2] = ReadUInt8();
			buff[1] = ReadUInt8();
			buff[0] = ReadUInt8();
		}

		return *(uint*)buff;
	}


	#endregion
	
	#region WRITES

	void WriteUInt8(byte b)
	{
		_idx_stream.WriteByte(b);
	}

	unsafe void WriteUInt32(uint i)
	{
		byte* buff = (byte*)&i;

		if (_idx_endianness == _system_endianness)
		{
			_idx_stream.WriteByte(buff[0]);
			_idx_stream.WriteByte(buff[1]);
			_idx_stream.WriteByte(buff[2]);
			_idx_stream.WriteByte(buff[3]);
		}
		else
		{
			_idx_stream.WriteByte(buff[3]);
			_idx_stream.WriteByte(buff[2]);
			_idx_stream.WriteByte(buff[1]);
			_idx_stream.WriteByte(buff[0]);		
		}
	}

	#endregion

	// constructor for reading an IDX file 
	public IDX(Stream in_stream, bool in_writing=false)
	{
		_idx_stream = in_stream;
		// verify we can actually use this stream
		Debug.Assert(_idx_stream.CanRead && _idx_stream.CanSeek);

		_writing = in_writing;
		if (_writing)
		{
			// just verify stream is writeable
			Debug.Assert(_idx_stream.CanWrite == true);
		}


		// get our endianness
		_idx_endianness = (Endianness)(255 * ReadUInt8() + ReadUInt8());
		Debug.Assert(_idx_endianness == Endianness.BigEndian || _idx_endianness == Endianness.LittleEndian);

		// get data format
		_data_format = (DataFormat)ReadUInt8();

		// number of dimensions
		byte dimensions = ReadUInt8();
		Debug.Assert(dimensions > 0);
		// get number of rows
		_rows = ReadUInt32();

		// calculate length of each row
		_row_length = 1;
		for (int k = 1; k < dimensions; k++)
		{
			_row_length *= ReadUInt32();
		}

		_row_length_bytes = _row_length;
		switch (_data_format)
		{
			case DataFormat.UInt8:
			case DataFormat.SInt8:
				_row_length_bytes *= 1;
				break;
			case DataFormat.SInt16:
				_row_length_bytes *= 2;
				break;
			case DataFormat.SInt32:
			case DataFormat.Single:
				_row_length_bytes *= 4;
				break;
			case DataFormat.Double:
				_row_length_bytes *= 8;
				break;
		}

		// get size of the header
		_header_size = (uint)_idx_stream.Position;
	}

	// constructor for writing IDX objects
	public IDX(Stream in_stream, Endianness in_endianness, DataFormat in_format, params uint[] row_dimensions)
	{
		_writing = true;
		_idx_stream = in_stream;
		// debug we can use this stream
		Debug.Assert(_idx_stream.CanWrite && _idx_stream.CanRead && _idx_stream.CanSeek);

		// set endianness
		_idx_endianness = in_endianness;
		// make sure valid endianness
		Debug.Assert(_idx_endianness == Endianness.BigEndian || _idx_endianness == Endianness.LittleEndian);

		// get the data format
		_data_format = in_format;
		// make sure it's a valid format
		Debug.Assert(_data_format == DataFormat.UInt8 ||
			_data_format == DataFormat.SInt8 ||
			_data_format == DataFormat.SInt16 ||
			_data_format == DataFormat.SInt32 ||
			_data_format == DataFormat.Single ||
			_data_format == DataFormat.Double);

		// verify row dimensions are all valid
		Debug.Assert(row_dimensions.Length > 1 && row_dimensions.Length < 255);
		_row_length = 1;
		for (int i = 0; i < row_dimensions.Length; i++)
		{
			_row_length *= row_dimensions[i];
			Debug.Assert(row_dimensions[i] > 0);
		}

		_row_length_bytes = _row_length;
		switch (_data_format)
		{
			case DataFormat.UInt8:
			case DataFormat.SInt8:
				_row_length_bytes *= 1;
				break;
			case DataFormat.SInt16:
				_row_length_bytes *= 2;
				break;
			case DataFormat.SInt32:
			case DataFormat.Single:
				_row_length_bytes *= 4;
				break;
			case DataFormat.Double:
				_row_length_bytes *= 8;
				break;
		}

		// write out IDX header

		// endianness first
		if (_idx_endianness == Endianness.BigEndian)
		{
			WriteUInt8(0x00);
			WriteUInt8(0x00);
		}
		else
		{
			WriteUInt8(0xFF);
			WriteUInt8(0xFF);
		}

		// write data format
		WriteUInt8((byte)_data_format);
		// write number of dimensions
		WriteUInt8((byte)(1 + row_dimensions.Length));
		// number of rows is 0 until we actually add rows
		WriteUInt32(0);
		foreach (uint i in row_dimensions)
		{
			WriteUInt32(i);
		}

		_header_size = (uint)_idx_stream.Position;
	}



	public uint Rows
	{
		get
		{
			return _rows;
		}

	}

	public uint RowLength
	{
		get
		{
			return _row_length;
		}
	}

	public DataFormat DataType
	{
		get
		{
			return _data_format;
		}
	}

	public uint AddRow()
	{
		Debug.Assert(_writing == true);		// can only add rows when we're writing
		Debug.Assert(_rows != uint.MaxValue);	// prevent overflow

		_idx_stream.Position = _header_size + _row_length_bytes * _rows;
		for (uint i = 0; i < _row_length_bytes; i++)
		{
			_idx_stream.WriteByte((byte)0);
		}

		return _rows++;	// returns index of new row
	}

	public void AddRows(uint count)
	{
		Debug.Assert(_writing == true);	// can only add rows when writing
		Debug.Assert(count != 0);	// verify count is positive
		Debug.Assert(_rows + count > _rows);	// make sure we don't overflow

		_idx_stream.Position = _header_size + _row_length_bytes * _rows;
		for (uint k = 0; k < count; k++)
		{
			for (uint i = 0; i < _row_length; i++)
			{
				_idx_stream.WriteByte((byte)0);
			}
		}
		_rows += count;
	}
	#region Row Reads

	public void ReadRow(uint index, byte[] buffer)
	{
		Debug.Assert(index < _rows);
		Debug.Assert(_data_format == DataFormat.UInt8);

		_idx_stream.Position = _header_size + _row_length_bytes * index;

		_idx_stream.Read(buffer, 0, (int)_row_length);
	}

	public void ReadRow(uint index, sbyte[] buffer)
	{
		Debug.Assert(index < _rows);
		Debug.Assert(_data_format == DataFormat.SInt8);

		_idx_stream.Position = _header_size + _row_length_bytes * index;

		for (uint k = 0; k < _row_length; k++)
		{
			buffer[k] = (sbyte)ReadUInt8();
		}
	}

	unsafe public void ReadRow(uint index, short[] buffer)
	{
		Debug.Assert(index < _rows);
		Debug.Assert(_data_format == DataFormat.SInt16);

		_idx_stream.Position = _header_size + _row_length_bytes * index;

		byte* temp = stackalloc byte[2];
		short* val = (short*)temp;

		if (_idx_endianness == _system_endianness)
		{
			for (uint k = 0; k < _row_length; k++)
			{
				temp[0] = ReadUInt8();
				temp[1] = ReadUInt8();

				buffer[k] = *val;
			}
		}
		else
		{
			for (uint k = 0; k < _row_length; k++)
			{
				temp[1] = ReadUInt8();
				temp[0] = ReadUInt8();

				buffer[k] = *val;
			}
		}
	}

	unsafe public void ReadRow(uint index, int[] buffer)
	{
		Debug.Assert(index < _rows);
		Debug.Assert(_data_format == DataFormat.SInt32);

		_idx_stream.Position = _header_size + _row_length_bytes * index;

		byte* temp = stackalloc byte[4];
		int* val = (int*)temp;

		if (_idx_endianness == _system_endianness)
		{
			for (uint k = 0; k < _row_length; k++)
			{
				temp[0] = ReadUInt8();
				temp[1] = ReadUInt8();
				temp[2] = ReadUInt8();
				temp[3] = ReadUInt8();

				buffer[k] = *val;
			}
		}
		else
		{
			for (uint k = 0; k < _row_length; k++)
			{
				temp[3] = ReadUInt8();
				temp[2] = ReadUInt8();
				temp[1] = ReadUInt8();
				temp[0] = ReadUInt8();

				buffer[k] = *val;
			}
		}
	}

	unsafe public void ReadRow(uint index, float[] buffer)
	{
		Debug.Assert(index < _rows);
		Debug.Assert(_data_format == DataFormat.Single);

		_idx_stream.Position = _header_size + _row_length_bytes * index;

		byte* temp = stackalloc byte[4];
		float* val = (float*)temp;

		if (_idx_endianness == _system_endianness)
		{
			for (uint k = 0; k < _row_length; k++)
			{
				temp[0] = ReadUInt8();
				temp[1] = ReadUInt8();
				temp[2] = ReadUInt8();
				temp[3] = ReadUInt8();

				buffer[k] = *val;
			}
		}
		else
		{
			for (uint k = 0; k < _row_length; k++)
			{
				temp[3] = ReadUInt8();
				temp[2] = ReadUInt8();
				temp[1] = ReadUInt8();
				temp[0] = ReadUInt8();

				buffer[k] = *val;
			}
		}
	}

	unsafe public void ReadRow(uint index, double[] buffer)
	{
		Debug.Assert(index < _rows);
		Debug.Assert(_data_format == DataFormat.Double);
		
		_idx_stream.Position = _header_size + _row_length_bytes * index;

		byte* temp = stackalloc byte[4];
		float* val = (float*)temp;

		if (_idx_endianness == _system_endianness)
		{
			for (uint k = 0; k < _row_length; k++)
			{
				temp[0] = ReadUInt8();
				temp[1] = ReadUInt8();
				temp[2] = ReadUInt8();
				temp[3] = ReadUInt8();
				temp[4] = ReadUInt8();
				temp[5] = ReadUInt8();
				temp[6] = ReadUInt8();
				temp[7] = ReadUInt8();

				buffer[k] = *val;
			}
		}
		else
		{
			for (uint k = 0; k < _row_length; k++)
			{
				temp[7] = ReadUInt8();
				temp[6] = ReadUInt8();
				temp[5] = ReadUInt8();
				temp[4] = ReadUInt8();
				temp[3] = ReadUInt8();
				temp[2] = ReadUInt8();
				temp[1] = ReadUInt8();
				temp[0] = ReadUInt8();

				buffer[k] = *val;
			}
		}
	}

	#endregion

	#region Row Writes

	public void WriteRow(uint index, byte[] buffer)
	{
		Debug.Assert(buffer.Length == _row_length);	// right amount of data
		Debug.Assert(_data_format == DataFormat.UInt8);	// right data format
		Debug.Assert(index < _rows);	// valid index
		Debug.Assert(_writing == true);	// make sure we can actually write

		_idx_stream.Position = _header_size + _rows * _row_length_bytes;

		_idx_stream.Write(buffer, 0, (int)_row_length);
		_rows++;
	}

	unsafe public void WriteRow(int index, sbyte[] in_buffer)
	{
		Debug.Assert(in_buffer.Length == _row_length);	// right amount of data
		Debug.Assert(_data_format == DataFormat.SInt8);	// right data format
		Debug.Assert(index < _rows);	// valid index
		Debug.Assert(_writing == true);	// make sure we can actually write

		_idx_stream.Position = _header_size + _row_length_bytes * index;

		fixed(sbyte* buffer = in_buffer)
		{
			byte* raw = (byte*)buffer;
			for (int k = 0; k < _row_length; k++)
			{
				WriteUInt8(raw[k]);
			}
		}
	}

	unsafe public void WriteRow(uint index, short[] in_buffer)
	{
		Debug.Assert(in_buffer.Length == _row_length);	// right amount of data
		Debug.Assert(_data_format == DataFormat.SInt16);	// right data format
		Debug.Assert(index < _rows);	// valid index
		Debug.Assert(_writing == true);	// make sure we can actually write

		fixed(short* buffer = in_buffer)
		{
			byte* raw = (byte*)buffer;

			_idx_stream.Position = _header_size + _row_length_bytes * index;

			if (_idx_endianness == _system_endianness)
			{
				for (int k = 0; k < _row_length_bytes; k++)
				{
					WriteUInt8(raw[k]);
				}
			}
			// byteswap
			else
			{
				for (int k = 0; k < _row_length; k++)
				{
					WriteUInt8(raw[1]);
					WriteUInt8(raw[0]);
					raw += 2;
				}
			}
		}
	}

	unsafe public void WriteRow(uint index, int[] in_buffer)
	{
		Debug.Assert(in_buffer.Length == _row_length);	// right amount of data
		Debug.Assert(_data_format == DataFormat.SInt32);	// right data format
		Debug.Assert(index < _rows);	// valid index
		Debug.Assert(_writing == true);	// make sure we can actually write

		fixed (int* buffer = in_buffer)
		{
			byte* raw = (byte*)buffer;

			_idx_stream.Position = _header_size + _row_length_bytes * index;

			if (_idx_endianness == _system_endianness)
			{
				for (int k = 0; k < _row_length_bytes; k++)
				{
					WriteUInt8(raw[k]);
				}
			}
			// byteswap
			else
			{
				for (int k = 0; k < _row_length; k++)
				{
					WriteUInt8(raw[3]);
					WriteUInt8(raw[2]);
					WriteUInt8(raw[1]);
					WriteUInt8(raw[0]);
					raw += 4;
				}
			}
		}
	}

	unsafe public void WriteRow(uint index, float[] in_buffer)
	{
		Debug.Assert(in_buffer.Length == _row_length);	// right amount of data
		Debug.Assert(_data_format == DataFormat.Single);	// right data format
		Debug.Assert(index < _rows);	// valid index
		Debug.Assert(_writing == true);	// make sure we can actually write

		fixed (float* buffer = in_buffer)
		{
			byte* raw = (byte*)buffer;

			_idx_stream.Position = _header_size + _row_length_bytes * index;

			if (_idx_endianness == _system_endianness)
			{
				for (int k = 0; k < _row_length_bytes; k++)
				{
					WriteUInt8(raw[k]);
				}
			}
			// byteswap
			else
			{
				for (int k = 0; k < _row_length; k++)
				{
					WriteUInt8(raw[3]);
					WriteUInt8(raw[2]);
					WriteUInt8(raw[1]);
					WriteUInt8(raw[0]);
					raw += 4;
				}
			}
		}

		_rows++;
	}

	unsafe public void WriteRow(uint index, double[] in_buffer)
	{
		Debug.Assert(in_buffer.Length == _row_length); // right amount of data
		Debug.Assert(_data_format == DataFormat.Double);	// right data format
		Debug.Assert(index < _rows);	// valid index
		Debug.Assert(_writing == true);	// make sure we can actually write

		fixed (double* buffer = in_buffer)
		{
			byte* raw = (byte*)buffer;

			_idx_stream.Position = _header_size + _row_length_bytes * index;

			if (_idx_endianness == _system_endianness)
			{
				for (int k = 0; k < _row_length_bytes; k++)
				{
					WriteUInt8(raw[k]);
				}
			}
			// byteswap
			else
			{
				for (int k = 0; k < _row_length; k++)
				{
					WriteUInt8(raw[7]);
					WriteUInt8(raw[6]);
					WriteUInt8(raw[5]);
					WriteUInt8(raw[4]);
					WriteUInt8(raw[3]);
					WriteUInt8(raw[2]);
					WriteUInt8(raw[1]);
					WriteUInt8(raw[0]);
					raw += 8;
				}
			}
		}
	}

#endregion
}
