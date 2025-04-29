#pragma once
#include <vector>
#include <glm/glm.hpp>
using namespace std;
using namespace glm;

class Geo
{
private:
	Geo* parent;
	vector<Geo*> children;
	vec3 pos = vec3(0, 0, 0);
	vec3 color = vec3(1, 1, 1);
	float velocity = 0;
	float angle = 0;
	int n = 0;
	mat4 globalM = mat4(1);
	mat4 localM = mat4(1);
	mat4 localG = mat4(1);
public:
	Geo(Geo* parent=NULL)
	{
		this->parent = parent;
		if(parent!=NULL)
			parent->children.push_back(this);
	}
	void SetColor(vec3 color)
	{
		this->color = color;
	}
	vec3 GetColor()
	{
		return color;
	}
	void SetLocalM(mat4 LocalM)
	{
		this->localM = LocalM;
	}
	mat4 GetLocalM()
	{
		return localM;
	}
	void SetLocalG(mat4 LocalG)
	{
		this->localG = LocalG;
	}
	mat4 GetLocalG()
	{
		return localG;
	}
	void SetGlobalM()
	{
		if (parent != NULL)
			globalM = parent->GetGlobalM() * GetLocalM();
		else
			globalM = GetLocalM();
		for (int i = 0; i < children.size(); i++)
		{
			children[i]->SetGlobalM();
		}
	}
	mat4 GetGlobalM()
	{
		return globalM;
	}
	mat4 GetModelMatrix()
	{
		return globalM * localG;
	}
	vec3 GetPos()
	{
		return pos;
	}
};