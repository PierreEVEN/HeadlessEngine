
#AUTO_LOCATION out vec4 outColor;
#AUTO_LOCATION out vec4 outNormal;
#AUTO_LOCATION out vec4 outPosition;

void main() {
	vec3 sun = normalize(vec3(-1.0, 1.0, 1.0));
	float ambiant = 0.2;
	float light_power = max(0, dot(sun, normal)) * (1 - ambiant) + ambiant;
	outColor = light_power * vec4(mod(position.x / 20, 1), mod(position.y / 20, 1), mod(position.z / 20, 1), 0);
	
	if (outColor.a < 0.5) discard;
	outPosition = vec4(position, 1.0);

	outNormal = vec4(normal, 1.0);
}