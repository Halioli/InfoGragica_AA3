#version 330 
layout(pixel_center_integer) in vec4 gl_FragCoord;
in vec3 texCoords;
out vec4 out_Color;
uniform samplerCube diffuseTexture;

void main() {
	out_Color = texture(diffuseTexture, texCoords);
}