#version 460

// IN
layout(location = 10) in vec2 uv;

layout (binding = 1) uniform sampler2D samplerAlbedo;
layout (binding = 2) uniform sampler2D samplerNormal;
layout (binding = 3) uniform sampler2D samplerPosition;

// UNIFORM BUFFER
layout(binding = 9) uniform GlobalCameraUniformBuffer {
    mat4 worldProjection;
    mat4 viewMatrix;
	vec3 cameraLocation;
} ubo;

// OUT
layout(location = 0) out vec4 outColor;

void demo_deferred() {
	if (uv.x +(uv.y * 0.2 - 0.1) < 1.0/3)
		outColor = texture(samplerPosition, uv) / 4;	
	else if (uv.x +(uv.y * 0.2 - 0.1) < 2.0/3)
		outColor = texture(samplerAlbedo, uv);	
	else 
		outColor = texture(samplerNormal, uv);	
}

void deferred_render() {

	// Get G-Buffer values
	vec3 fragPos = texture(samplerPosition, uv).rgb;
	vec3 normal = texture(samplerNormal, uv).rgb;
	vec4 albedo = texture(samplerAlbedo, uv);
	
	// Render-target composition

	// Ambient part
	vec3 fragcolor  = albedo.rgb * 0.5;
		
	// Viewer to fragment
	vec3 V = ubo.cameraLocation.xyz - fragPos;
	V = normalize(V);
	
	// Light to fragment
	vec3 L = normalize(vec3(1, 1, 1));

	// Diffuse part
	vec3 N = normalize(normal);
	float NdotL = max(0.0, dot(N, L));
	vec3 diff = albedo.rgb * NdotL;

	// Specular part
	// Specular map values are stored in alpha of albedo mrt
	vec3 R = reflect(-L, N);
	float NdotR = max(0.0, dot(R, V));
	vec3 spec = vec3(albedo.a * pow(NdotR, 16.0));

	fragcolor += diff + spec;	
	
	outColor = vec4(fragcolor, 1.0);	
}


void main() {
	
	deferred_render();
}