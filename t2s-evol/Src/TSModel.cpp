#include "TSModel.h"



TSModel::TSModel(Object *obj)
:m_object(obj)
{

}


TSModel::~TSModel()
{
}

void TSModel::render(const Transform &trans, const Material &material, bool applyShadow)
{
	
	m_object->render(trans, m_initTrans, material, applyShadow);
}
