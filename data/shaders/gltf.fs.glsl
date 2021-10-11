#AUTO_LOCATION out vec4 outColor;
#AUTO_LOCATION out vec4 outNormal;
#AUTO_LOCATION out vec4 outPosition;

void main() {
	outColor = texture(diffuse_color, uvs);
	if (outColor.a < 0.5) discard;
	outPosition = pos;

	outNormal = vec4(normal, 1.0);

}