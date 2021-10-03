#version 460
#extension GL_ARB_separate_shader_objects : enable

// IN
layout (location = 6) in vec3 normal;
layout (location = 7) in vec2 uvs;
layout (location = 8) in vec4 in_color;
layout (location = 9) in vec3 position;
layout (location = 10) in vec3 tangent;

// OUT
layout(location = 0) out vec4 outColor;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outPosition;

// TEXTURES
layout (binding = 6) uniform sampler2D diffuse_color;

void main() {
	outColor = texture(diffuse_color, uvs);
	if (outColor.a < 0.5) discard;
	outPosition = vec4(position, 1.0);

	// Calculate normal in tangent space
	/*
	vec3 N = normalize(normal);
	vec3 T = normalize(tangent);
	vec3 B = cross(N, T);
	mat3 TBN = mat3(T, B, N);
	vec3 tnorm = TBN * normalize(texture(normal_sampler, uvs).xyz * 2.0 - vec3(1.0));
	*/
	outNormal = vec4(normal, 1.0);

}