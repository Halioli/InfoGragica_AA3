#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <cstdio>
#include <cassert>
#include <vector>

#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>

#include "GL_framework.h"
#include "SDL_timer.h"
#include "LoadOBJ.h"
#include "Shaders.h"
#include "Models.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

GLuint compileShader(const char* shaderStr, GLenum shaderType, const char* name = "");
void linkProgram(GLuint program);

///////// fw decl
namespace ImGui 
{
	void Render();
}

namespace Axis 
{
	void setupAxis();
	void cleanupAxis();
	void drawAxis();
}

namespace RenderVars
{
	const float FOV = glm::radians(65.f);
	const float zNear = 1.f;
	const float zFar = 100.f;

	glm::mat4 _projection;
	glm::mat4 _modelView;
	glm::mat4 _MVP;
	glm::mat4 _inv_modelview;
	glm::vec4 _cameraPoint;

	struct prevMouse
	{
		float lastx, lasty;
		MouseEvent::Button button = MouseEvent::Button::None;
		bool waspressed = false;
	} prevMouse;

	float panv[3] = { 0.f, -5.f, -15.f };
	float rota[2] = { 0.f, 0.f };
}
namespace RV = RenderVars;

void GLResize(int width, int height) 
{
	glViewport(0, 0, width, height);
	if (height != 0) RV::_projection = glm::perspective(RV::FOV, (float)width / (float)height, RV::zNear, RV::zFar);
	else RV::_projection = glm::perspective(RV::FOV, 0.f, RV::zNear, RV::zFar);
}

void GLmousecb(MouseEvent ev) 
{
	if (RV::prevMouse.waspressed && RV::prevMouse.button == ev.button) 
	{
		float diffx = ev.posx - RV::prevMouse.lastx;
		float diffy = ev.posy - RV::prevMouse.lasty;
		switch (ev.button) 
		{
		case MouseEvent::Button::Left: // ROTATE
			RV::rota[0] += diffx * 0.005f;
			RV::rota[1] += diffy * 0.005f;
			break;
		case MouseEvent::Button::Right: // MOVE XY
			RV::panv[0] += diffx * 0.03f;
			RV::panv[1] -= diffy * 0.03f;
			break;
		case MouseEvent::Button::Middle: // MOVE Z
			RV::panv[2] += diffy * 0.05f;
			break;
		default: break;
		}
	}
	else 
	{
		RV::prevMouse.button = ev.button;
		RV::prevMouse.waspressed = true;
	}
	RV::prevMouse.lastx = ev.posx;
	RV::prevMouse.lasty = ev.posy;
}

//////////////////////////////////////////////////
GLuint compileShader(const char* shaderStr, GLenum shaderType, const char* name)
{
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderStr, NULL);
	glCompileShader(shader);
	GLint res;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &res);
	if (res == GL_FALSE) 
	{
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &res);
		char* buff = new char[res];
		glGetShaderInfoLog(shader, res, &res, buff);
		fprintf(stderr, "Error Shader %s: %s", name, buff);
		delete[] buff;
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

void linkProgram(GLuint program) 
{
	glLinkProgram(program);
	GLint res;
	glGetProgramiv(program, GL_LINK_STATUS, &res);
	if (res == GL_FALSE) 
	{
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &res);
		char* buff = new char[res];
		glGetProgramInfoLog(program, res, &res, buff);
		fprintf(stderr, "Error Link: %s", buff);
		delete[] buff;
	}
}

namespace Framebuffer
{
	// == FRAMEBUFFER ==
	GLuint fbo;
	GLuint fbo_tex;
	
	GLuint rbo;

	// To draw a scene to a texture, we need a frame buffer:
	void SetupFBO()
	{
		// Setup FBO texture
		glGenFramebuffers(1, &fbo);

		// Create texture exactly as before:
		glGenTextures(1, &fbo_tex);
		glBindTexture(GL_TEXTURE_2D, fbo_tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 800, 800, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		
		// If we need a depth or stencil buffer, we do it here
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 800);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

		// We bind texture (or renderbuffer) to framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_tex, 0);
	}
	// == FRAMEBUFFER ==
}

