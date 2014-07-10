import sys
import struct
import os

class Endianness:
	big_endian = 0x0000
	little_endian = 0xFFFF

class DataFormat:
	invalid = 0x00
	uint8 = 0x08
	sint8 = 0x09
	sint16 = 0x0b
	sint32 = 0x0c
	single = 0x0d
	double = 0x0e

# an IDX file object
class IDX:

# private:
	def __init__(self):
		self.__file = None
		self.__writable = False
		self.__row_length = 0
		self.__row_length_bytes = 0
		self.__row_dimensions_count = 0
		self.__row_dimensions = None
		self.__endianness = Endianness.big_endian
		self.__data_format = DataFormat.invalid

	def __enter__(self):
		return self
		

	def __exit__(self, type, value, traceback):
		self.close()

	def __iter__(self):
		self.__current_index = 0
		return self

	def next(self):
		if self.__current_index == self.row_count():
			raise StopIteration
		else:
			result = self.read_row(self.__current_index)
			self.__current_index += 1
			return result


# public:
	# close an idx file
	def close(self):
		if self.__file != None:

			# write the number of rows we have
			if self.__writable == True:
				self.__file.seek(4)
				self.__file.write(struct.pack("%sI" % self.__endianness_string, self.row_count()))

			self.__file.close()
			self.__file = None

	# load an IDX file from disk
	@staticmethod				
	def load(filename, writable=False):
		try:
			f = open(filename, "r+b") if writable else open(filename, "rb")

			# create result and store off file object
			idx = IDX()
			idx.__file = f
			idx.__writable = writable

			# read and verify endianness
			idx.__endianness = struct.unpack("H",f.read(2))[0]
			if idx.__endianness == Endianness.big_endian:
				idx.__endianness_string = ">"
			elif idx.__endianness == Endianness.little_endian:
				idx.__endianness_string = "<"
			else:
				print "Invalid Endianness bytes found in IDX header"
				return None

			# read and verify the data format
			idx.__data_format = struct.unpack("B", f.read(1))[0]

			if idx.__data_format == DataFormat.uint8:
				idx.__data_format_string = "B"
			elif idx.__data_format == DataFormat.sint8:
				idx.__data_format_string = "b"
			elif idx.__data_format == DataFormat.sint16:
				idx.__data_format_string = "h"
			elif idx.__data_format == DataFormat.sint32:
				idx.__data_format_string = "i"
			elif idx.__data_format == DataFormat.single:
				idx.__data_format_string = "f"
			elif idx.__data_format == DataFormat.double:
				idx.__data_format_string = "d"
			else:
				print "Invalid Data Format byte found in IDX header"
				return None

			# read number of dimensions
			idx.__row_dimensions_count = struct.unpack("B", f.read(1))[0]

			# read in our row dimensions
			idx.__row_dimensions = struct.unpack("%s%iL" % (idx.__endianness_string, idx.__row_dimensions_count), f.read(4 * idx.__row_dimensions_count))

			# calculate our row size in bytes
			idx.__row_length = 1
			for k in range(1, len(idx.__row_dimensions)):
				idx.__row_length *= idx.__row_dimensions[k]

			if idx.__data_format == DataFormat.uint8 or idx.__data_format == DataFormat.sint8:
				idx.__row_length_bytes = idx.__row_length * 1
			elif idx.__data_format == DataFormat.sint16:
				idx.__row_length_bytes = idx.__row_length * 2
			elif idx.__data_format == DataFormat.sint32 or idx.__data_format == DataFormat.single:
				idx.__row_length_bytes = idx.__row_length *4
			elif idx.__data_format == DataFormat.double:
				idx.__row_length_bytes = idx.__row_length * 8

			# get our header size
			idx.__header_size = f.tell()

			# verify file size
			f.seek(0, os.SEEK_END)
			measured_file_size = f.tell()
			expected_file_size = idx.__row_dimensions[0] * idx.__row_length_bytes + idx.__header_size
			if measured_file_size != expected_file_size:
				print "Incorrect file size given the header\n  measured: %i bytes\n  expected: %i bytes" % (measured_file_size, expected_file_size)
				return None

			# save off our pack/unpack format string
			idx.__format_string = "%s%i%s" % (idx.__endianness_string, idx.__row_length, idx.__data_format_string)

			return idx
		except Exception as ex:
			print ex
			return None
		
	# create a new IDX file for writing
	@staticmethod
	def create(filename, endianness, data_format, *row_dimensions):
		try:
			f = open(filename, "w+b")

			# create new IDX 
			idx = IDX()		
			idx.__file = f	
			idx.__writable = True

			# set the endianness
			if endianness == Endianness.big_endian:
				idx.__endianness_string = ">"
			elif endianness == Endianness.little_endian:
				idx.__endianness_string = "<"
			else:
				print "Invalid Endianness specified"
				return None
			idx.__endianness = endianness

			# set the data format

			if data_format == DataFormat.uint8:
				idx.__data_format_string = "B"
			elif data_format == DataFormat.sint8:
				idx.__data_format_string = "b"
			elif data_format == DataFormat.sint16:
				idx.__data_format_string = "h"
			elif data_format == DataFormat.sint32:
				idx.__data_format_string = "i"
			elif data_format == DataFormat.single:
				idx.__data_format_string = "f"
			elif data_format == DataFormat.double:
				idx.__data_format_string = "d"
			else:
				print "Invalid Data Format byte found in IDX header"
				return None	
			idx.__data_format = data_format

			# set the row dimensions (row count is 0 initially)
			idx.__row_dimensions = []
			idx.__row_dimensions.append(0)
			for i in row_dimensions:
				idx.__row_dimensions.append(i)
			idx.__row_dimensions_count = len(idx.__row_dimensions)

			# calculate our row size in bytes
			idx.__row_length = 1
			for k in range(1, len(idx.__row_dimensions)):
				idx.__row_length *= idx.__row_dimensions[k]

			if idx.__data_format == DataFormat.uint8 or idx.__data_format == DataFormat.sint8:
				idx.__row_length_bytes = idx.__row_length * 1
			elif idx.__data_format == DataFormat.sint16:
				idx.__row_length_bytes = idx.__row_length * 2
			elif idx.__data_format == DataFormat.sint32 or idx.__data_format == DataFormat.single:
				idx.__row_length_bytes = idx.__row_length *4
			elif idx.__data_format == DataFormat.double:
				idx.__row_length_bytes = idx.__row_length * 8

			# write our header to disk
			f.write(struct.pack("H", idx.__endianness))
			f.write(struct.pack("B", idx.__data_format))
			f.write(struct.pack("B", idx.__row_dimensions_count))
			for dim in idx.__row_dimensions:
				f.write(struct.pack("%sI" % idx.__endianness_string, dim))

			# get our header size
			idx.__header_size = f.tell()

			# save off our pack/unpack format string
			idx.__format_string = "%s%i%s" % (idx.__endianness_string, idx.__row_length, idx.__data_format_string)

			return idx
		except Exception as ex:
			print ex
			return None



	def row_count(self):
		return self.__row_dimensions[0]

	# read in a row at a given index
	def read_row(self, index):

		# verify this is a valid index
		if index < 0 or index >= self.row_count():
			return None

		# seek to location
		offset = self.__header_size + self.__row_length_bytes * index
		self.__file.seek(offset)

		# read in row to tuple and return
		result = struct.unpack(self.__format_string, self.__file.read(self.__row_length_bytes))
		
		return result

	# append new row to the IDX file
	def add_row(self, new_row):

		# file must be writable
		if self.__writable == False:
			return False

		# incoming row must be the full length
		if len(new_row) != self.__row_length:
			return False

		# seek to end of file
		self.__file.seek(0, os.SEEK_END)

		# write row to disk
		self.__file.write(struct.pack(self.__format_string, *new_row))

		# update our row count
		self.__row_dimensions[0] += 1

		return True

	def write_row(self, index, new_row):

		# file must be writable
		if self.__writable == False:
			return False

		# verify this is a valid index
		if index < 0 or index >= self.row_count():
			return False

		# incoming row must be the full length
		if len(new_row) != self.__row_length:
			return False

		# seek to location
		offset = self.__header_size + self.__row_length_bytes * index
		self.__file.seek(offset)

		# write to disk
		self.__file.write(struct.pack(self.__format_string, *new_row))
