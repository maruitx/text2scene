#include "RelationModel.h"


PairwiseRelationModel::PairwiseRelationModel(const QString &anchorName, const QString &actName, const QString &conditionName, const QString & relationName /*= "general"*/)
	:m_anchorObjName(anchorName), m_actObjName(actName), m_conditionName(conditionName), m_relationName(relationName)

{
	m_relationKey = m_anchorObjName + "_" + m_actObjName + "_" + m_conditionName + "_" + m_relationName;
}

void PairwiseRelationModel::loadModel()
{

}

OccurrenceModel::OccurrenceModel(const QString &objName, int objNum)
	:m_objName(objName), m_objNum(objNum)
{
	m_occurKey = QString("%1_%2").arg(m_objName).arg(m_objNum);
	m_numInstance = 0;
	m_occurProb = 0;
}

GroupRelationModel::GroupRelationModel(const QString &anchorObjName, const QString &relationName)
	:m_anchorObjName(anchorObjName), m_relationName(relationName)
{
	m_groupKey = m_relationName + "_" + m_anchorObjName;
	m_numInstance = 0;
}

GroupRelationModel::~GroupRelationModel()
{

}

void GroupRelationModel::loadModel()
{

}