////////////////////////////////////////////////// OBJECT
namespace Object
{
	Shader cubeShader("box_vertexShader.vs", "cube_fragmentShader.fs", "cube_geometryShader.gs", "wood.png", false);
	Shader floorShader("cube_vertexShader.vs", "cube_fragmentShader.fs", "cube_geometryShader.gs", "alfombra.png", false);
	Shader camaroShader("car_vertexShader.vs", "car_fragmentShader.fs", "car_geometryShader.gs", "Camaro_AlbedoTransparency_alt.png", true);
	Shader mirrorPlaneShader("cube_vertexShader.vs", "cube_fragmentShader.fs", "cube_geometryShader.gs", "wood.png", false);
	Shader windowPlaneShader("cube_vertexShader.vs", "window_fragmentShader.fs", "cube_geometryShader.gs", "red.png", false);

	Model cubeModel("newCube.obj");
	Model floorModel("groundPlane.obj");
	Model camaroModel("Camaro.obj");
	Model mirrorPlaneModel("basicPlane.obj");
	Model windowPlaneModel("basicPlane.obj");

	std::vector<int> randomOffset;
	std::vector<int> randomPos;
	std::vector<glm::mat4> camaroObjMats;
	std::vector<glm::mat4> boxObjMats;
	glm::vec3 prevCamaroPos;
	glm::vec3 currCamaroPos;
	glm::vec3 camaroForwardVec;
	bool inFreeCam = true;
	float windowAlpha = 0.75f;

	void setup()
	{
		randomOffset.resize(10);
		randomPos.resize(10);
		for (int i = 0; i < randomOffset.size(); i++)
		{
			randomOffset[i] = rand() % (40 - 10 + 1) + 10;
			randomPos[i] = rand() % (40 - -40 + 1) + -40;
		}

		camaroObjMats.resize(10);
		for (int i = 0; i < camaroObjMats.size(); i++)
		{
			camaroObjMats[i] = glm::translate(glm::mat4(), glm::vec3(glm::sin(randomOffset[i]) * 10.f, 0.f, glm::sin(randomOffset[i]) * 10.f))
							 * glm::rotate(glm::mat4(), randomOffset[i] + 90.f, glm::vec3(0.f, glm::degrees(randomOffset[i] + 90.f), 0.f))
							 * glm::scale(glm::mat4(), glm::vec3(0.05f));
		}

		boxObjMats.resize(10);
		for (int i = 0; i < boxObjMats.size(); i++)
		{
			boxObjMats[i] = glm::translate(glm::mat4(), glm::vec3(randomPos[i], 0.f, randomOffset[i]))
						  * glm::scale(glm::mat4(), glm::vec3(0.2f));
		}

		//Framebuffer::SetupFBO();

		
		//Inicialitzar el Shader 
		cubeShader.CreateAllShaders();
		camaroShader.CreateAllShaders();
		floorShader.CreateAllShaders();
		mirrorPlaneShader.CreateAllShaders();
		windowPlaneShader.CreateAllShaders();

		//Create the vertex array object
		cubeModel.CreateVertexArrayObject();
		camaroModel.CreateVertexArrayObject();
		floorModel.CreateVertexArrayObject();
		mirrorPlaneModel.CreateVertexArrayObject();
		windowPlaneModel.CreateVertexArrayObject();

		// Texture
		cubeShader.GenerateTexture();
		camaroShader.GenerateTexture();
		floorShader.GenerateTexture();
		mirrorPlaneShader.GenerateTexture();
		windowPlaneShader.GenerateTexture();

		// Clean
		glBindVertexArray(0);
	}

	void cleanup()
	{
		cubeShader.DeleteProgram();
		camaroShader.DeleteProgram();
		floorShader.DeleteProgram();
		mirrorPlaneShader.DeleteProgram();
		windowPlaneShader.DeleteProgram();

		cubeModel.Cleanup();
		camaroModel.Cleanup();
		floorModel.Cleanup();
		mirrorPlaneModel.Cleanup();
		windowPlaneModel.Cleanup();
	}

