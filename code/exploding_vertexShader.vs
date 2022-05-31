#version 330 
layout (location = 0) in vec3 in_Vertices;
layout (location = 1) in vec3 in_Normals;
layout (location = 2) in vec2 in_UVs;
out vec3 vert_Normal;
out vec3 FragPos;
out vec2 fragUVs;

void main() {
	gl_Position = vec4(in_Vertices, 1.0);
	vert_Normal = normalize(in_Normals);
	FragPos = in_Vertices;

	fragUVs = in_UVs;
}