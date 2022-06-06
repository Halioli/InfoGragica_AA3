#version 330 
layout(pixel_center_integer) in vec4 gl_FragCoord;
in vec4 fragmentNorm;
in vec3 fragmentPos;
in vec2 fragmentUV;
in vec3 objNorm;
out vec4 out_Color;
uniform vec3 lightPos;
uniform mat4 mv_Mat;
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
	out_Color = texture2D(diffuseTexture, fragmentUV);

	// Remove alpha color
	if (out_Color.a < 0.9)
	{
		discard;
	}
}