	void DrawCubeFBOTex(glm::vec4 fragColor, float time)
	{
		// We store the current values in a temporary variable
		glm::mat4 t_mvp = RenderVars::_MVP;
		glm::mat4 t_mv = RenderVars::_modelView;
		
		// We set up our framebuffer and draw into it
		glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer::fbo);
		glClearColor(1.f, 1.f, 1.f, 1.f);
		glViewport(0, 0, 800, 800);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		RenderVars::_MVP = RenderVars::_projection;
		RenderVars::_modelView = glm::mat4(1.f);
		
		// Everything you want to draw in your texture should go here
		glm::mat4 objMat = glm::lookAt(glm::vec3(0.f, 1.5f, 3.5f), glm::vec3(0.f, 1.5f, 0.f), glm::vec3(0.f, 1.f, 0.f));

		// == DRAW SCENE ==

		// We restore the previous conditions
		RenderVars::_MVP = t_mvp;
		RenderVars::_modelView = t_mv;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1.f, 1.f, 1.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		// We set up a texture where to draw our FBO:
		glViewport(0, 0, 800, 800);
		glBindTexture(GL_TEXTURE_2D, Framebuffer::fbo_tex);

		// == DRAW TEXTURE ==
	}

	void render()
	{
		glm::vec4 fragColor;
		fragColor = glm::vec4(0.5f, 0.5f, 0.5f, 0.5f);
		float time = ImGui::GetTime();
		
		// == CUBE ==
		cubeShader.UseProgram();
		cubeModel.BindVertex();

		// Texture
		cubeShader.ActivateTexture();

		cubeModel.SetLocation(glm::vec3(10.f, 0.f, -10.f));
		cubeModel.SetScale(glm::vec3(0.2f));
		cubeModel.SetUniforms(cubeShader, RenderVars::_modelView, RenderVars::_MVP, fragColor);

		cubeModel.DrawArraysTrianglesInstanced(boxObjMats, cubeShader);
		//cubeModel.DrawArraysTriangles();
		// ======

		// == FLOOR ==
		floorShader.UseProgram();
		floorModel.BindVertex();

		floorShader.ActivateTexture();

		floorModel.SetScale(glm::vec3(0.4f));
		floorModel.SetUniforms(floorShader, RenderVars::_modelView, RenderVars::_MVP, fragColor);

		floorModel.DrawArraysTriangles();
		// ======


		// == CAMARO ==
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 1, 0xff);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(0xff);

		glDisable(GL_CULL_FACE); // Disable it to draw even if normal is pointing outwards
		camaroShader.UseProgram();
		camaroModel.BindVertex();

		camaroShader.ActivateTexture();

		prevCamaroPos = camaroModel.GetLocation();
		currCamaroPos = glm::vec3(glm::sin(time) * 30.f, 0.f, glm::cos(time) * 30.f);

		// Change position (transalte)
		camaroModel.SetLocation(currCamaroPos);
		camaroForwardVec = glm::normalize(currCamaroPos - prevCamaroPos);

		// Change y-rotation (rotate)
		camaroModel.SetRoatationAngle(time + 90.f);
		camaroModel.SetRotation(glm::vec3(0.f, glm::degrees(camaroModel.GetRotationAngle()), 0.f));

		// Change size (scale)
		camaroModel.SetScale(glm::vec3(0.05f));

		camaroModel.SetUniforms(camaroShader, RenderVars::_modelView, RenderVars::_MVP, glm::vec4(5.f, 5.f, 5.f, 0.8f));

		camaroObjMats[0] = camaroModel.GetModelMatrix();
		for (int i = 1; i < camaroObjMats.size(); i++)
		{
			camaroObjMats[i] = glm::translate(glm::mat4(), glm::vec3(glm::sin(time + randomOffset[i]) * randomOffset[i], 0.f, glm::cos(time + randomOffset[i]) * randomOffset[i]))
				* glm::rotate(glm::mat4(), time + randomOffset[i] + 90.f, glm::vec3(0.f, glm::degrees(time + randomOffset[i] + 90.f), 0.f))
				* glm::scale(glm::mat4(), glm::vec3(0.05f));
		}
		camaroModel.DrawArraysTrianglesInstanced(camaroObjMats, camaroShader);
		//camaroModel.DrawArraysTriangles();
		glEnable(GL_CULL_FACE);
		// ======


		// == MIRROR ==
		//mirrorPlaneShader.GenerateFramebufferTexture();
		//DrawCubeFBOTex(fragColor, time);

		mirrorPlaneShader.UseProgram();
		mirrorPlaneModel.BindVertex();

		mirrorPlaneShader.ActivateTexture();

		mirrorPlaneModel.SetLocation(camaroModel.GetLocation() + glm::vec3(0.f, 4.f, 0.f));
		mirrorPlaneModel.SetRoatationAngle(time + 0.5f);
		mirrorPlaneModel.SetRotation(glm::vec3(0.f, glm::degrees(mirrorPlaneModel.GetRotationAngle()), 0.f));
		mirrorPlaneModel.SetScale(glm::vec3(0.05f));

		mirrorPlaneModel.SetUniforms(mirrorPlaneShader, RenderVars::_modelView, RenderVars::_MVP, fragColor);

		mirrorPlaneModel.DrawArraysTriangles();
		// ======


		// == WINDOW ==
		// Alpha blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		if (!inFreeCam)
		{
			glStencilFunc(GL_GREATER, 1, 0xff);
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

			windowPlaneShader.UseProgram();
			windowPlaneModel.BindVertex();

			windowPlaneShader.ActivateTexture();

			windowPlaneModel.SetLocation(camaroModel.GetLocation() + glm::vec3(0.01f, 4.f, 0.01f));
			windowPlaneModel.SetRoatationAngle(time + 0.5f);
			windowPlaneModel.SetRotation(glm::vec3(0.f, glm::degrees(mirrorPlaneModel.GetRotationAngle()), 0.f));
			windowPlaneModel.SetScale(glm::vec3(0.1f, 0.1f, 0.4f));
			windowPlaneModel.SetUniforms(windowPlaneShader, RenderVars::_modelView, RenderVars::_MVP, fragColor, windowAlpha);

			windowPlaneModel.DrawArraysTriangles();
		}

		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
		// ======

		glBindVertexArray(0);
	}
}

