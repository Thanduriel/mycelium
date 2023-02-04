#version 450

layout(location=0) in vec3 in_start;
layout(location=1) in vec3 in_end;
layout(location=2) in vec4 in_color;
layout(location=3) in float in_size;

layout(location=0) out vec3 out_start;
layout(location=1) out vec3 out_end;
layout(location=2) out vec4 out_color;
layout(location=3) out float out_size;

void main()
{
	out_start = in_start;
	out_end = in_end;
	out_color = in_color;
	out_size = in_size;
}
