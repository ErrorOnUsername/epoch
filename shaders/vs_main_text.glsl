#version 400 core

layout( location = 0 ) in vec3 in_position;
layout( location = 1 ) in vec2 in_uv;
layout( location = 2 ) in vec3 in_color;
layout( location = 3 ) in int  in_texture_slot;

out vec2 pass_uv;
out vec3 pass_color;
flat out int pass_texture_slot;

uniform mat4 u_proj_matrix;

void main()
{
	pass_uv           = in_uv;
	pass_color        = in_color;
	pass_texture_slot = in_texture_slot;

	gl_Position = u_proj_matrix * vec4( in_position, 1.0 );
}