////////////////////////////////////////////////// AXIS
namespace Axis 
{
	GLuint AxisVao;
	GLuint AxisVbo[3];
	GLuint AxisShader[2];
	GLuint AxisProgram;

	float AxisVerts[] = {
		0.0, 0.0, 0.0,
		1.0, 0.0, 0.0,
		0.0, 0.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 0.0, 0.0,
		0.0, 0.0, 1.0
	};

	float AxisColors[] = {
		1.0, 0.0, 0.0, 1.0,
		1.0, 0.0, 0.0, 1.0,
		0.0, 1.0, 0.0, 1.0,
		0.0, 1.0, 0.0, 1.0,
		0.0, 0.0, 1.0, 1.0,
		0.0, 0.0, 1.0, 1.0
	};

	GLubyte AxisIdx[] = {
		0, 1,
		2, 3,
		4, 5
	};

	const char* Axis_vertShader =
		"#version 330\n\
		in vec3 in_Position;\n\
		in vec4 in_Color;\n\
		out vec4 vert_color;\n\
		uniform mat4 mvpMat;\n\
		void main() {\n\
			vert_color = in_Color;\n\
			gl_Position = mvpMat * vec4(in_Position, 1.0);\n\
		}";

	const char* Axis_fragShader =
		"#version 330\n\
		in vec4 vert_color;\n\
		out vec4 out_Color;\n\
		void main() {\n\
			out_Color = vert_color;\n\
		}";

