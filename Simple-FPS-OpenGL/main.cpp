#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <ShaderReader.h>
#include <vector>
#include <math.h>

using namespace std;
using namespace glm;

const char* TITLE = "Simple FPS OpenGL";
const int TARGET_FPS = 60;
const int SCREEN_WIDTH = 1600;
const int SCREEN_HEIGHT = 900;
const int INITIAL_X = -5;
const int INITIAL_Y = 10;
const int INITIAL_Z = 5;
const int ARM_DISTANCE = 3;
const float MOUSE_SPEED = 0.001;


const char* vs = R"(
#version 330 core
layout (location = 0) in vec3 vertexPosition; 
layout (location = 1) in vec3 vertexNormal; 
out vec3 fragmentColor;
out vec3 viewpos;
out vec3 fragpos;
out vec3 normal;
out float ambientStrength;
out float specularStrength;
out float shininess;
out float diffuseStrength;

uniform mat4 Amat; 
uniform mat4 Mmat; 
uniform vec3 camVec;
uniform vec3 vertexColor;
uniform float ambientStrength_in;
uniform float specularStrength_in;
uniform float shininess_in;
uniform float diffuseStrength_in;


void main()
{
    vec4 point = vec4(vertexPosition, 1.0);
    gl_Position = Amat * point;
    
    // geometric information
    viewpos = camVec;
    fragpos = vec3(Mmat*point); // world (global) coordinates
    normal = normalize(inverse(transpose(mat3(Mmat)))*vertexNormal);

    // Material properties
    vec3 material_color = vertexColor;
    ambientStrength = ambientStrength_in;
    specularStrength = specularStrength_in;
    shininess = shininess_in;
    diffuseStrength = diffuseStrength_in;
    
    fragmentColor = material_color;

}
)";
const char* fs = R"(
#version 330 core
in vec3 viewpos;
in vec3 fragpos;
in vec3 normal;
in vec3 fragmentColor;
in float ambientStrength;
in float specularStrength;
in float shininess;
in float diffuseStrength;
out vec4 fragmentColorOut;

struct phLight { 
    vec3 color; 
    vec3 position; 
};

vec3 LightingModel(vec3 material_color, vec3 lightColor, 
        vec3 lightPosition, vec3 fragpos, vec3 normal, vec3 viewpos, 
        float ambientStrength_in, float specularStrength_in, float shininess_in, float diffuseStrength_in)
{
    //ambient
    vec3 ambient = ambientStrength_in*material_color*lightColor;

    // diffuse
    vec3 Lvec=normalize(lightPosition-fragpos);
    float cosTheta=max(dot(Lvec,normal),0.0);
    vec3 D=diffuseStrength_in*material_color;
    vec3 diffuse = D*cosTheta*lightColor;

    // specular
    vec3 Vvec=normalize(viewpos-fragpos);
    vec3 Rvec=2*dot(Lvec,normal)*normal-Lvec;
    float exponent=shininess_in;   //32
    float specFactor=pow(max(dot(Rvec,Vvec),0.0),exponent);
    vec3 specular = specFactor*specularStrength_in*lightColor;

    vec3 color =  ambient + diffuse + specular;
    return color;
}

void main()
{
    phLight light;
    light=phLight(vec3(1,1,1),vec3(0,8,0));
    vec3 color = LightingModel(fragmentColor,light.color,
                                light.position,fragpos,normal,viewpos,
                                ambientStrength,specularStrength,shininess,diffuseStrength);
    fragmentColorOut = vec4(color, 1.0);
}
)";

const char* vs_base = R"(
#version 330 core
layout (location = 0) in vec3 vertexPosition; 
layout (location = 1) in vec3 vertexColor; 
out vec3 fragmentColor;
uniform mat4 Amat; 
void main()
{
    vec4 point = vec4(vertexPosition, 1.0);
    gl_Position = Amat * point;
    fragmentColor = vertexColor;
}
)";

