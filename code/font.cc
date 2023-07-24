#include "font.hh"

#include <freetype/freetype.h>
#include <glad/glad.h>

#include "window.hh"


#define DEFAULT_FONT_PX_SIZE 18


bool init_font_atlas( FontAtlas* atlas, char const* path )
{
	FT_Library ft;

	int err = FT_Init_FreeType( &ft );
	if ( err )
	{
		return false;
	}

	FT_Face ft_face;
	err = FT_New_Face( ft, path, 0, &ft_face );
	if ( err )
	{
		return false;
	}

	err = FT_Set_Pixel_Sizes( ft_face, 0, DEFAULT_FONT_PX_SIZE );
	if ( err )
	{
		return false;
	}

	int load_flags = (FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT);
	for ( int i = 32; i < 128; i++ )
	{
		err = FT_Load_Char( ft_face, i, load_flags );
		if ( err )
		{
			return false;
		}

		atlas->width += ft_face->glyph->bitmap.width;
		if ( atlas->height < ft_face->glyph->bitmap.rows)
		{
			atlas->height = ft_face->glyph->bitmap.rows;
		}
	}

	glGenTextures( 1, &atlas->texture );

	glBindTexture( GL_TEXTURE_2D, atlas->texture );
	glActiveTexture( GL_TEXTURE0 );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RED,
		atlas->width,
		atlas->height,
		0,
		GL_RED,
		GL_UNSIGNED_BYTE,
		nullptr
	);

	int x = 0;

	for ( int i = 32; i < 128; i++ )
	{
		err = FT_Load_Char( ft_face, i, load_flags );
		if ( err )
		{
			return false;
		}

		err = FT_Render_Glyph( ft_face->glyph, FT_RENDER_MODE_NORMAL );
		if ( err )
		{
			return false;
		}

		atlas->metrics[i].advance.x         = (float)( ft_face->glyph->advance.x >> 6 );
		atlas->metrics[i].advance.y         = (float)( ft_face->glyph->advance.y >> 6 );
		atlas->metrics[i].bitmap_size.x     = (float)ft_face->glyph->bitmap.width;
		atlas->metrics[i].bitmap_size.y     = (float)ft_face->glyph->bitmap.rows;
		atlas->metrics[i].bitmap_top_left.x = (float)ft_face->glyph->bitmap_left;
		atlas->metrics[i].bitmap_top_left.y = (float)ft_face->glyph->bitmap_top;
		atlas->metrics[i].tex_off_x         = (float)x / atlas->width;

		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
		glTexSubImage2D(
			GL_TEXTURE_2D,
			0,
			x,
			0,
			ft_face->glyph->bitmap.width,
			ft_face->glyph->bitmap.rows,
			GL_RED,
			GL_UNSIGNED_BYTE,
			ft_face->glyph->bitmap.buffer );

		x += ft_face->glyph->bitmap.width;
	}

	FT_Done_FreeType( ft );

	return true;
}
