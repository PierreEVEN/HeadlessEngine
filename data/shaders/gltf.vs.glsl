#AUTO_LOCATION out vec3 normal;
#AUTO_LOCATION out vec2 uvs;
#AUTO_LOCATION out vec4 color;
#AUTO_LOCATION out vec3 pos;
#AUTO_LOCATION out vec3 tangent;

void main() {

	mat4 model = instance_transform[gl_InstanceIndex].model;
	
	vec4 tmpPos = vec4(vertex_position.xyz, 1.0);

	gl_Position = worldProjection * viewMatrix * model * tmpPos;

	uvs = vertex_uvs;
	
	// Vertex position in world space
	pos = vec3(model * tmpPos);
	
	// Normal in world space
	mat3 mNormal = transpose(inverse(mat3(model)));
	normal = mNormal * normalize(vertex_normal);	
	tangent = mNormal * normalize(vertex_tangent);
	
	// Currently just vertex color
	color = vertex_color;
}