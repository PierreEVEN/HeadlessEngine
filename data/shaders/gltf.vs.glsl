#version 460

// IN
layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec4 col;
layout(location = 3) in vec3 norm;
layout(location = 4) in vec3 tang;
layout(location = 5) in vec3 bitang;

// OUT
layout (location = 6) out vec3 out_normal;
layout (location = 7) out vec2 out_uvs;
layout (location = 8) out vec4 out_color;
layout (location = 9) out vec3 out_pos;
layout (location = 10) out vec3 out_tangent;

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

	mat4 model = objectBuffer.objects[gl_InstanceIndex].model;
	
	vec4 tmpPos = vec4(pos.xyz, 1.0);

	gl_Position = ubo.worldProjection * ubo.viewMatrix * model * tmpPos;

	out_uvs = uv;
	
	// Vertex position in world space
	out_pos = vec3(model * tmpPos);
	
	// Normal in world space
	mat3 mNormal = transpose(inverse(mat3(model)));
	out_normal = mNormal * normalize(norm);	
	out_tangent = mNormal * normalize(tang);
	
	// Currently just vertex color
	out_color = col;
}