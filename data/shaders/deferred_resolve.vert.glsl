#version 460

// OUT
layout (location = 10) out vec2 uvs;

void main() {
	uvs = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(uvs * 2.0f - 1.0f, 0.0f, 1.0f);
	uvs.y = 1 - uvs.y;
}