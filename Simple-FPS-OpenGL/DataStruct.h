#pragma once
#include <vector>
#include <glm/glm.hpp>
using namespace std;
using namespace glm;


//데이터 구조체
struct Cam
{
	bool shoot = false;
	bool grab = false;
	bool noclip = false;
	int grabbedObject = 0;
	int cam = 0;
	int height;
	int width;
	int gravityTimer = 0;
	int tall = 3;
	float distance;
	float fov = 45;
	float aspect = (float)width / (float)height;
	float near = 0.1;
	float far = 1000;
	float angle1 = 0;
	float angle2 = 0;
	vec3 viewVector = vec3(0, 0, 0);
	vec3 armPos = vec3(0, 0, 0);
	vec3 up = vec3(0, 1, 0);
	vec3 pos = vec3(0, 0, 0);
	vec3 target = vec3(0, 0, 0);
	vec3 inertia = vec3(0, 0, 0);
};

struct ShaderLoc
{
	GLuint MmatLoc;
	GLuint AmatLoc;
	GLuint camVecLoc;
	GLuint vertexColorLoc;
	GLuint ambientStrengthLoc;
	GLuint specularStrengthLoc;
	GLuint shininessLoc;
	GLuint diffuseStrengthLoc;
	GLuint lightColorLoc;
	GLuint lightPositionLoc;
};