#pragma once

#include "SiCKL.h"

#include <sstream>
#include <set>
#include <string>


namespace SiCKL
{
	class OpenGLRuntime
	{
	public:
		// setup the opengl runtime
		static bool Initialize();
		// and tear it down
		static bool Finalize();
		static int32_t GetMaxTextureSize();
		static const int32_t* GetMaxViewportSize();
		static int32_t GetVertexShader();
		static uint32_t RequiredBufferSpace(uint32_t width, uint32_t height, ReturnType::Type type);

		friend class OpenGLCompiler;
	};

	// sample with texelFetch (see page 95 of 
	// http://www.opengl.org/registry/doc/GLSLangSpec.3.30.6.pdf )
	
	
	struct OpenGLBuffer1D
	{
		OpenGLBuffer1D();

		OpenGLBuffer1D(int32_t length, ReturnType::Type type, void* data);
		~OpenGLBuffer1D();
		OpenGLBuffer1D(const OpenGLBuffer1D&);
		OpenGLBuffer1D& operator=(const OpenGLBuffer1D&);

		uint32_t GetBufferSize() const;

		void SetData(void* in_buffer);

		const int32_t Length;
		const ReturnType::Type Type;
		const uint32_t BufferHandle;
		const uint32_t TextureHandle;
	private:
		int32_t* _counter;
		// uses GL_TEXTURE_BUFFER
	};

	struct OpenGLBuffer2D
	{
		OpenGLBuffer2D();
		OpenGLBuffer2D(int32_t width, int32_t height, ReturnType::Type type, void* data);
		~OpenGLBuffer2D();
		OpenGLBuffer2D(const OpenGLBuffer2D&);
		OpenGLBuffer2D& operator=(const OpenGLBuffer2D&);
		
		uint32_t GetBufferSize() const;
		
		template<typename T>
		inline void GetData(T*& in_out_buffer)
		{
			get_data((void**)&in_out_buffer);
		}

		void SetData(void* in_buffer);

		const int32_t Width;
		const int32_t Height;
		const ReturnType::Type Type;
		const uint32_t TextureHandle;
	private:
		void get_data(void** in_out_buffer) const;

		int32_t* _counter;
		// uses GL_TEXTURE_RECTANGLE
	};

	/// handles for setting inputs and outputs for OpenGL Programs
	typedef int32_t input_t;
	typedef int32_t output_t;
	class OpenGLProgram : public Program
	{
	public:
		virtual ~OpenGLProgram();
		// sets up framebuffer and vertex buffer
		void Initialize(int32_t width, int32_t height);

		input_t GetInputHandle(const char*);
		output_t GetOutputHandle(const char*);
		// inputs to shader
		void SetInput(input_t, bool);
		void SetInput(input_t, int32_t);
		void SetInput(input_t, uint32_t);
		void SetInput(input_t, float);
		// vec2s
		void SetInput(input_t, int32_t, int32_t);
		void SetInput(input_t, uint32_t, uint32_t);
		void SetInput(input_t, float, float);
		// vec3s
		void SetInput(input_t, int32_t, int32_t, int32_t);
		void SetInput(input_t, uint32_t, uint32_t, uint32_t);
		void SetInput(input_t, float, float, float);
		// vec4s
		void SetInput(input_t, int32_t, int32_t, int32_t, int32_t);
		void SetInput(input_t, uint32_t, uint32_t, uint32_t, uint32_t);
		void SetInput(input_t, float, float, float, float);

		void SetInput(input_t, const OpenGLBuffer1D&);
		void SetInput(input_t, const OpenGLBuffer2D&);


		// outputs
		void BindOutput(output_t, const OpenGLBuffer2D&);

		// read output buffer back to CPU memory
		template<typename T>
		inline void GetOutput(output_t o, T*& in_out_buffer)
		{
			get_output(o, 0, 0, _size[0], _size[1], (void**)&in_out_buffer);
		}

		template<typename T>
		inline void GetSubOutput(output_t o, int32_t offset_x, int32_t offset_y, int32_t width, int32_t height, T*& in_out_buffer)
		{
			get_output(o, offset_x, offset_y, width, height, (void**)&in_out_buffer);
		}

		virtual void Run();
		const  std::string& GetSource() const {return _source;}
	private:
		OpenGLProgram() {};
		OpenGLProgram(const OpenGLProgram&) {};
		OpenGLProgram& operator=(const OpenGLProgram&) {return *this;};
		OpenGLProgram(const std::string& source, const ASTNode* uniforms, const ASTNode* outputs);
		friend class OpenGLCompiler;

		void get_output(output_t, int32_t, int32_t, int32_t, int32_t, void**);

		// glsl source code for a compiled program
		std::string _source;

		// render dimensions
		int32_t _size[2];

		uint32_t _vertex_array;
		uint32_t _vertex_buffer;
		uint32_t _frame_buffer;

		struct Uniform
		{
			std::string _name;
			ReturnType::Type _type;
			int32_t _param_location;

			// data we'll pass as uniforms on Run
			union
			{
				bool _bool;
				int32_t _int;
				uint32_t _uint;
				float _float;
				struct  
				{
					int32_t x;
					int32_t y;
					int32_t z;
					int32_t w;
				} _ivec;
				struct  
				{
					uint32_t x;
					uint32_t y;
					uint32_t z;
					uint32_t w;
				} _uvec;
				struct  
				{
					float x;
					float y;
					float z;
					float w;
				} _fvec;
				// all samplers
				struct 
				{
					int32_t texture_unit;
					int32_t handle;
				} _sampler;	
			};
		};
		int32_t _uniform_count;
		Uniform* _uniforms;

		struct Output
		{
			std::string _name;
			ReturnType::Type _type;
			uint32_t _texture_handle;
		};

		uint32_t* _render_buffers;

		int32_t _output_count;
		Output* _outputs;


		int32_t _fragment_shader;
		int32_t _program;
		int32_t _size_handle;
	};

	class OpenGLCompiler : public Compiler<OpenGLProgram>
	{
	public:
		virtual OpenGLProgram* Build(const Source&);
	private:
		uint32_t _indent;
		std::stringstream _ss;
		std::set<symbol_id_t> _declared_vars;
		void print_type(ReturnType::Type);
		void print_declaration(symbol_id_t, ReturnType::Type);
		void print_indent();
		void print_newline(const ASTNode*);
		void print_operator(const char*, const ASTNode*, const ASTNode*);
		void print_code(const ASTNode*);
		void print_function(const ASTNode*);
		void print_var(symbol_id_t);
		void print_glsl(const ASTNode*, const ASTNode*, const ASTNode*);
		static std::string get_var_name(symbol_id_t);
		friend class OpenGLProgram;
	};
}