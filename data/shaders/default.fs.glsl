#version 460
#extension GL_ARB_separate_shader_objects : enable

// IN
layout (location = 8) in vec3 position;
layout (location = 9) in vec3 normal;
layout (location = 10) in vec2 uvs;
layout (location = 11) in vec4 color;

// OUT
layout(location = 0) out vec4 outColor;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outPosition;

// PARAMS
const vec4 big_color = vec4(1, 0.8, 0.2, 1);
const vec4 small_color = vec4(1, 1, 1, 1);
const vec4 normal_color = vec4(0.8, 0.8, 0.8, 1);
const vec4 no_uv_color = vec4(1, 0, 1, 1);

float grid(float position, float max_pos, float count, float width) {
	float percent = mod((position / max_pos) * count, 1.0);	
	return (width - percent) > 0 || (width + percent) > 1 ? 1 : 0;
}

vec4 calc_color() {

	if (uvs.x == 0 && uvs.y == 0) return no_uv_color;

	float small = min(1, grid(uvs.x, 1.0, 10, 0.02) + grid(uvs.y, 1.0, 10, 0.02));
	float big = min(1, grid(uvs.x, 1.0, 1, 0.008) + grid(uvs.y, 1.0, 1, 0.008));
	
	return (big > 0) ? big_color : small > 0 ? small_color : normal_color * color;
}

void main() {
	vec3 sun = normalize(vec3(-1.0, 1.0, 1.0));
	float ambiant = 0.2;
	float light_power = max(0, dot(sun, normal)) * (1 - ambiant) + ambiant;
	outColor = light_power * vec4(mod(position.x / 20, 1), mod(position.y / 20, 1), mod(position.z / 20, 1), 0);
}