#version 450

layout(location=0) in vec4 in_color;
layout(location=1) in vec2 in_texCoord;

layout(location = 0) out vec4 out_color;

void main()
{
	float fadeShort = max(0, 1 - in_texCoord.y * in_texCoord.y);
	out_color =  in_color * fadeShort;
//	out_color = vec4(1.0,1.0,1.0,1.0);
}