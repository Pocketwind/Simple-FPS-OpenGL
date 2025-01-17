#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <ShaderReader.h>
#include <vector>
#include <math.h>
#include "DataStruct.h"
#include "LoadOBJ.h"

using namespace std;
using namespace glm;

//기본 설정값들
const char* TITLE = "Simple FPS OpenGL";
const int TARGET_FPS = 30;
const int SCREEN_WIDTH = 1600;
const int SCREEN_HEIGHT = 900;
const int INITIAL_X = -5;
const int INITIAL_Y = 10;
const int INITIAL_Z = 5;
const int ARM_DISTANCE = 3;
const float MOVE = 0.01;
const float SLIDE = 0.8;
const float MOUSE_SPEED = 0.001;
const float SLIDE_THRESHOLD = 0.001;

//GLSL 불러오기
extern const char* vs, * fs, * vs_base, * fs_base;
//Cube vertex
extern const float cube_v[], cube_vn[], cube_vt[];

//헤더
GLuint InitializeBase();
GLuint InitializeObject(OBJ&);
GLuint CreateShader(const char*, const char*);
void CursorCallback(GLFWwindow*, double, double);
void key_callback(GLFWwindow*, int, int, int, int);
void InputControl(GLFWwindow*);

//전역	변수
//Cam1 초기화 
Cam cam1;

int main() 
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	if (!glfwInit())
	{
		cerr << "Failed to initialize GLFW" << endl;
		return -1;
	}
	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, TITLE, NULL, NULL);
	if (!window) 
	{
		cerr << "Failed to create window" << endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, CursorCallback);
	//Vsync
	glfwSwapInterval(1);
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << endl;
		return -1;
	}

	//object shader 로드
	//OBJ file
	OBJ obj_cube = OBJ("cube.obj", false);
	OBJ obj_sphere = OBJ("sphere.obj", false);
	OBJ obj_paimon = OBJ("paimon.obj", false);
	GLuint cube = InitializeObject(obj_cube);
	GLuint sphere = InitializeObject(obj_sphere);
	GLuint paimon = InitializeObject(obj_paimon);
	//const char* vs = textFileRead("vertex.vs");
	//const char* fs = textFileRead("fragment.fs");
	GLuint shader = CreateShader(vs, fs);
	GLuint MmatLoc = glGetUniformLocation(shader, "Mmat");
	GLuint AmatLoc = glGetUniformLocation(shader, "Amat");
	GLuint camVecLoc = glGetUniformLocation(shader, "camVec");
	GLuint vertexColorLoc = glGetUniformLocation(shader, "vertexColor");
	GLuint ambientStrengthLoc = glGetUniformLocation(shader, "ambientStrength_in");
	GLuint specularStrengthLoc = glGetUniformLocation(shader, "specularStrength_in");
	GLuint shininessLoc = glGetUniformLocation(shader, "shininess_in");
	GLuint diffuseStrengthLoc = glGetUniformLocation(shader, "diffuseStrength_in");

	//base shader 로드
	GLuint base = InitializeBase();
	GLuint shader_base = CreateShader(vs_base, fs_base);
	GLuint AmatLoc_base = glGetUniformLocation(shader_base, "Amat");


	//free(vs);
	//free(fs);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);

	//cam1 초기화
	cam1.cam = 0;
	cam1.distance = ARM_DISTANCE;
	cam1.up = vec3(0, 1, 0);
	cam1.pos = vec3(INITIAL_X, INITIAL_Y, INITIAL_Z);
	cam1.target = vec3(0, 0, 0);
	cam1.width = SCREEN_WIDTH;
	cam1.height = SCREEN_HEIGHT;
	cam1.aspect = (float)cam1.width / (float)cam1.height;


	double lastTime = 0;
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	while (!glfwWindowShouldClose(window))
	{
		double currentTime = glfwGetTime();
		double delta = currentTime - lastTime;

		//Input
		InputControl(window);

		//렌더링 시작
		double frameStart = glfwGetTime();
		//버퍼
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		//커서 중앙으로 고정
		glfwSetCursorPos(window, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

		//캠1 update
		cam1.viewVector = vec3(
			cos(cam1.angle1) * cos(cam1.angle2),
			sin(cam1.angle2),
			sin(cam1.angle1) * cos(cam1.angle2)
		);
		//pos update
		cam1.pos = cam1.pos + cam1.inertia * vec3(delta);
		//inertia update
		cam1.inertia *= SLIDE;
		if (cam1.inertia.x < SLIDE_THRESHOLD && cam1.inertia.x > -SLIDE_THRESHOLD)
			cam1.inertia.x = 0;
		if (cam1.inertia.y < SLIDE_THRESHOLD && cam1.inertia.y > -SLIDE_THRESHOLD)
			cam1.inertia.y = 0;
		if (cam1.inertia.z < SLIDE_THRESHOLD && cam1.inertia.z > -SLIDE_THRESHOLD)
			cam1.inertia.z = 0;
		
		//target update
		cam1.target = cam1.pos + cam1.viewVector;
		//armpos update
		cam1.armPos = cam1.pos + cam1.viewVector * cam1.distance;

		//Projection Matrix
		mat4 P = perspective(cam1.fov, cam1.aspect, cam1.near, cam1.far);
		//View Matrix
		mat4 V = lookAt(cam1.pos, cam1.target, cam1.up);
		//Model Matrix
		mat4 M = mat4(1.0f);
		//MVP Matrix
		mat4 MVP = P * V * M;

		//Base 그리기
		glUseProgram(shader_base);
		glBindVertexArray(base);
		glUniformMatrix4fv(AmatLoc_base, 1, GL_FALSE, &MVP[0][0]);
		glDrawArrays(GL_LINES, 0, 6);

		//Object 그리기
		glUseProgram(shader);
		glBindVertexArray(cube);
		glUniformMatrix4fv(MmatLoc, 1, GL_FALSE, &M[0][0]);
		glUniformMatrix4fv(AmatLoc, 1, GL_FALSE, &MVP[0][0]);
		glUniform3fv(camVecLoc, 1, &cam1.pos[0]);
		glUniform3fv(vertexColorLoc, 1, &vec3(1, 1, 1)[0]);
		glUniform1f(ambientStrengthLoc, 0.1);
		glUniform1f(specularStrengthLoc, 0.5);
		glUniform1f(shininessLoc, 32);
		glUniform1f(diffuseStrengthLoc, 0.5);
		glDrawArrays(GL_TRIANGLES, 0, obj_cube.polygons);
		
		
		glBindVertexArray(sphere);
		glUniformMatrix4fv(MmatLoc, 1, GL_FALSE, &M[0][0]);
		glUniformMatrix4fv(AmatLoc, 1, GL_FALSE, &MVP[0][0]);
		glUniform3fv(camVecLoc, 1, &cam1.pos[0]);
		glUniform3fv(vertexColorLoc, 1, &vec3(1, 1, 1)[0]);
		glUniform1f(ambientStrengthLoc, 0.1);
		glUniform1f(specularStrengthLoc, 0.5);
		glUniform1f(shininessLoc, 32);
		glUniform1f(diffuseStrengthLoc, 0.5);
		glDrawArrays(GL_TRIANGLES, 0, obj_sphere.polygons);

		/*
		glBindVertexArray(paimon);
		glUniformMatrix4fv(MmatLoc, 1, GL_FALSE, &M[0][0]);
		glUniformMatrix4fv(AmatLoc, 1, GL_FALSE, &MVP[0][0]);
		glUniform3fv(camVecLoc, 1, &cam1.pos[0]);
		glUniform3fv(vertexColorLoc, 1, &vec3(1, 1, 1)[0]);
		glUniform1f(ambientStrengthLoc, 0.1);
		glUniform1f(specularStrengthLoc, 0.5);
		glUniform1f(shininessLoc, 32);
		glUniform1f(diffuseStrengthLoc, 0.5);
		glDrawArrays(GL_TRIANGLES, 0, obj_paimon.polygons);
		*/



		//버퍼
		glCullFace(GL_BACK);
		glfwSwapBuffers(window);
		glfwPollEvents();

		double frameEnd = glfwGetTime();
		double frameDelta = frameEnd - frameStart;

		cout << "FPS: " << 1/delta << endl;
		cout << "Pos: " << cam1.pos.x << " " << cam1.pos.y << " " << cam1.pos.z << endl;
	}

	glfwTerminate();
	cout << "Window Closed" << endl;
	return 0;
}

