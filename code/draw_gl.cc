#include "draw_gl.hh"
#include <cassert>
#include <cstdio>
#include <cstdlib>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include "window.hh"
#include "font.hh"


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
                                           "        out_color = vec4( pass_uv, 0.0, 1.0 ) * texture( u_textures[pass_texture_slot - 1], pass_uv ).r;\n"
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


static uint32_t create_shader_program( char const* vert_path, char const* frag_path )
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


static uint32_t s_main_shader_id     = 0;
static uint32_t s_main_vertex_buffer = 0;
static uint32_t s_main_index_buffer  = 0;
static uint32_t s_main_vertex_array  = 0;


static constexpr size_t MAX_QUAD_COUNT = 10000;
struct QuadVertex {
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 color;
	int       texture_slot;
};

static_assert( sizeof( QuadVertex ) == 9 * 4 );

static size_t     s_quad_push_idx = 0;
static QuadVertex s_batch_quad_pool[MAX_QUAD_COUNT * 4];
static uint32_t*  s_batch_quad_indices = nullptr;

static FontAtlas s_main_atlas;


static uint32_t create_vertex_array( uint32_t vertex_buffer_id, uint32_t index_buffer_id );


void renderer_init()
{
	s_batch_quad_indices = (uint32_t*)malloc( MAX_QUAD_COUNT * 4 * 6 * sizeof(uint32_t) );
	for ( uint32_t i = 0; i < MAX_QUAD_COUNT; i++ )
	{
		s_batch_quad_indices[( 6 * i ) + 0] = ( 4 * i ) + 0;
		s_batch_quad_indices[( 6 * i ) + 1] = ( 4 * i ) + 1;
		s_batch_quad_indices[( 6 * i ) + 2] = ( 4 * i ) + 2;
		s_batch_quad_indices[( 6 * i ) + 3] = ( 4 * i ) + 2;
		s_batch_quad_indices[( 6 * i ) + 4] = ( 4 * i ) + 3;
		s_batch_quad_indices[( 6 * i ) + 5] = ( 4 * i ) + 0;
	}

	s_main_shader_id = create_shader_program( "shaders/vs_main_text.glsl", "shaders/fs_main_text.glsl" );

	glGenBuffers( 1, &s_main_vertex_buffer );
	glGenBuffers( 1, &s_main_index_buffer );

	s_main_vertex_array  = create_vertex_array( s_main_vertex_buffer, s_main_index_buffer );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, s_main_index_buffer );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, MAX_QUAD_COUNT * 4 * 6 * sizeof(uint32_t), s_batch_quad_indices, GL_STATIC_DRAW );

	bool atlas_init_succeeded = init_font_atlas( &s_main_atlas, "fonts/jet_brains_mono/fonts/ttf/JetBrainsMono-Regular.ttf" );
	if ( !atlas_init_succeeded )
	{
		fprintf( stderr, "Could not initialize the font atlas!" );
		exit( 1 );
	}
}


void renderer_deinit()
{
	glDeleteProgram( s_main_shader_id );
	glDeleteBuffers( 1, &s_main_vertex_buffer );
	glDeleteBuffers( 1, &s_main_index_buffer );
	glDeleteVertexArrays( 1, &s_main_vertex_array );

	glDeleteTextures( 1, &s_main_atlas.texture );

	free( (void*)s_batch_quad_indices );
}


void renderer_clear( float r, float g, float b )
{
	glClearColor( r, g, b, 1.0f );

	glClear( GL_COLOR_BUFFER_BIT );
}


void immediate_begin( float width, float height )
{
	s_quad_push_idx = 0;

	float zoom = 1.0f;
	glm::mat4 proj_mat = glm::ortho( 0.0f, width * zoom, 0.0f, height * zoom, -5.0f, 5.0f );

	glUseProgram( s_main_shader_id );

	int location = glGetUniformLocation( s_main_shader_id, "u_proj_matrix" );
	if ( location != -1 )
	{
		glUniformMatrix4fv( location, 1, GL_FALSE, (float const*)glm::value_ptr( proj_mat ) );
	}
}


void immediate_flush()
{
	if ( s_quad_push_idx == 0 ) return;

	size_t verts_size = s_quad_push_idx * sizeof( QuadVertex );
	size_t idxs_size  = ( ( s_quad_push_idx / 4 ) * 6 ) * sizeof( uint32_t );

	glUseProgram( s_main_shader_id );

	glBindBuffer( GL_ARRAY_BUFFER, s_main_vertex_buffer );
	glBufferData( GL_ARRAY_BUFFER, verts_size, (float*)s_batch_quad_pool, GL_DYNAMIC_DRAW );

	glBindVertexArray( s_main_vertex_array );
	glBindBuffer( GL_ARRAY_BUFFER, s_main_vertex_buffer );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, s_main_index_buffer );

	glDrawElements( GL_TRIANGLES, (GLsizei)( ( s_quad_push_idx / 4 ) * 6 ), GL_UNSIGNED_INT, nullptr );
}


