#AUTO_LOCATION out vec3 vertex_normal;
#AUTO_LOCATION out vec2 vertex_uvs;
#AUTO_LOCATION out vec4 vertex_color;
#AUTO_LOCATION out vec3 vertex_pos;
#AUTO_LOCATION out vec3 vertex_tangent;

const vec3 pos = vec3(0);
const vec2 uv = vec2(0);
const vec3 norm = vec3(0);
const vec3 tang = vec3(0);
const vec4 col = vec4(0);


void main() {

	mat4 model = instance_transform[gl_InstanceIndex].model;
	
	vec4 tmpPos = vec4(pos.xyz, 1.0);

	gl_Position = worldProjection * viewMatrix * model * tmpPos;

	vertex_uvs = uv;
	
	// Vertex position in world space
	vertex_pos = vec3(model * tmpPos);
	
	// Normal in world space
	mat3 mNormal = transpose(inverse(mat3(model)));
	vertex_normal = mNormal * normalize(norm);	
	vertex_tangent = mNormal * normalize(tang);
	
	// Currently just vertex color
	vertex_color = col;
}