#AUTO_LOCATION out vec4 outColor;

void main() {	
  outColor = texture(colorSampler, uv);
}