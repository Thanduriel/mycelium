#version 450

#define OUT_VERTS 4

layout(location=0) in vec3 in_start[1];
layout(location=1) in vec3 in_end[1];
layout(location=2) in vec4 in_color[1];
layout(location=3) in float in_size[1];

layout(location=0) out vec4 out_color;
layout(location=1) out vec2 out_texCoord;

layout(points) in;
layout(triangle_strip, max_vertices = OUT_VERTS) out;

layout(location = 0) uniform mat4 c_viewProjection;

void main(void)
{
	// Transform to projection space - the rest is done directly in screen space
	vec4 l1 = c_viewProjection * vec4(in_start[0], 1);
	vec4 l2 = c_viewProjection * vec4(in_end[0], 1);
	
	// Do a manual clipping.
	float threshold = -5.0; // TODO: use nearplane
	vec4 direction = normalize(l2 - l1);
	// Compute original distance for texture coordinate reprojection.
	float len = length(l2.xyz - l1.xyz) * 0.5;
	// Reproject end points to the near-thresholdplane
	if( l1.z < threshold )
		l1 -= direction * (l1.z-threshold) / direction.z;
	float len2 = length(l2.xyz - l1.xyz);
	float tex1 = 1.0 - len2 / len;
	if( l2.z < threshold )
		l2 -= direction * (l2.z-threshold) / direction.z;
	float tex2 = length(l2.xyz - l1.xyz) / len2 * 2.0 - 1.0;

	// Compute a vector perpendicular vector to create a beam
	vec2 dir = normalize(l2.xy / l2.w - l1.xy / l1.w);
	// Cross product with view direction
	vec4 perpendicular = vec4(-dir.y * in_size[0], dir.x * in_size[0], 0, 0);

	out_color = in_color[0];
	gl_Position = l1 + perpendicular;
	out_texCoord = vec2(tex1, -1.0);
	EmitVertex();
	
	gl_Position = l1 - perpendicular;
	out_texCoord = vec2(tex1, 1.0);
	EmitVertex();
	
	out_color = in_color[0];
	gl_Position = l2 + perpendicular;
	out_texCoord = vec2(tex2, -1.0);
	EmitVertex();
	
	gl_Position = l2 - perpendicular;
	out_texCoord = vec2(tex2, 1.0);
	EmitVertex();
	EndPrimitive();
}