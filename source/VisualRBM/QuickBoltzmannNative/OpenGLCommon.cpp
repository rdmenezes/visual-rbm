#include <stdint.h>
#include <stdio.h>

#include "Common.h"
#include "OpenGLCommon.h"
#include "Shaders.h"

// everyone uses the same shader
GLint vertex_shader_handle = -1;
// framebuffer handle
GLuint FrameBuffer;
GLuint VertexArrayObject;
GLuint VertexBufferObject;

// has opengl context been created
bool ContextCreated = false;
int WindowId;

uint32_t* AllocationCounter = NULL;

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

	while(glGetError() != GL_NO_ERROR);
	

	// some bookkeeping
	glPolygonMode(GL_FRONT_AND_BACK , GL_FILL);

	glEnable(GL_DEPTH_TEST);

	// construct our vertex arrays
	glGenVertexArrays(1, &VertexArrayObject);
	glBindVertexArray(VertexArrayObject);
	// generate the buffer
	glGenBuffers(1, &VertexBufferObject);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);


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

GLint BuildVertexShader(const char* source)
{
	int source_length = strlen(source);

	// load the vertex shader
	int shader_handle = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(shader_handle, 1, (const GLchar**)&source, &source_length);
	glCompileShader(shader_handle);
	
	GLint compile_status = -1;
	glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &compile_status);
	ASSERT(compile_status == GL_TRUE );
	
	return shader_handle;
}

GLint BuildFragmentProgram(const char* source, GLint& size_handle, GLint& depth_handle)
{
	int source_length = strlen(source);
	
	// create shader from source
	GLint shader_handle = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(shader_handle, 1, (const GLchar**)&source, &source_length);
	glCompileShader(shader_handle);
	
	// verify that it compiled ok
	GLint compile_status = -1;
	glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &compile_status);
	ASSERT(compile_status == GL_TRUE );
	
	GLint program_handle = glCreateProgram();


	// attach the fragment shader to the program
	glAttachShader(program_handle, shader_handle);
	if(vertex_shader_handle == -1)
		vertex_shader_handle = BuildVertexShader(calc_texture_coordinates);
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

void RegisterAllocationCounter(uint32_t* ptr)
{
	AllocationCounter = ptr;
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
	
	if(AllocationCounter)
	{
		*AllocationCounter += rows * columns * sizeof(float);
	}

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

	if(AllocationCounter)
	{
		*AllocationCounter += rows * columns * sizeof(uint32_t);
	}

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
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

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

	if(AllocationCounter)
	{
		*AllocationCounter += rows * columns * sizeof(uint8_t);
	}

	return result;
}

void ReleaseTextures(GLuint*& tex_head, uint32_t count)
{
	if(tex_head)
	{
		glDeleteTextures(count, tex_head);
		delete[] tex_head;
		tex_head = NULL;
	}
}

void PrintError()
{
	GLenum error = glGetError();

	printf("%s: %x\n", gluErrorString(error), error);
}

RenderTarget::RenderTarget()
	: _rows(0)
	, _columns(0)
	, _frame_buffer(-1)
	, _depth_buffer(-1)
{

}

void RenderTarget::Reset()
{
	if(_depth_buffer != -1)
	{
		glDeleteRenderbuffers(1, &_depth_buffer);
		_depth_buffer = -1;
	}

	if(_frame_buffer != -1)
	{
		glDeleteFramebuffers(1, &_frame_buffer);
		_frame_buffer = -1;
	}

	_rows = 0;
	_columns = 0;
}

void RenderTarget::Generate(uint32_t Rows, uint32_t Columns)
{
	ASSERT(_rows == 0 && _columns == 0 && _frame_buffer == -1 && _depth_buffer == -1);

	_rows = Rows;
	_columns = Columns;

	// generate framebuffer
	glGenFramebuffers(1, &_frame_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer);

	// generate depthbuffer for the render targets framebuffer
	glGenRenderbuffers(1, &_depth_buffer);
	glBindRenderbuffer(GL_RENDERBUFFER, _depth_buffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _columns, _rows);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depth_buffer);

	glEnable(GL_DEPTH_TEST);
	glDepthRange(0.0f, 1.0f);
}

void RenderTarget::SetTarget(GLuint Target)
{
	_target_texture = Target;
}

void RenderTarget::BindTarget()
{
	glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, _target_texture, 0);
}