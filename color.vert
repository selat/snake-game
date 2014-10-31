#version 130

uniform mat4 modelview_matrix;
uniform mat4 projection_matrix;
in vec2 coord;
in vec4 color_f;
out vec4 color;

void main(void)
{
	color = color_f;
	vec4 pos = modelview_matrix * vec4(coord, 0.0f, 1.0f);
	gl_Position = projection_matrix * pos;
}
