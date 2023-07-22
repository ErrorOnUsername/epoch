#include "draw_gl.hh"
#include <cassert>
#include <cstdio>
#include <cstdlib>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>
#include <stb_truetype.h>

#include "window.hh"


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

static uint32_t s_code_font_textures[FontStyle_Count];


#define FONT_ATLAS_SIZE 512
static uint8_t s_fallback_image[4] = { 0x7f, 0xff, 0xff, 0x7f };
static stbtt_bakedchar s_stbtt_char_data[96]; // ASCII [32..127) ' ' - '~'


static uint32_t create_vertex_array( uint32_t vertex_buffer_id, uint32_t index_buffer_id );
static uint32_t load_font_texture( char const* path );
static void font_convert_glyph_quad( char msg, Vec3* pos, Vec2* size );


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

	s_code_font_textures[FontStyle_Regular] = load_font_texture( "fonts/jet_brains_mono/fonts/ttf/JetBrainsMono-Regular.ttf" );
}


void renderer_deinit()
{
	glDeleteProgram( s_main_shader_id );
	glDeleteBuffers( 1, &s_main_vertex_buffer );
	glDeleteBuffers( 1, &s_main_index_buffer );
	glDeleteVertexArrays( 1, &s_main_vertex_array );

	for ( uint32_t i = 0; i < FontStyle_Count; i++ )
	{
		glDeleteTextures( 1, &s_code_font_textures[i] );
	}

	free( (void*)s_batch_quad_indices );
}


