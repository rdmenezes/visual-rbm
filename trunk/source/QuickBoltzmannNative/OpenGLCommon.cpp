#include <stdint.h>
#include <stdio.h>

#include "OpenGLCommon.h"

// everyone uses the same shader
GLint vertex_shader_handle = -1;
// framebuffer handle
GLuint FrameBuffer;
GLuint VertexArrayObject;
GLuint VertexBufferObject;

// has opengl context been created
bool ContextCreated = false;
int WindowId;


bool StartupOpenGL()
{
	if(ContextCreated)
		return true;

	// opengl initialization stuff
	static char* name = "QuickBoltzmannNative";
	static int32_t count = 1;

	glutInit(&count, &name);
	// creates context as new as it can by default
	WindowId = glutCreateWindow(name);

	int major, minor;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);

	int version = major * 10 + minor;

	if(version != 33)	// version 3.3
	{
		glutDestroyWindow(WindowId);
		if(version < 33)
		{
			return false;
		}
		else
		{
			// newer context, rollback to old
			glutInitContextVersion(3, 3);
			WindowId = glutCreateWindow(name);
		}
	}

	ContextCreated = true;
	// we have to set this to true or else we won't get all the functions we need
	glewExperimental=TRUE;
	glewInit();

	// some bookkeeping
	glPolygonMode(GL_FRONT, GL_FILL);
	glEnable(GL_DEPTH_TEST);
	// build our framebuffer
	glGenFramebuffers(1, &FrameBuffer);
	// set up render target framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, FrameBuffer);

	// construct our vertex arrays
	glGenVertexArrays(1, &VertexArrayObject);
	glBindVertexArray(VertexArrayObject);
	// generate the buffer
	glGenBuffers(1, &VertexBufferObject);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableClientState(GL_VERTEX_ARRAY);

	return true;
}

void ShutdownOpenGL()
{
	if(ContextCreated)
	{
		ContextCreated = false;
		glutDestroyWindow(WindowId);
	}
}

struct SourceBuffer
{
	GLchar* Data;
	GLint Length;

	SourceBuffer() : Data(0), Length(0) { }

	~SourceBuffer()
	{
		if(Data)
		{
			delete[] Data;
			Data = 0;
			Length = 0;
		}
	}
};

void LoadFile(const char* filename, SourceBuffer& db)
{
	// open file fro reading
	FILE* file = fopen(filename, "rb");
	// verify it was opened
	assert(file != 0);
	// get the length of the file
	fseek(file, 0, SEEK_END);
	db.Length = ftell(file);
	rewind(file);

	// read file into buffer
	db.Data = new GLchar[db.Length];
	fread(db.Data, 1, db.Length, file);
	fflush(file);
	fclose(file);
}

GLint BuildVertexShader(const char* filename)
{
	SourceBuffer vertex_source;
	LoadFile(filename, vertex_source);

	// load the vertex shader
	int shader_handle = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(shader_handle, 1, (const GLchar**)&vertex_source.Data, &vertex_source.Length);
	glCompileShader(shader_handle);
	
	GLint compile_status = -1;
	glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &compile_status);
	assert(compile_status == GL_TRUE );
	
	return shader_handle;
}

GLint BuildFragmentProgram(const char* filename, GLint& size_handle, GLint& depth_handle)
{
	SourceBuffer fragment_source;
	LoadFile(filename, fragment_source);
	
	// create shader from source
	GLint shader_handle = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(shader_handle, 1, (const GLchar**)&fragment_source.Data, &fragment_source.Length);
	glCompileShader(shader_handle);
	
	// verify that it compiled ok
	GLint compile_status = -1;
	glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &compile_status);
	assert(compile_status == GL_TRUE );
	
	GLint program_handle = glCreateProgram();


	// attach the fragment shader to the program
	glAttachShader(program_handle, shader_handle);
	if(vertex_shader_handle == -1)
		vertex_shader_handle = BuildVertexShader("NativeShaders/calc_texture_coordinates.vert");
	// also attach the vertex shader
	glAttachShader(program_handle, vertex_shader_handle);

	// link
	glLinkProgram(program_handle);

	// get the location for width and height params (for vertex shader)
	//width_handle = glGetUniformLocation(program_handle, "width");
	//height_handle = glGetUniformLocation(program_handle, "height");

	size_handle = glGetUniformLocation(program_handle, "size");
	depth_handle = glGetUniformLocation(program_handle, "depth");

	return program_handle;
}


GLuint AllocateFloatTexture(uint32_t rows, uint32_t columns, float* initial_data)
{
	GLuint result;
	glGenTextures(1, &result);

	glBindTexture(GL_TEXTURE_RECTANGLE, result);

	// use nearest neighbor sampling
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// clamp uv coordinates
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP);

	if(initial_data == NULL)
	{
		initial_data = new float[rows * columns];
		memset(initial_data, 0, sizeof(float) * rows * columns);
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R32F, columns, rows, 0, GL_RED, GL_FLOAT, initial_data);
		delete[] initial_data;
	}
	else
	{
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R32F, columns, rows, 0, GL_RED, GL_FLOAT, initial_data);
	}

	

	glBindTexture(GL_TEXTURE_RECTANGLE, 0);
	
	return result;
}

GLuint AllocateUInt32Texture(uint32_t rows, uint32_t columns, uint32_t* initial_data)
{
	GLuint result;
	glGenTextures(1, &result);

	glBindTexture(GL_TEXTURE_RECTANGLE, result);

	// use nearest neighbor sampling
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// clamp uv coordinates
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP);

	if(initial_data == NULL)
	{
		initial_data = new uint32_t[rows * columns];
		memset(initial_data, 0, sizeof(uint32_t) * rows * columns);
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R32UI, columns, rows, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, initial_data);
	}
	else
	{
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R32UI, columns, rows, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, initial_data);
	}

	glBindTexture(GL_TEXTURE_RECTANGLE, 0);

	return result;
}

GLuint AllocateUInt8Texture(uint32_t rows, uint32_t columns, uint8_t* initial_data)
{
	GLuint result;
	glGenTextures(1, &result);

	glBindTexture(GL_TEXTURE_RECTANGLE, result);

	// use nearest neighbor sampling
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// clamp uv coordinates
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP);

	if(initial_data == NULL)
	{
		initial_data = new uint8_t[rows * columns];
		memset(initial_data, 0, sizeof(uint8_t) * rows * columns);
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R8UI, columns, rows, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, initial_data);
	}
	else
	{
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_R8UI, columns, rows, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, initial_data);
	}

	glBindTexture(GL_TEXTURE_RECTANGLE, 0);

	return result;
}

void ReleaseTextures(GLuint* tex_head, uint32_t count)
{
	glDeleteTextures(count, tex_head);
}

void PrintError()
{
	GLenum error = glGetError();

	printf("%s: %x\n", gluErrorString(error), error);
}

GLuint depth_buffer;
void BindDepthBuffer(uint32_t rows, uint32_t columns)
{
	glGenRenderbuffers(1, &depth_buffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, columns, rows);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
	
	glEnable(GL_DEPTH_TEST);
	glDepthRange(0.0f, 1.0f);
}

void DeleteDepthBuffer()
{
	glDeleteRenderbuffers(1, &depth_buffer);
}