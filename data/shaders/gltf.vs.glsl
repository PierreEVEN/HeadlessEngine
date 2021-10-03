#version 460

// IN
layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec4 col;
layout(location = 3) in vec3 norm;
layout(location = 4) in vec3 tang;
layout(location = 5) in vec3 bitang;

// OUT
layout (location = 9) out vec3 normal;
layout (location = 10) out vec2 uvs;

// UNIFORM BUFFER
layout(binding = 9) uniform GlobalCameraUniformBuffer {
    mat4 worldProjection;
    mat4 viewMatrix;
	vec3 cameraLocation;
} ubo;

struct ObjectData{
	mat4 model;
};

layout(std140, binding = 0) readonly buffer ObjectBuffer{
	ObjectData objects[];
} objectBuffer;

void main() {
	normal = norm;
	uvs = uv;
	gl_Position = ubo.worldProjection * ubo.viewMatrix * objectBuffer.objects[gl_InstanceIndex].model * vec4(pos.xyz, 1.0);
}