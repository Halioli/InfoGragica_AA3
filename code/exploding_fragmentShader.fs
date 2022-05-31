#version 330 
layout(pixel_center_integer) in vec4 gl_FragCoord;
in vec4 fragmentNorm;
//in vec4 fragmentPos;
in vec2 fragmentUV;
out vec4 out_Color;
uniform vec3 lightPos;
//uniform mat4 mv_Mat;
uniform vec4 color;
uniform sampler2D diffuseTexture;

struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};
uniform Material material;

void main() {
	//out_Color = fragmentNorm;
	//out_Color = vec4(fragmentUV, 0.0, 1.0);
	out_Color = texture(diffuseTexture, fragmentUV);// * vec4(color.xyz * dot(fragmentNorm, mv_Mat * vec4(0.0, 1.0, 0.0, 0.0)) + color.xyz * 0.3, 1.0 );
}