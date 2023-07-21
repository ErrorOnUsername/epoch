#version 400 core

#define MAX_TEXTURE_SLOTS 16

in vec2 pass_uv;
in vec3 pass_color;
flat in int pass_texture_slot;

out vec4 out_color;

uniform sampler2D u_textures[MAX_TEXTURE_SLOTS];

void main()
{
	if ( pass_texture_slot == 0 )
	{
		out_color = vec4( pass_color, 1.0 );
	}
	else
	{
		out_color = vec4( pass_color, 1.0 ) * texture( u_textures[pass_texture_slot - 1], pass_uv );
	}
}

