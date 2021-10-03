#version 460
#extension GL_ARB_separate_shader_objects : enable

// IN
layout (location = 9) in vec3 normal;
layout (location = 10) in vec2 uvs;

// OUT
layout(location = 0) out vec4 outColor;

// TEXTURES
layout (binding = 6) uniform sampler2D diffuse_color;

void main() {
	vec3 sun = normalize(vec3(-1.0, 1.0, 1.0));

	float ambiant = 0.4;

	float light_power = max(0, dot(sun, normal)) * (1 - ambiant) + ambiant;

	vec4 finalcol = texture(diffuse_color, uvs);
	if (finalcol.a < 0.05) discard;
	outColor = light_power * finalcol;
}