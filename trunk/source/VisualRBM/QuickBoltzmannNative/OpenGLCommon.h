#pragma once

#include <assert.h>
// opengl and friends
#include <GL/glew.h>
#include <GL/freeglut.h>

extern bool StartupOpenGL();
extern void ShutdownOpenGL();
// returns handle to the compiled fragment program
extern GLint BuildFragmentProgram(const char* source, GLint& size_handle, GLint& depth_handle);
// framebuffer handle
extern GLuint FrameBuffer;
extern GLuint VertexArrayObject;
extern GLuint VertexBufferObject;

extern void BindDepthBuffer(uint32_t rows, uint32_t columns);
extern void DeleteDepthBuffer();

// allocate textures
extern GLuint AllocateFloatTexture(uint32_t rows, uint32_t columns, float* initial_data=0);
extern GLuint AllocateUInt8Texture(uint32_t rows, uint32_t columns, uint8_t* initial_data=0);
extern GLuint AllocateUInt32Texture(uint32_t rows, uint32_t columns, uint32_t* initial_data=0);

extern void ReleaseTextures(GLuint* tex_head, uint32_t count);
extern void PrintError();

enum ParamType
{
	Int,
	Float,
	Texture,
	Vec2
};

// E needs to be an enum ranging from 0 to Count
template<typename T>
struct Shader
{
	Shader() : _texture_handle_counter(0), _inputs_registered(0), _depth(0.5)
	{
	}

	void Build(const char* source, const char* output_location)
	{
		_program = BuildFragmentProgram(source, _size_location, _depth_location);
		glBindFragDataLocation(_program, 0, output_location);
	} 

	void RegisterParameter(decltype(T::Count) e, const char* in_name, ParamType in_type)
	{
		// increment number of inputs registered
		assert(_inputs_registered < T::Count);
		_inputs_registered++;

		_input[e].Type = in_type;
		_input[e].ParamLocation = glGetUniformLocation(_program, in_name);
		if(_input[e].Type == Texture)
		{
			// assign this texture param a texture unit to use
			assert(_texture_handle_counter < 32);
			_input[e].TextureUnit = GL_TEXTURE0 + _texture_handle_counter++;
		}
		
	}

	void SetDepth(float Depth)
	{
		_depth = Depth;
	}

	void SetRenderTargetSize(int32_t Width, int32_t Height)
	{
		_rendertarget_width = Width;
		_rendertarget_height = Height;
	}

	void SetParam(decltype(T::Count) e, GLuint tex)
	{
		assert(_input[e].Type == Texture);
		_input[e].TextureHandle = tex;
	}

	void SetParam(decltype(T::Count) e, int32_t i)
	{
		assert(_input[e].Type == Int);
		_input[e].Integer = i;
	}

	void SetParam(decltype(T::Count) e, float f)
	{
		assert(_input[e].Type == Float);
		_input[e].FloatingPoint = f;
	}

	void SetParam(decltype(T::Count) e, float x, float y)
	{
		assert(_input[e].Type == Vec2);
		_input[e].X = x;
		_input[e].Y = y;
	}

	void Run(int32_t Width, int32_t Height, GLint RenderDestination)
	{
		Run(0, 0, Width, Height, RenderDestination);
	}

	void Run(int32_t XOffset, int32_t YOffset, int32_t Width, int32_t Height, GLint RenderDestination)
	{
		// set to use program
		glUseProgram(_program);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, RenderDestination, 0);
		// set up viewport
		glViewport(0, 0, _rendertarget_width, _rendertarget_height);

		// set width/height parameters (same for all shaders)
		glUniform2f(_size_location, (float)_rendertarget_width, (float)_rendertarget_height);
		glUniform1f(_depth_location, _depth);

		// iterate over shader parameters and set them up
		for(uint32_t k = 0; k < _params; k++)
		{
			Input& i = _input[k];
			switch(i.Type)
			{
			case Float:
				glUniform1f(i.ParamLocation, i.FloatingPoint);
				break;
			case Int:
				glUniform1i(i.ParamLocation, i.Integer);
				break;
			case Texture:
				// bind the texture to the appropriate unit
				glActiveTexture(i.TextureUnit);
				glBindTexture(GL_TEXTURE_RECTANGLE, i.TextureHandle);
				glUniform1i(i.ParamLocation, i.TextureUnit - GL_TEXTURE0);
				break;
			case Vec2:
				glUniform2f(i.ParamLocation, i.X, i.Y);
				break;
			}
		}

		float vertices[] =
		{
			(float)XOffset,(float)YOffset,
			(float)(XOffset + Width),(float)YOffset,
			(float)(XOffset + Width),(float)(YOffset + Height),
			(float)XOffset,(float)(YOffset + Height)
		};

		// bind vbo
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);
		// copy data to buffer
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		// bind to 0 slot
		glEnableVertexAttribArray(0);

		// draw!
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

		// disbale vvertex array		
		glDisableVertexAttribArray(0);
		  
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// back out this program
		glUseProgram(0);

	}

private:
	struct Input
	{
		ParamType Type;
		GLint ParamLocation;

#pragma warning (push)
#pragma warning (disable : 4201)
		union
		{
			int32_t Integer;
			float FloatingPoint;
			struct  
			{
				int32_t TextureUnit;
				GLint TextureHandle;
			};
			struct
			{
				float X;
				float Y;
			};
		};
#pragma warning (pop)
	};

	// handle for our program
	const static uint32_t _params = T::Count;
	GLint _program;
	Input _input[_params + 1];
	
	GLint _size_location;
	GLint _depth_location;

	GLint _rendertarget_width;
	GLint _rendertarget_height;

	float _depth;

	int32_t _texture_handle_counter;
	int32_t _inputs_registered;
};