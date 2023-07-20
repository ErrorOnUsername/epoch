#include "draw_gl.hh"
#include <cstdio>
#include <cstdlib>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "util.hh"


static char const* s_default_vert_shader = "#version 400 core\n"
                                           "layout( location = 0 ) in vec3 in_position;\n"
                                           "layout( location = 1 ) in vec2 in_uv;\n"
                                           "layout( location = 2 ) in vec3 in_color;\n"
                                           "layout( location = 3 ) in int  in_texture_slot;\n"
                                           "out vec2 pass_uv;\n"
                                           "out vec3 pass_color;\n"
                                           "flat out int pass_texture_slot;\n"
                                           "uniform mat4 u_proj_matrix;\n"
                                           "void main()\n"
                                           "{\n"
                                           "    pass_uv           = in_uv;\n"
                                           "    pass_color        = in_color;\n"
                                           "    pass_texture_slot = in_texture_slot;\n"
                                           "    gl_Position = u_proj_matrix * vec4( in_position, 1.0 );\n"
                                           "}\n";

static char const* s_default_frag_shader = "#version 400 core\n"
                                           "#define MAX_TEXTURE_SLOTS 16\n"
                                           "in vec2 pass_uv;\n"
                                           "in vec3 pass_color;\n"
                                           "flat in int pass_texture_slot;\n"
                                           "out vec4 out_color;\n"
                                           "uniform sampler2D u_textures[MAX_TEXTURE_SLOTS];\n"
                                           "void main()\n"
                                           "{\n"
                                           "    if ( pass_texture_slot == 0 )\n"
                                           "    {\n"
                                           "        out_color = vec4( pass_uv, 0.0, 1.0 );\n"
                                           "    }\n"
                                           "    else\n"
                                           "    {\n"
                                           "        out_color = vec4( pass_uv, 0.0, 1.0 ) * texture( u_textures[pass_texture_slot - 1], pass_uv );\n"
                                           "    }\n"
                                           "}\n";


static int compile_shader( GLenum shader_type, char const* path )
{
	bool using_fallback_shaders = false;
	char const* shader_source = read_entire_file( path );
	if ( !shader_source )
	{
		// Assign the default shader in the event of a failure to read the file.
		using_fallback_shaders = true;

		if ( shader_type == GL_VERTEX_SHADER )
		{
			shader_source = s_default_vert_shader;
		}
		else if ( shader_type == GL_FRAGMENT_SHADER )
		{
			shader_source = s_default_frag_shader;
		}
	}

	uint32_t shader_id = glCreateShader( shader_type );

	glShaderSource( shader_id, 1, &shader_source, nullptr );
	glCompileShader( shader_id );

	int shader_compiled = 0;
	glGetShaderiv( shader_id, GL_COMPILE_STATUS, &shader_compiled );

	if ( shader_compiled != GL_TRUE )
	{
		GLsizei log_size = 0;
		GLchar msg[1024];

		glGetShaderInfoLog( shader_id, 1024, &log_size, msg );
		fprintf( stderr, "shader compilation error:\n%s", (const char*)msg );
	}

	if ( !using_fallback_shaders ) free( (void*)shader_source );

	return shader_id;
}


static int create_shader_program( char const* vert_path, char const* frag_path )
{
	int vert_id = compile_shader( GL_VERTEX_SHADER, vert_path );
	int frag_id = compile_shader( GL_FRAGMENT_SHADER, frag_path );

	uint32_t program_id = glCreateProgram();

	glAttachShader( program_id, vert_id );
	glAttachShader( program_id, frag_id );
	glLinkProgram( program_id );
	glValidateProgram( program_id );

	glDetachShader( program_id, vert_id );
	glDeleteShader( vert_id );
	glDetachShader( program_id, frag_id );
	glDeleteShader( frag_id );

	return program_id;
}


static int s_main_shader_id = -1;


static constexpr size_t MAX_QUAD_COUNT = 10000;
struct QuadVertex {
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 color;
	int       texture_slot;
};

static QuadVertex s_batch_quad_pool[MAX_QUAD_COUNT * 4];


void renderer_init()
{
	s_main_shader_id = create_shader_program( "shaders/vs_main_text.glsl", "shaders/fs_main_text.glsl" );
}


void renderer_deinit()
{
	glDeleteProgram( s_main_shader_id );
}


void renderer_clear( float r, float g, float b )
{
	glClearColor( r, g, b, 1.0f );

	glClear( GL_COLOR_BUFFER_BIT );
}


void immediate_begin()
{
}


void immediate_flush()
{
}

