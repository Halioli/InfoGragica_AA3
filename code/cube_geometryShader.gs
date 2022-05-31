#version 330 
//layout (points) in; // This for billboards
layout (triangles) in; // This for cube
layout (triangle_strip, max_vertices = 6) out;
in vec4 vert_Normal[];
in vec3 FragPos[];
in vec2 fragUVs[];
in vec3 objNormal[];
out vec4 fragmentNorm;
out vec3 fragmentPos;
out vec2 fragmentUV;
out vec3 objNorm;
uniform mat4 mvpMat;
void main() {
	gl_Position = mvpMat * gl_in[0].gl_Position + vec4(0, 5, 0, 1);
	fragmentNorm = vert_Normal[0];
	fragmentPos = FragPos[0];
	fragmentUV = fragUVs[0];
	EmitVertex();
	
	gl_Position = mvpMat * gl_in[1].gl_Position + vec4(0, 5, 0, 1);
	fragmentNorm = vert_Normal[1];
	fragmentPos = FragPos[1];
	fragmentUV = fragUVs[1];
	EmitVertex();
	
	gl_Position = mvpMat * gl_in[2].gl_Position + vec4(0, 5, 0, 1);
	fragmentNorm = vert_Normal[2];
	fragmentPos = FragPos[2];
	fragmentUV = fragUVs[2];
	EmitVertex();

	EndPrimitive();

	objNorm = objNormal[0];
}