#pragma once
#include <iostream>
#include <vector>
#include <map>

#include "GL\glew.h"
#include "GL\glut.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "../include/opencv2/opencv.hpp"

#include "objloader.hpp"
#include "LoadShaders.h"

#define PARTSNUM 11

using namespace std;

class Robot {
private:
	//MODEL PART
	vector<glm::vec3> _vertices[PARTSNUM];
	vector<glm::vec2> _uvs[PARTSNUM];
	vector<glm::vec3> _normals[PARTSNUM];
	vector<unsigned int> _faces[PARTSNUM];
	vector<string> _materials[PARTSNUM];
	vector<glm::vec3> _matKas[PARTSNUM];
	vector<glm::vec3> _matKds[PARTSNUM];
	vector<glm::vec3> _wholeVertices;

	glm::mat4 _modelMatrix[PARTSNUM];

	map<string, glm::vec3> _materialKas;	//mtlname - kas
	map<string, glm::vec3> _materialKds;	//mtlname - kds

	//BACKGROUND PART
	vector<glm::vec3> _bgVertices;
	vector<glm::vec2> _bgUVs;
	GLuint _bgTexture;

	//SHADER PART -- shared by all
	GLuint _VAO;

	//SHADER PART -- robot
	GLuint _program;
	GLuint _VBOs[PARTSNUM];
	GLuint _KaVBOs[PARTSNUM];
	GLuint _KdVBOs[PARTSNUM];
	GLuint _normalVBOs[PARTSNUM];

	//SHADER PART -- background
	GLuint _bgProgram;
	GLuint _bgVBOs;
	GLuint _bgUVvbos;


	//ANIMATION
	float _angle, _delta;
	bool _firstRun;
	int _runTime;

	//framebuffer
	GLuint _framebuffer;
	GLuint _frameBufferTexture;
	GLuint _frameBufferDepthTexture;

	//POST PROCESS shader
	GLuint _postProgram;
	GLuint _VBO;
	float _time;

public:
	Robot() {
		_angle = 0;
		_delta = 0.1;
		_firstRun = true;
	}