GLuint InitializeBase()
{
	float vertices[] =
	{
		0,0,0,
		10,0,0,
		0,0,0,
		0,10,0,
		0,0,0,
		0,0,10
	};
	float colors[] =
	{
		1,0,0,
		1,0,0,
		0,1,0,
		0,1,0,
		0,0,1,
		0,0,1
	};
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo[2];
	glGenBuffers(2, vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

	return vao;
}


GLuint InitializeObject(OBJ &model)
{
	//VAO
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	//VBO
	GLuint vbo[2];
	glGenBuffers(2, vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, model.trivertex.size() * sizeof(float), &model.trivertex[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, model.trinormal.size() * sizeof(float), &model.trinormal[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

	return vao;
}


GLuint CreateShader(const char* vs, const char* fs)
{
	//버텍스 컴파일
	GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vs, NULL);
	glCompileShader(vertex);
	//fragment 컴파일
	GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fs, NULL);
	glCompileShader(fragment);
	//프로그램 링크
	GLuint program = glCreateProgram();
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);
	glLinkProgram(program);
	//제거
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	return program;
}

void CursorCallback(GLFWwindow* window, double xpos, double ypos)
{
	cam1.angle1 -= MOUSE_SPEED * (SCREEN_WIDTH / 2 - xpos);
	cam1.angle2 += MOUSE_SPEED * (SCREEN_HEIGHT / 2 - ypos);
	if (cam1.angle2 > 3.1415 / 2)
		cam1.angle2 = 3.1415 / 2;
	if (cam1.angle2 < -3.1415 / 2)
		cam1.angle2 = -3.1415 / 2;
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}
void InputControl(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		cam1.inertia += cam1.viewVector * MOVE;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		cam1.inertia -= cam1.viewVector * MOVE;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		cam1.inertia -= normalize(cross(cam1.viewVector, cam1.up)) * MOVE;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		cam1.inertia += normalize(cross(cam1.viewVector, cam1.up)) * MOVE;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		cam1.inertia.y -= 0.1;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		cam1.inertia.y += 0.1;
	}
}