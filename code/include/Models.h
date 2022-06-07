#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <vector>

#include "LoadOBJ.h"
#include "Shaders.h"

class Model
{
public:
	Model(char* objPath);
	~Model();

	void CreateVertexArrayObject();
	void Cleanup();
	void BindVertex();

	void SetObjMat(glm::mat4 newObjMat);
	void SetScale(glm::vec3 newScale);
	void SetLocation(glm::vec3 newLocation);
	void SetRotation(glm::vec3 newRotation);
	void SetRoatationAngle(float newAngle);
	
	void CalculateObjMat();

	void SetUniforms(Shader shader, glm::mat4 modelView, glm::mat4 MVP, glm::vec4 fragColor);
	void SetUniforms(Shader shader, glm::mat4 modelView, glm::mat4 MVP, glm::vec4 cameraPoint, glm::vec4 fragColor);
	void SetUniforms(Shader shader, glm::mat4 modelView, glm::mat4 MVP, float &time, glm::vec4 fragColor);
	void SetUniforms(Shader shader, glm::mat4 modelView, glm::mat4 MVP, glm::vec4 fragColor, float alphaVal);
	
	void DrawArraysTriangles();
	void DrawArraysPoints();
	void DrawArraysTrianglesInstanced(std::vector<glm::mat4> objMats, Shader shader);

	glm::vec3 GetLocation();
	glm::vec3 GetRotation();
	float GetRotationAngle();
	glm::mat4 GetModelMatrix();
	glm::vec3 GetObjNormal(int id);

private:
	GLuint VAO;
	GLuint VBO[3];

	std::vector<glm::vec3> objVertices;
	std::vector<glm::vec2> objUVs;
	std::vector<glm::vec3> objNormals;

	float rotationAngle;
	glm::mat4 objMat;
	glm::vec3 rotation;
	glm::vec3 scale;
	glm::vec3 location;
};