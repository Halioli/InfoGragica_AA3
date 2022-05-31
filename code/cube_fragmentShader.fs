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
	// Checkerboard pattern
	/*float X = floor((gl_FragCoord.x) / 8.0);
	float Y = floor((gl_FragCoord.y) / 8.0);

	if (mod(X + Y, 2) == 0)
	{
		discard;
	}*/
	//

	out_Color = texture(diffuseTexture, fragmentUV);// * vec4(color.xyz * dot(fragmentNorm, mv_Mat * vec4(0.0, 1.0, 0.0, 0.0)) + color.xyz * 0.3, 1.0 );

	// Remove alpha color
	if (out_Color.a < 0.9)
	{
		discard;
	}
}