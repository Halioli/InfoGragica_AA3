#version 330 
layout (location = 0) in vec3 in_Vertices;
layout (location = 1) in vec3 in_Normals;
layout (location = 2) in vec2 in_UVs;
in vec4 cameraPoint;
out vec4 vert_Normal;
out vec3 FragPos;
out vec2 fragUVs;
out vec3 objNormal;
out vec3 TexCoords;
uniform mat4 objMat;
uniform mat4 mv_Mat;
uniform mat4 mvpMat;
uniform vec3 viewPos;
void main() {
	TexCoords = in_Vertices;
	gl_Position = objMat * vec4(in_Vertices, 1.0);
	vert_Normal = mv_Mat * objMat * vec4(in_Normals, 0.0);
	FragPos = vec3(objMat * vec4(in_Vertices, 1.0));

	fragUVs = in_UVs;
}