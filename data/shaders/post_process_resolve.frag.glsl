#version 460

// IN
layout(location = 10) in vec2 uv;

layout (binding = 1) uniform sampler2D colorSampler;

// OUT
layout(location = 0) out vec4 outColor;

void main() {
	
  outColor = texture(colorSampler, uv);	


}