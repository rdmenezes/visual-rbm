// enum represents the endianness of the IDX file
var Endianness = 
{
	"BigEndian" : 0x0000,
	"LittleEndian" : 0xFFFF
};

// format of the binary data of the IDX file
var DataFormat =
{
	"Invalid" : 0x00,
	"UInt8" : 0x08,
	"SInt8" : 0x09,
	"SInt16" : 0x0B,
	"SInt32" : 0x0C,
	"Single" : 0x0D,
	"Double" : 0x0E,
};

// helpe renum that denotes size (in bytes) of each data format
var DataSize =
{
	"Invalid" : 0,
	"UInt8" : 1,
	"SInt8" : 1,
	"SInt16" : 2,
	"SInt32" : 4,
	"Single" : 4,
	"Double" : 8,
};


// file -> a File object received from a file input element
// loaded_callback -> required user supplied callback that is triggered once we have
// finished reading in hte IDX header info; loaded_callback takes as input the IDX object
// that was loaded
function IDX(file, loaded_callback)
{
	var idx = this;

	idx._read_request_queue = new Array();
	
	idx._file = file;
	idx._reader = new FileReader();
	idx._loaded_callback = loaded_callback;
	
	// read the first part (endianness, and data format)
	var blob = idx._file.slice(0, 4);

	// callback reads in the first bits of header info (endianness, data format, number of dimensions)
	idx._reader.onloadend = function(head_evt)
	{
		if (head_evt.target.readyState == FileReader.DONE)
		{
			var raw = new Uint8Array(head_evt.target.result);
			idx._endianness = raw[0] * 256 + raw[1];
			idx._dataformat = raw[2];
			idx._dimension_count = raw[3];
			
			console.log(idx._file.name + " IDX header:");
			switch(idx._endianness)
			{
				case Endianness.BigEndian:
					console.log(" Endianness: Big Endian (Network Byte Order)");
					break;
				case Endianness.LittleEndian:
					console.log(" Endianness: Little Endian (Intel)");
					break;
				default:
					console.log(" Endianness: Invalid");
					break;
			}
			
			var data_format_name = "Invalid";
			for(var key in DataFormat)
			{
				if(idx._dataformat == DataFormat[key])
				{
					idx._dataformat_name = key;
					break;
				}
			}
			console.log(" DataFormat: " + idx._dataformat_name);
			console.log(" Dimensions: " + idx._dimension_count);

			idx._header_size = 4 + DataSize.SInt32 * idx._dimension_count;
			
			// now read in the list of dimensions
			var blob = idx._file.slice(4, idx._header_size);
			idx._dimensions = new Array();
			
			// load callback reads in the dimensions from the header
			idx._reader.onloadend = function(dim_evt)
			{
				idx._row_length = 1;
				var data_view = new DataView(head_evt.target.result);
				for(var i = 0; i < idx._dimension_count; i++)
				{
					idx._dimensions[i] = idx.readSInt32(data_view, i);
					if(i > 0)
					{
						idx._row_length *= idx._dimensions[i];
					}
					else
					{
						idx._row_count = idx._dimensions[0];
					}
					console.log("  Dim " + i + ": " + idx._dimensions[i]);
				}
				console.log(" RowLength: " + idx._row_length);
				idx._row_size = idx._row_length * DataSize[idx._dataformat_name];
				console.log(" RowSize: " + idx._row_size + " bytes");
				
				// once the dimensions have been read in, we can now call the loaded callback
				// to notify user IDX file is ready for use
				idx._loaded_callback(idx);
			}
			idx._reader.readAsArrayBuffer(blob);
		}
	};
	
	idx._reader.readAsArrayBuffer(blob);
}

// IDX read methods (don't call these)
IDX.prototype.readUInt8 = function(data_view, i)
{
	return data_view.getUint8(i);
}

IDX.prototype.readSInt8 = function(data_view, i)
{
	return data_view.getInt8(i);
}

IDX.prototype.readSInt16 = function(data_view, i)
{
	return data_view.getInt16(i * DataSize.SInt16, this._endianness == Endianness.LittleEndian);
}

IDX.prototype.readSInt32 = function(data_view, i)
{
	return data_view.getInt32(i * DataSize.SInt32, this._endianness == Endianness.LittleEndian);
}

IDX.prototype.readSingle = function(data_view, i)
{
	return data_view.getFloat32(i * DataSize.Single, this._endianness == Endianness.LittleEndian);
}	

IDX.prototype.readDouble = function(data_view, i)
{
	return data_view.getFloat64(i * DataSize.Double, this._endianness == Endianness.LittleEndian);
}		

// jump table for ReadRow 
var ReadFunc =
{
	"UInt8" : IDX.prototype.readUInt8,
	"SInt8" : IDX.prototype.readSInt8,
	"SInt16" : IDX.prototype.readSInt16,
	"SInt32" : IDX.prototype.readSInt32,
	"Single" : IDX.prototype.readSingle,
	"Double" : IDX.prototype.readDouble,
};

// index -> index of the IDX file we want to load
// read_callback -> called on successful read of the row; read_callback takes 2 parameters:
//	- An array of values from the read in row
//	- The index of the row loaded
IDX.prototype.ReadRow = function(in_index, in_read_callback)
{
	// only 1 read request can execute at once (or else javascript complains)
	// so we have to explcitily queue up read requests and daisy chain
	// reads
	
	this._read_request_queue.push(
	{
		index : in_index,
		read_callback : in_read_callback
	});
	
	// did we just add the only entry?
	if(this._read_request_queue.length == 1)
	{
		this.ConsumeReadQueue();
	}
}

IDX.prototype.ConsumeReadQueue = function()
{
	var idx = this;

	var index = idx._read_request_queue[0].index;
	var read_callback = idx._read_request_queue[0].read_callback;
	
	// get the begin of the row, and the end of the row
	var begin = idx._header_size + index * idx._row_size;
	var end = idx._header_size + (index + 1)* idx._row_size;
	
	// define the blob for this row
	var blob = idx._file.slice(begin, end);
	
	// define a new callback function which copies the read data to an Array, and
	// then passes said result Array to a user provided callback
	idx._reader.onloadend = function(evt)
	{
		var data_view = new DataView(evt.target.result);
		var read_func = ReadFunc[idx._dataformat_name];
		var row = new Array();
		
		for(var i = 0; i < idx._row_length; i++)
		{
			row.push(read_func.call(idx, data_view, i));
		}
		
		read_callback(row, index);
		
		idx._read_request_queue.shift();
		if(idx._read_request_queue.length > 0)
		{
			idx.ConsumeReadQueue();
		}
	}
	// initiate read
	idx._reader.readAsArrayBuffer(blob);
}