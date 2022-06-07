#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <GL\glew.h>
#include <cstring>
#include <sstream>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <vector>

//#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

class Shader
{
public:
	Shader(std::string vertexShaderPath, std::string fragmentShaderPath, std::string geometryShaderPath, char* texturePath, bool fliped);
	Shader(std::string vertexShaderPath, std::string fragmentShaderPath, std::string geometryShaderPath, char* texturePath, bool fliped, std::vector<std::string> faces);
	Shader(std::string vertexShaderPath, std::string fragmentShaderPath, char* texturePath, bool fliped);
	~Shader();

	std::string GetShaderFromPath(std::string fragmentPath);

	void CreateAllShaders();
	void UseProgram();
	void DeleteProgram();
	GLuint GetProgram();

	void GenerateTexture();

	void ActivateTexture();
	void ActivateCubemapTexture();
	void ActivateTexture(int newTex);

	void GenerateFramebufferTexture();
	GLuint LoadCubemap(std::vector<std::string> faces);

	GLuint GetUniformLocation(const char* uniformName);
	void SetUniformInt(char* uniformName, int value);
	void SetUniformFloat(char* uniformName, float value);
	void SetUniformVector3(char* uniformName, glm::vec3 value);
	void SetUniformVector4(char* uniformName, glm::vec3 value);
	void SetUniformMatrix4(const char* uniformName, glm::mat4 value);

	int GetTextureWidth();
	int GetTextureHeight();

private:
	std::string vertexShaderSource;
	std::string geometryShaderSource;
	std::string fragmentShaderSource;

	GLuint program;

	// TEXTURES
	unsigned char* textureData;
	GLuint textureID;
	int width = 600,
		height = 600,
		numberOfColorChannels = 4;

	// FRAMEBUFFER
	GLuint fbo;
	GLuint fbo_tex;
};