	void initFrameBuffer() {
		//init the framebuffer texture
		glGenTextures(1, &_frameBufferTexture);
		glBindTexture(GL_TEXTURE_2D, _frameBufferTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 500, 500, 0, GL_RGBA, GL_FLOAT, 0);

		GLuint depthrenderbuffer;
		glGenRenderbuffersEXT(1, &depthrenderbuffer);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthrenderbuffer);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, 500, 500);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

		glGenTextures(1, &_frameBufferDepthTexture);
		glBindTexture(GL_TEXTURE_2D, _frameBufferDepthTexture);
		glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT24, 500, 500, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		//init the framebuffer
		_framebuffer = 0;
		glGenFramebuffers(1, &_framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
		//bind the texture
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _frameBufferTexture, 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _frameBufferDepthTexture, 0);
		GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			cout << "framebuffer err\n" << endl;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, 500, 500);

		ShaderInfo shaders[3];
		shaders[0].type = GL_VERTEX_SHADER;
		shaders[0].filename = "./Shader/PostProcess.vert";
		shaders[1].type = GL_FRAGMENT_SHADER;
		shaders[1].filename = "./Shader/PostProcess.frag";
		shaders[2].type = GL_NONE;

		_postProgram = LoadShaders(shaders);
	}

	void loadBgPart() {
		_bgVertices.push_back(glm::vec3(1, 1, -1));
		_bgVertices.push_back(glm::vec3(1, -1, -1));
		_bgVertices.push_back(glm::vec3(-1, -1, -1));
		_bgVertices.push_back(glm::vec3(-1, 1, -1));

		glGenBuffers(1, &_bgVBOs);
		glBindBuffer(GL_ARRAY_BUFFER, _bgVBOs);
		glBufferData(GL_ARRAY_BUFFER, _bgVertices.size() * sizeof(glm::vec3), &_bgVertices[0], GL_STATIC_DRAW);

		_bgUVs.push_back(glm::vec2(0, 0));
		_bgUVs.push_back(glm::vec2(0, 1));
		_bgUVs.push_back(glm::vec2(1, 1));
		_bgUVs.push_back(glm::vec2(1, 0));

		glGenBuffers(1, &_bgUVvbos);
		glBindBuffer(GL_ARRAY_BUFFER, _bgUVvbos);
		glBufferData(GL_ARRAY_BUFFER, _bgUVs.size() * sizeof(glm::vec2), &_bgUVs[0], GL_STATIC_DRAW);

		IplImage* bgTexture = cvLoadImage("./bg.jpg", 1);
		glGenTextures(1, &_bgTexture);
		glBindTexture(GL_TEXTURE_2D, _bgTexture);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bgTexture->width, bgTexture->height, 0, GL_BGR, GL_UNSIGNED_BYTE, (GLvoid*)bgTexture->imageData);
	
		//load the shaders
		ShaderInfo shaders[3];
		shaders[0].type = GL_VERTEX_SHADER;
		shaders[0].filename = "./Shader/Background.vert";
		shaders[1].type = GL_FRAGMENT_SHADER;
		shaders[1].filename = "./Shader/Background.frag";
		shaders[2].type = GL_NONE;

		_bgProgram = LoadShaders(shaders);
	}

	void load() {
		//load the mtl
		std::vector<glm::vec3> Kds;
		std::vector<glm::vec3> Kas;
		std::vector<glm::vec3> Kss;
		std::vector<std::string> Materials;//mtl-name
		std::string texture;
		loadMTL("./Mesh/Al.mtl", Kds, Kas, Kss, Materials, texture);
		for (int i = 0; i < Materials.size(); i++) {
			string mtlname = Materials[i];
			//  name            vec3
			_materialKas[mtlname] = Kas[i];
			_materialKds[mtlname] = Kds[i];
		}

		//load the objs
		std::string filePaths[PARTSNUM];
		filePaths[0]  = "./Mesh/Al_HeadTorso.obj";
		filePaths[1]  = "./Mesh/Al_RUpperArm.obj";
		filePaths[2]  = "./Mesh/Al_RLowerArm.obj";
		filePaths[3]  = "./Mesh/Al_LUpperArm.obj";
		filePaths[4]  = "./Mesh/Al_LLowerArm.obj";
		filePaths[5]  = "./Mesh/Al_RUpperLeg.obj";
		filePaths[6]  = "./Mesh/Al_RLowerLeg.obj";
		filePaths[7]  = "./Mesh/Al_LUpperLeg.obj";
		filePaths[8]  = "./Mesh/Al_LLowerLeg.obj";
		filePaths[9]  = "./Mesh/Al_FrontCloth.obj";
		filePaths[10] = "./Mesh/Al_BackCloth.obj";

		for (int i = 0; i < PARTSNUM; i++) {
			loadOBJ(filePaths[i].c_str(), _vertices[i], _uvs[i], _normals[i], _faces[i], _materials[i]);
			glGenBuffers(1, &_VBOs[i]);
			glBindBuffer(GL_ARRAY_BUFFER, _VBOs[i]);
			glBufferData(GL_ARRAY_BUFFER, _vertices[i].size() * sizeof(glm::vec3), &_vertices[i][0], GL_STATIC_DRAW);

			_wholeVertices.insert(_wholeVertices.end(), _vertices[i].begin(), _vertices[i].end());

			glGenBuffers(1, &_normalVBOs[i]);
			glBindBuffer(GL_ARRAY_BUFFER, _normalVBOs[i]);
			glBufferData(GL_ARRAY_BUFFER, _normals[i].size() * sizeof(glm::vec3), &_normals[i][0], GL_STATIC_DRAW);

			_modelMatrix[i] = glm::mat4(1);

			//take care of the materials
			int matFcount;
			for (int j = 0; j < _materials[i].size(); j++) {
				matFcount = _faces[i][j + 1];
				for (int k = 0; k < matFcount * 3; k++) {
					_matKas[i].push_back(_materialKas[_materials[i][j]]);
					_matKds[i].push_back(_materialKds[_materials[i][j]]);
				}
			}

			glGenBuffers(1, &_KaVBOs[i]);
			glBindBuffer(GL_ARRAY_BUFFER, _KaVBOs[i]);
			glBufferData(GL_ARRAY_BUFFER, _matKas[i].size() * sizeof(glm::vec3), &_matKas[i][0], GL_STATIC_DRAW);

			glGenBuffers(1, &_KdVBOs[i]);
			glBindBuffer(GL_ARRAY_BUFFER, _KdVBOs[i]);
			glBufferData(GL_ARRAY_BUFFER, _matKds[i].size() * sizeof(glm::vec3), &_matKds[i][0], GL_STATIC_DRAW);
		}

		glGenBuffers(1, &_VBO);
		glBindBuffer(GL_ARRAY_BUFFER, _VBO);
		glBufferData(GL_ARRAY_BUFFER, _wholeVertices.size() * sizeof(glm::vec3), &_wholeVertices[0], GL_STATIC_DRAW);

		glGenVertexArrays(1, &_VAO);
		glBindVertexArray(_VAO);

		//load the shaders
		ShaderInfo shaders[3];
		shaders[0].type = GL_VERTEX_SHADER;
		shaders[0].filename = "./Shader/Model.vert";
		shaders[1].type = GL_FRAGMENT_SHADER;
		shaders[1].filename = "./Shader/Model.frag";
		shaders[2].type = GL_NONE;

		_program = LoadShaders(shaders);

		loadBgPart();
		initFrameBuffer();
	}

	void idle() {
		_angle = 0;
		_delta = 0.05;
		for (int i = 0; i < PARTSNUM; i++)
			_modelMatrix[i] = glm::mat4(1);
		_firstRun = true;
	}

	void run() {
		if (_firstRun) {
			//Al_HeadTorso
			//Al_RUpperArm
			//Al_RLowerArm
			_modelMatrix[2] = glm::rotate(glm::mat4(1.0f), -60.0f, glm::vec3(1, 0, 0)) * _modelMatrix[2];
			//Al_LUpperArm
			//Al_LLowerArm
			_modelMatrix[4] = glm::rotate(glm::mat4(1.0f), -60.0f, glm::vec3(1, 0, 0)) * _modelMatrix[4];
			//Al_RUpperLeg
			//Al_RLowerLeg
			//Al_LUpperLeg
			//Al_LLowerLeg
			//Al_FrontCloth
			_modelMatrix[9] = glm::rotate(glm::mat4(1.0f), -5.0f, glm::vec3(1, 0, 0)) * _modelMatrix[9];
			//Al_BackCloth
			_modelMatrix[10] = glm::rotate(glm::mat4(1.0f), 5.0f, glm::vec3(1, 0, 0)) * _modelMatrix[10];

			_firstRun = false;
			_runTime = 0;
		}

		if ((int)_angle / 30 == 0 && _angle > 10)
			_delta = -0.05;
		else if ((int)_angle / -60 == 0 && _angle < -10)
			_delta = 0.05;
		_angle += _delta;

		//Al_HeadTorso
		//Al_RUpperArm
		_modelMatrix[1] = glm::rotate(glm::mat4(1.0f), _delta, glm::vec3(1, 0, 0)) * _modelMatrix[1];
		//Al_RLowerArm
		_modelMatrix[2] = glm::rotate(glm::mat4(1.0f), _delta, glm::vec3(1, 0, 0)) * _modelMatrix[2];
		//Al_LUpperArm
		_modelMatrix[3] = glm::rotate(glm::mat4(1.0f), -_delta, glm::vec3(1, 0, 0)) * _modelMatrix[3];
		//Al_LLowerArm
		_modelMatrix[4] = glm::rotate(glm::mat4(1.0f), -_delta, glm::vec3(1, 0, 0)) * _modelMatrix[4];
		//Al_RUpperLeg
		_modelMatrix[5] = glm::rotate(glm::mat4(1.0f), -_delta, glm::vec3(1, 0, 0)) * _modelMatrix[5];
		//Al_RLowerLeg
		_modelMatrix[6] = glm::rotate(glm::mat4(1.0f), -_delta, glm::vec3(1, 0, 0)) * _modelMatrix[6];
		//Al_LUpperLeg
		_modelMatrix[7] = glm::rotate(glm::mat4(1.0f), _delta , glm::vec3(1, 0, 0)) * _modelMatrix[7];
		//Al_LLowerLeg
		_modelMatrix[8] = glm::rotate(glm::mat4(1.0f), _delta , glm::vec3(1, 0, 0)) * _modelMatrix[8];
		//Al_LLowerLeg
		//Al_FrontCloth
		//Al_BackCloth

		for (int i = 0; i < PARTSNUM; i++) {
			_modelMatrix[i] = glm::translate(glm::mat4(1.0f), glm::vec3(0, _delta / 50.0f, 0)) * _modelMatrix[i];
		}
	}

	void renderPerPart(glm::mat4 projectionMatrix, glm::mat4 viewMatrix, int i, int stype = 0) {
		glBindVertexArray(_VAO);
		glUseProgram(_program);

		glUniformMatrix4fv(glGetUniformLocation(_program, "ProjectionMatrix"), 1, false, &projectionMatrix[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(_program, "ViewMatrix"), 1, false, &viewMatrix[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(_program, "ModelMatrix"), 1, false, &_modelMatrix[i][0][0]);

		glUniform1i(glGetUniformLocation(_program, "shadingType"), stype);

		glBindBuffer(GL_ARRAY_BUFFER, _VBOs[i]);
		glEnableVertexAttribArray(0);
		glBufferData(GL_ARRAY_BUFFER, _vertices[i].size() * sizeof(glm::vec3), &_vertices[i][0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, _normalVBOs[i]);
		glEnableVertexAttribArray(1);
		glBufferData(GL_ARRAY_BUFFER, _normals[i].size() * sizeof(glm::vec3), &_normals[i][0], GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, _KaVBOs[i]);
		glEnableVertexAttribArray(2);
		glBufferData(GL_ARRAY_BUFFER, _matKas[i].size() * sizeof(glm::vec3), &_matKas[i][0], GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, _KdVBOs[i]);
		glEnableVertexAttribArray(3);
		glBufferData(GL_ARRAY_BUFFER, _matKds[i].size() * sizeof(glm::vec3), &_matKds[i][0], GL_STATIC_DRAW);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glDrawArrays(GL_TRIANGLES, 0, _vertices[i].size());

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);

		glUseProgram(0);
	}

	void renderBg() {
		glBindVertexArray(_VAO);
		glUseProgram(_bgProgram);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _bgTexture);
		glUniform1i(glGetUniformLocation(_bgProgram, "Texture"), 0);

		glBindBuffer(GL_ARRAY_BUFFER, _bgVBOs);
		glEnableVertexAttribArray(0);
		glBufferData(GL_ARRAY_BUFFER, _bgVertices.size() * sizeof(glm::vec3), &_bgVertices[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, _bgUVvbos);
		glEnableVertexAttribArray(1);
		glBufferData(GL_ARRAY_BUFFER, _bgUVs.size() * sizeof(glm::vec2), &_bgUVs[0], GL_STATIC_DRAW);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glDrawArrays(GL_QUADS, 0, _bgVertices.size());

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		glUseProgram(0);
	}

	void postProcessRender(glm::mat4 projectionMatrix, glm::mat4 viewMatrix, int type) {
		glBindVertexArray(_VAO);
		glUseProgram(_postProgram);

		//glUniformMatrix4fv(glGetUniformLocation(_postProgram, "ProjectionMatrix"), 1, false, &projectionMatrix[0][0]);
		//glUniformMatrix4fv(glGetUniformLocation(_postProgram, "ViewMatrix"), 1, false, &viewMatrix[0][0]);
		
		glUniform1f(glGetUniformLocation(_postProgram, "time"), _time);
		glUniform1i(glGetUniformLocation(_postProgram, "shaderType"), type);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, _frameBufferTexture);
		glUniform1i(glGetUniformLocation(_postProgram, "Texture"), 1);

		glBindBuffer(GL_ARRAY_BUFFER, _bgVBOs);
		glEnableVertexAttribArray(0);
		glBufferData(GL_ARRAY_BUFFER, _bgVertices.size() * sizeof(glm::vec3), &_bgVertices[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, _bgUVvbos);
		glEnableVertexAttribArray(1);
		glBufferData(GL_ARRAY_BUFFER, _bgUVs.size() * sizeof(glm::vec2), &_bgUVs[0], GL_STATIC_DRAW);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

		//glBindBuffer(GL_ARRAY_BUFFER, _VBO);
		//glEnableVertexAttribArray(0);
		//glBufferData(GL_ARRAY_BUFFER, _wholeVertices.size() * sizeof(glm::vec3), &_wholeVertices[0], GL_STATIC_DRAW);
		//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glDrawArrays(GL_QUADS, 0, _bgVertices.size());

		glDisableVertexAttribArray(0);

		glUseProgram(0);
	}

	void render(glm::mat4 projectionMatrix, glm::mat4 viewMatrix, int stype, int actionType) {
		int shader3DMode = 0;
		if (stype < 2)
			shader3DMode = stype;

		if (actionType == 0)
			idle();
		else if (actionType == 1)
			run();

		if (stype >= 2) {
			glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
			glViewport(0, 0, 500, 500);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		renderBg();

		glEnable(GL_DEPTH_TEST);
		for (int i = 0; i < PARTSNUM; i++) {
			//cout << i << " " << _matKds[i][0][0] << " " << _matKds[i][0][1] << " " << _matKds[i][0][2] << endl;
			renderPerPart(projectionMatrix, viewMatrix, i, shader3DMode);
		}
		glDisable(GL_DEPTH_TEST);

		if (stype >= 2) {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, 500, 500);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			postProcessRender(projectionMatrix, viewMatrix, stype);
		}

		_time += 0.001;
	}

	void resetTime() {
		_time = 0.0;
	}
};