	void setupAxis() 
	{
		glGenVertexArrays(1, &AxisVao);
		glBindVertexArray(AxisVao);
		glGenBuffers(3, AxisVbo);

		glBindBuffer(GL_ARRAY_BUFFER, AxisVbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, AxisVerts, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, AxisVbo[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, AxisColors, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)1, 4, GL_FLOAT, false, 0, 0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, AxisVbo[2]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte) * 6, AxisIdx, GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		AxisShader[0] = compileShader(Axis_vertShader, GL_VERTEX_SHADER, "AxisVert");
		AxisShader[1] = compileShader(Axis_fragShader, GL_FRAGMENT_SHADER, "AxisFrag");

		AxisProgram = glCreateProgram();
		glAttachShader(AxisProgram, AxisShader[0]);
		glAttachShader(AxisProgram, AxisShader[1]);
		glBindAttribLocation(AxisProgram, 0, "in_Position");
		glBindAttribLocation(AxisProgram, 1, "in_Color");
		linkProgram(AxisProgram);
	}

	void cleanupAxis() 
	{
		glDeleteBuffers(3, AxisVbo);
		glDeleteVertexArrays(1, &AxisVao);

		glDeleteProgram(AxisProgram);
		glDeleteShader(AxisShader[0]);
		glDeleteShader(AxisShader[1]);
	}

	void drawAxis() 
	{
		glBindVertexArray(AxisVao);
		glUseProgram(AxisProgram);
		glUniformMatrix4fv(glGetUniformLocation(AxisProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RV::_MVP));
		glDrawElements(GL_LINES, 6, GL_UNSIGNED_BYTE, 0);

		glUseProgram(0);
		glBindVertexArray(0);
	}
}
/////////////////////////////////////////////////


void GLinit(int width, int height) 
{
	glViewport(0, 0, width, height);
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
	glClearDepth(1.f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	RV::_projection = glm::perspective(RV::FOV, (float)width / (float)height, RV::zNear, RV::zFar);
	//RV::_projection = glm::ortho(RV::FOV, (float)width / (float)height, RV::zNear, RV::zFar);

	// Setup shaders & geometry
	Axis::setupAxis();
	Object::setup();
	//Cube::setupCube();
}

void GLcleanup() 
{
	Axis::cleanupAxis();
	Object::cleanup();
	//Cube::cleanupCube();
}

void GLrender(float dt) 
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	time_t currentTime = SDL_GetTicks() / 1000;

	const GLfloat color[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glClearBufferfv(GL_COLOR, 0, color);

	RV::_modelView = glm::mat4(1.f);
	if (Object::inFreeCam)
	{
		RV::_modelView = glm::translate(RV::_modelView, glm::vec3(RV::panv[0], RV::panv[1], RV::panv[2]));
		RV::_modelView = glm::rotate(RV::_modelView, RV::rota[1], glm::vec3(1.f, 0.f, 0.f));
		RV::_modelView = glm::rotate(RV::_modelView, RV::rota[0], glm::vec3(0.f, 1.f, 0.f));
	}
	else
	{
		RV::_modelView = glm::rotate(RV::_modelView, glm::radians(-Object::camaroModel.GetRotation().y + 180.f), glm::vec3(0.f, 1.f, 0.f));
		//RV::_modelView = glm::rotate(RV::_modelView, RV::rota[0], glm::vec3(0.f, 1.f, 0.f));

		glm::vec3 camaroCameraPos{ Object::camaroModel.GetLocation() + (Object::camaroForwardVec * -1.5f) + glm::vec3(0.f, 7.f, 0.f) };
		RV::_modelView = glm::translate(RV::_modelView, -camaroCameraPos);
	}
	

	RV::_MVP = RV::_projection * RV::_modelView;

	Axis::drawAxis();
	//Cube::drawCube();
	Object::render();

	ImGui::Render();
}


void GUI() 
{
	bool show = true;
	ImGui::Begin("Physics Parameters", &show, 0);

	{
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		if (ImGui::Button("Change Camera"))
		{
			Object::inFreeCam = !Object::inFreeCam;
		}

		ImGui::SliderFloat("Window Alpha", &Object::windowAlpha, 0.f, 1.f);
	}

	ImGui::End();

	// Example code -- ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	bool show_test_window = false;
	if (show_test_window) 
	{
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}