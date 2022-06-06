#version 330 
layout (location = 0) in vec3 in_Vertices;
layout (location = 1) in vec3 in_Normals;
layout (location = 2) in vec2 in_UVs;
out vec3 FragPos;
out vec2 fragUVs;
uniform mat4 objMat;
uniform mat4 mv_Mat;
uniform mat4 mvpMat;
void main() {
	gl_Position = objMat * vec4(in_Vertices, 1.0);
	FragPos = vec3(objMat * vec4(in_Vertices, 1.0));

	fragUVs = in_UVs;
}