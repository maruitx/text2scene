#include "TSModel.h"



TSModel::TSModel(Object *obj)
: m_object(obj), 
  m_initTrans(mat4::identitiy())
{

}


TSModel::~TSModel()
{
}

void TSModel::render(const Transform &trans, const Material &material, bool applyShadow)
{
	
	//m_object->render(trans, m_initTrans, material, applyShadow);
	//m_object->render(trans, mat4::identitiy(), material, applyShadow);
}