const char* fs_base = R"(
#version 330 core
in vec3 fragmentColor;
out vec4 fragmentColorOut;
void main()
{
   fragmentColorOut = vec4(fragmentColor, 1.0); // alpha
}
)";

struct Model
{
	vector<float> trivertex;
	vector<float> trinormal;
};
struct Cam
{
	bool shoot = false;
	bool grab = false;
	bool noclip = false;
	int grabbedObject = 0;
	int cam = 0;
	int height = SCREEN_HEIGHT;
	int width = SCREEN_WIDTH;
	int gravityTimer = 0;
	int tall = 3;
	float distance = ARM_DISTANCE;
	float fov = 45;
	float aspect = (float)width / (float)height;
	float near = 0.1;
	float far = 1000;
	float angle1 = 0;
	float angle2 = 0;
	vec3 viewVector = vec3(0, 0, 0);
	vec3 armPos = vec3(0, 0, 0);
	vec3 up = vec3(0, 1, 0);
	vec3 pos = vec3(INITIAL_X, INITIAL_Y, INITIAL_Z);
	vec3 target = vec3(0, 0, 0);
};

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) 
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) 
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
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


GLuint InitializeObject(Model& model)
{
	//VAO
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	//VBO
	GLuint vbo[2];
	glGenBuffers(2, vbo);
	//버텍스
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, model.trivertex.size() * sizeof(float), model.trivertex.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	//노말
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, model.trinormal.size() * sizeof(float), model.trinormal.data(), GL_STATIC_DRAW);
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

//Cam1 초기화 
Cam cam1;

void CursorCallback(GLFWwindow* window, double xpos, double ypos)
{
	cam1.angle1 -= MOUSE_SPEED * (SCREEN_WIDTH / 2 - xpos);
	cam1.angle2 += MOUSE_SPEED * (SCREEN_HEIGHT / 2 - ypos);
	if (cam1.angle2 > 3.1415 / 2)
		cam1.angle2 = 3.1415 / 2;
	if (cam1.angle2 < -3.1415 / 2)
		cam1.angle2 = -3.1415 / 2;
}

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
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << endl;
		return -1;
	}

	//object shader 로드
	//const char* vs = textFileRead("vertex.vs");
	//const char* fs = textFileRead("fragment.fs");
	GLuint shader_object = CreateShader(vs, fs);
	GLuint MmatLoc = glGetUniformLocation(shader_object, "Mmat");
	GLuint camVecLoc = glGetUniformLocation(shader_object, "camVec");
	GLuint vertexColorLoc = glGetUniformLocation(shader_object, "vertexColor");
	GLuint ambientStrengthLoc = glGetUniformLocation(shader_object, "ambientStrength_in");
	GLuint specularStrengthLoc = glGetUniformLocation(shader_object, "specularStrength_in");
	GLuint shininessLoc = glGetUniformLocation(shader_object, "shininess_in");
	GLuint diffuseStrengthLoc = glGetUniformLocation(shader_object, "diffuseStrength_in");

	//base shader 로드
	GLuint base = InitializeBase();
	GLuint shader = CreateShader(vs_base, fs_base);
	GLuint AmatLoc = glGetUniformLocation(shader, "Amat");

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


	while (!glfwWindowShouldClose(window))
	{
		double timePerFrame = glfwGetTime();
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
		glUseProgram(shader);
		glBindVertexArray(base);
		glUniformMatrix4fv(AmatLoc, 1, GL_FALSE, &MVP[0][0]);
		glDrawArrays(GL_LINES, 0, 6);

		glCullFace(GL_BACK);
		glfwSwapBuffers(window);
		glfwPollEvents();

		double timeElapsed = glfwGetTime() - timePerFrame;
		cout << "FPS: " << 1 / timeElapsed << endl;
	}

	glfwTerminate();
	cout << "Window Closed" << endl;
	return 0;
}
