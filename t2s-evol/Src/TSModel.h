#pragma once
#include "Object.h"

class TSModel 
{
public:
	TSModel(Object *obj);
	~TSModel();

	void setInitTrans(const mat4 &trans) { m_initTrans = trans; }

	void render(const Transform &trans, const Material &material, bool applyShadow);

public:

private:
	Object *m_object;
	mat4 m_initTrans;

};