static void immediate_push_textured_rect( Vec3 pos, Vec2 size, Vec3 color, Vec2 uv_bottom_left, Vec2 uv_top_right, int texture_slot )
{
	s_batch_quad_pool[s_quad_push_idx + 0].position     = { pos.x, pos.y, pos.z };
	s_batch_quad_pool[s_quad_push_idx + 0].uv           = { uv_bottom_left.x, uv_bottom_left.y };
	s_batch_quad_pool[s_quad_push_idx + 0].color        = { color.r, color.g, color.b };
	s_batch_quad_pool[s_quad_push_idx + 0].texture_slot = texture_slot;

	s_batch_quad_pool[s_quad_push_idx + 1].position     = { pos.x + size.x, pos.y, pos.z };
	s_batch_quad_pool[s_quad_push_idx + 1].uv           = { uv_top_right.x, uv_bottom_left.y };
	s_batch_quad_pool[s_quad_push_idx + 1].color        = { color.r, color.g, color.b };
	s_batch_quad_pool[s_quad_push_idx + 1].texture_slot = texture_slot;

	s_batch_quad_pool[s_quad_push_idx + 2].position     = { pos.x + size.x, pos.y + size.y, pos.z };
	s_batch_quad_pool[s_quad_push_idx + 2].uv           = { uv_top_right.x, uv_top_right.y };
	s_batch_quad_pool[s_quad_push_idx + 2].color        = { color.r, color.g, color.b };
	s_batch_quad_pool[s_quad_push_idx + 2].texture_slot = texture_slot;

	s_batch_quad_pool[s_quad_push_idx + 3].position     = { pos.x, pos.y + size.y, pos.z };
	s_batch_quad_pool[s_quad_push_idx + 3].uv           = { uv_bottom_left.x, uv_top_right.y };
	s_batch_quad_pool[s_quad_push_idx + 3].color        = { color.r, color.g, color.b };
	s_batch_quad_pool[s_quad_push_idx + 3].texture_slot = texture_slot;

	s_quad_push_idx += 4;
}


void immediate_push_rect( Vec3 pos, Vec2 size, Vec3 color )
{
	immediate_push_textured_rect(
		pos,
		size,
		color,
		{ 0.0f, 0.0f },
		{ 1.0f, 1.0f },
		0 );
}


void immediate_push_text( Vec3 pos, Vec3 color, char const* text, size_t len )
{
	Vec3 writing_pos = pos;
	Vec2 shifted_pos;
	Vec2 bottom_left;
	Vec2 top_right;
	Vec2 size;

	for ( size_t i = 0; i < len; i++ )
	{
		char c = text[i];
		if ( c == '\n' )
		{
			writing_pos.x = pos.x;
			writing_pos.y -= 18.0f;
			continue;
		}
		else if ( c == '\t' )
		{
			writing_pos.x += 4.0f * s_main_atlas.metrics[' '].advance.x;
			continue;
		}

		if ( c < 32 || c > 126 )
		{
			c = '?'; // TODO: Support non-ASCII characters
		}

		GlyphMetric metric = s_main_atlas.metrics[c];
		size = metric.bitmap_size;
		size.y *= -1;

		shifted_pos.x = writing_pos.x + metric.bitmap_top_left.x;
		shifted_pos.y = -writing_pos.y - metric.bitmap_top_left.y;

		writing_pos.x += metric.advance.x;
		writing_pos.y += metric.advance.y;

		bottom_left = Vec2 { metric.tex_off_x, 0.0f };
		top_right.x = bottom_left.x + ( metric.bitmap_size.x / (float)s_main_atlas.width );
		top_right.y = bottom_left.y + ( metric.bitmap_size.y / (float)s_main_atlas.height );

		immediate_push_textured_rect(
			{ shifted_pos.x, -shifted_pos.y, 0.0f },
			size,
			color,
			bottom_left,
			top_right,
			1 );
	}
}


static uint32_t create_vertex_array( uint32_t vertex_buffer_id, uint32_t index_buffer_id )
{
	uint32_t varr_id;
	glGenVertexArrays( 1, &varr_id );

	glBindVertexArray( varr_id );
	glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer_id );

	size_t offset = 0;

	glEnableVertexAttribArray( 0 ); // POSITION (vec3)
	glVertexAttribPointer( 0, 3, GL_FLOAT, false, sizeof( QuadVertex ), (const void*)( offset ) );

	offset += 3 * 4; // vec3 is 3 4-byte floats

	glEnableVertexAttribArray( 1 ); // UV (vec2)
	glVertexAttribPointer( 1, 2, GL_FLOAT, false, sizeof( QuadVertex ), (const void*)( offset ) );

	offset += 2 * 4; // vec2 is 2 4-byte floats

	glEnableVertexAttribArray( 2 ); // COLOR (vec3)
	glVertexAttribPointer( 2, 3, GL_FLOAT, false, sizeof( QuadVertex ), (const void*)( offset ) );

	offset += 3 * 4; // vec3 is 3 4-byte floats

	glEnableVertexAttribArray( 3 ); // TEXTURE_SLOT(int)
	glVertexAttribIPointer( 3, 1, GL_INT, sizeof( QuadVertex ), (const void*)( offset ) );

	offset += 4; // int is a 32-bit integer

	assert( offset == sizeof( QuadVertex ) );

	return varr_id;
}


static uint32_t load_font_texture( char const* path )
{
	uint32_t tex_id = 0;

	return tex_id;
}