void renderer_clear( float r, float g, float b )
{
	glClearColor( r, g, b, 1.0f );

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
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


static inline void push_textured_quad( Vec3 pos, Vec2 size, Vec3 color, uint32_t tex_id, int slot )
{
	Vec2 scale = window_get_scale();

	if ( slot > 0 )
	{
		glBindTexture( GL_TEXTURE_2D, tex_id );
		glActiveTexture( GL_TEXTURE0 + ( (uint32_t)slot - 1 ) );
	}

	pos.x *= scale.x;
	pos.y *= scale.y;
	size.x *= scale.x;
	size.y *= scale.y;

	s_batch_quad_pool[s_quad_push_idx + 0].position     = { pos.x, pos.y, pos.z };
	s_batch_quad_pool[s_quad_push_idx + 0].uv           = { 0.0f, 0.0f };
	s_batch_quad_pool[s_quad_push_idx + 0].color        = { color.r, color.g, color.b };
	s_batch_quad_pool[s_quad_push_idx + 0].texture_slot = slot;

	s_batch_quad_pool[s_quad_push_idx + 1].position     = { pos.x + size.x, pos.y, pos.z };
	s_batch_quad_pool[s_quad_push_idx + 1].uv           = { 1.0f, 0.0f };
	s_batch_quad_pool[s_quad_push_idx + 1].color        = { color.r, color.g, color.b };
	s_batch_quad_pool[s_quad_push_idx + 1].texture_slot = slot;

	s_batch_quad_pool[s_quad_push_idx + 2].position     = { pos.x + size.x, pos.y + size.y, pos.z };
	s_batch_quad_pool[s_quad_push_idx + 2].uv           = { 1.0f, 1.0f };
	s_batch_quad_pool[s_quad_push_idx + 2].color        = { color.r, color.g, color.b };
	s_batch_quad_pool[s_quad_push_idx + 2].texture_slot = slot;

	s_batch_quad_pool[s_quad_push_idx + 3].position     = { pos.x, pos.y + size.y, pos.z };
	s_batch_quad_pool[s_quad_push_idx + 3].uv           = { 0.0f, 1.0f };
	s_batch_quad_pool[s_quad_push_idx + 3].color        = { color.r, color.g, color.b };
	s_batch_quad_pool[s_quad_push_idx + 3].texture_slot = slot;

	s_quad_push_idx += 4;
}


void immediate_push_rect( Vec3 pos, Vec2 size, Vec3 color )
{
	push_textured_quad( pos, size, color, 0, 0 );
}


void immediate_push_text( char const* msg, Vec3 pos, Vec2 size, Vec3 color )
{
	float advance = 15.0f;

	glBindTexture( GL_TEXTURE_2D, s_code_font_textures[FontStyle_Regular] );
	glActiveTexture( GL_TEXTURE0 );

	while ( *msg )
	{
		float x = 0.0f;
		float y = 0.0f;

		stbtt_aligned_quad q;
		stbtt_GetBakedQuad( s_stbtt_char_data, FONT_ATLAS_SIZE, FONT_ATLAS_SIZE, *msg - 32, &x, &y, &q, 1 );

		s_batch_quad_pool[s_quad_push_idx + 0].position     = { pos.x, pos.y, pos.z };
		s_batch_quad_pool[s_quad_push_idx + 0].uv           = { q.s0, q.t1 };
		s_batch_quad_pool[s_quad_push_idx + 0].color        = { color.r, color.g, color.b };
		s_batch_quad_pool[s_quad_push_idx + 0].texture_slot = 1;

		s_batch_quad_pool[s_quad_push_idx + 1].position     = { pos.x + size.x, pos.y, pos.z };
		s_batch_quad_pool[s_quad_push_idx + 1].uv           = { q.s1, q.t1 };
		s_batch_quad_pool[s_quad_push_idx + 1].color        = { color.r, color.g, color.b };
		s_batch_quad_pool[s_quad_push_idx + 1].texture_slot = 1;

		s_batch_quad_pool[s_quad_push_idx + 2].position     = { pos.x + size.x, pos.y + size.y, pos.z };
		s_batch_quad_pool[s_quad_push_idx + 2].uv           = { q.s1, q.t0 };
		s_batch_quad_pool[s_quad_push_idx + 2].color        = { color.r, color.g, color.b };
		s_batch_quad_pool[s_quad_push_idx + 2].texture_slot = 1;

		s_batch_quad_pool[s_quad_push_idx + 3].position     = { pos.x, pos.y + size.y, pos.z };
		s_batch_quad_pool[s_quad_push_idx + 3].uv           = { q.s0, q.t0 };
		s_batch_quad_pool[s_quad_push_idx + 3].color        = { color.r, color.g, color.b };
		s_batch_quad_pool[s_quad_push_idx + 3].texture_slot = 1;

		pos.x += advance;
		s_quad_push_idx += 4;

		msg++;
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


static uint8_t* get_raw_font_bitmap( char const* path, bool* out_success )
{
	uint8_t* font_ttf_data = (uint8_t*)read_entire_file( path );
	if ( !font_ttf_data )
	{
		*out_success = false;
		return s_fallback_image;
	}

	uint8_t* font_bitmap_data = (uint8_t*)malloc( FONT_ATLAS_SIZE * FONT_ATLAS_SIZE );

	stbtt_BakeFontBitmap(
		font_ttf_data,
		0,
		32.0f,
		font_bitmap_data,
		FONT_ATLAS_SIZE,
		FONT_ATLAS_SIZE,
		32,
		96,
		s_stbtt_char_data );

	free( (void*)font_ttf_data );

	*out_success = true;
	return font_bitmap_data;
}


static uint32_t load_font_texture( char const* path )
{
	uint32_t tex_id = 0;

	bool success;
	uint8_t* font_bitmap_data = get_raw_font_bitmap( path, &success );
	uint32_t size = success ? FONT_ATLAS_SIZE : 2;

	glGenTextures( 1, &tex_id );
	glBindTexture( GL_TEXTURE_2D, tex_id );
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_ALPHA,
		size,
		size,
		0,
		GL_ALPHA,
		GL_UNSIGNED_BYTE,
		font_bitmap_data );

	if ( success ) free( (void*)font_bitmap_data );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	return tex_id;
}

