#AUTO_LOCATION out vec3 position;
#AUTO_LOCATION out vec3 normal;
#AUTO_LOCATION out vec2 uvs;

void main() {
	position = vec3(0);
	uvs = vec2(0);
	normal = vec3(0);
	gl_Position = worldProjection * viewMatrix * instance_transform[gl_InstanceIndex].model * vec4(position.xyz, 1.0);
}