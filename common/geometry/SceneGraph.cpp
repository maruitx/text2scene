#include "SceneGraph.h"
#include "Scene.h"
#include "CModel.h"

SceneGraph::SceneGraph()
{
}

SceneGraph::SceneGraph(CScene *s):
m_scene(s), m_SuppThresh(0.05)
{
	m_nodeNum = m_scene->getModelNum();
	this->Initialize(m_nodeNum);
}

SceneGraph::~SceneGraph()
{
}

int SceneGraph::extractSupportRel()
{
	double dT = m_SuppThresh;
	for (unsigned int i = 0; i < m_nodeNum; i++) {
		CModel *pMI = m_scene->getModel(i);
		for (unsigned int j = i + 1; j < m_nodeNum; j++) {
			CModel *pMJ = m_scene->getModel(j);
			bool roughOBB = false;
			if (pMI->IsSupport(pMJ, roughOBB, dT, m_scene->getUprightVec())) {
				this->InsertEdge(i, j, CT_SUPPORT);	// upright support
			}
		}
	}

	return 0;
}

void SceneGraph::updateGraph(int modelID)
{
	m_nodeNum = m_scene->getModelNum();

	std::vector<int> neighborEdges;
	this->GetAllNeigborEdgeList(modelID, neighborEdges);

	for (int i = 0; i < neighborEdges.size(); i++)
	{
		this->DeleteEdge(neighborEdges[i]);
	}

	updateSupportRel(modelID);
	pruneSupportRel();
}

void SceneGraph::updateGraph(int modelID, int suppModelID)
{
	std::vector<int> neighborEdges;
	this->GetAllNeigborEdgeList(modelID, neighborEdges);

	for (int i = 0; i < neighborEdges.size(); i++)
	{
		this->DeleteEdge(neighborEdges[i]);
	}

	// update support relationship
	CModel *pMJ = m_scene->getModel(modelID);
	CModel *pMI = m_scene->getModel(suppModelID);

	if (pMI->IsSupport(pMJ, false, m_SuppThresh, m_scene->getUprightVec())) {
		this->InsertEdge(suppModelID, modelID, CT_SUPPORT);	// upright support
	}
}

int SceneGraph::updateSupportRel(int modelID)
{
	double dT = m_SuppThresh;
	CModel *pMJ = m_scene->getModel(modelID); 

	for (unsigned int i = 0; i < m_nodeNum; i++) {
		if ( i!=modelID)
		{
			CModel *pMI = m_scene->getModel(i);

			if (pMI->IsSupport(pMJ, false, dT, m_scene->getUprightVec())) {
				this->InsertEdge(i, modelID, CT_SUPPORT);	// upright support
			}
		}
	}

	return 0;
}

void SceneGraph::buildGraph()
{
	extractSupportRel();
	pruneSupportRel();
}

int SceneGraph::readGraph(const QString &filename)
{
	std::ifstream ifs(filename.toStdString());

	if (!ifs.is_open())
	{
		return -1;
	}

	char buf[MAX_STR_BUF_SIZE];
	ifs >> buf;
	while (!ifs.eof()) {
		switch (buf[0]) {
		case 'N':
			this->Clear();
			this->ReadVert(ifs);
			if (this->Size() != m_scene->getModelNum()) {
				Simple_Message_Box("Read SG: Number of models does not match!");
				return -1;
			}
			break;
		case 'E':
			this->ReadEdge(ifs);
			break;
		default:
			// eat up rest of the line
			ifs.getline(buf, MAX_STR_BUF_SIZE, '\n');
			break;
		}
		ifs >> buf;
	}
	return 0;
}

int SceneGraph::saveGraph(const QString &filename)
{
	std::ofstream  ofs(filename.toStdString());

	if (!ofs.is_open())
	{
		Simple_Message_Box(QString("Write SG: cannot open %1").arg(filename));
		return -1;
	}

	//////////////////////////////////////////////////////////////////////////
	this->WriteVert(ofs);
	this->WriteEdge(ofs);
	//////////////////////////////////////////////////////////////////////////
	return 0;
}

void SceneGraph::drawGraph()
{
	if (m_nodeNum == 0)
	{
		return;
	}

	GLfloat red[] = { 1.0f, 0.3f, 0.3f, 1.0f };
	GLfloat blue[] = { 0.1f, 0.1f, 1.0f, 1.0f };
	GLfloat green[] = { 0.4f, 1.0f, 0.6f, 1.0f };
	GLubyte color[4] = { 0 };

	glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_HINT_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);

	// Relations indicated with lines
	glDisable(GL_LIGHTING);
	glEnable(GL_LINE_STIPPLE);
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	for (unsigned int i = 0; i < this->ESize(); i++) {
		const CUDGraph::Edge *e = this->GetEdge(i);
		switch (e->t) {
		case CT_SUPPORT:
			glLineWidth(5.0);
			glColor3f(1.0f, 0.3f, 0.3f); //red
			break;
		case CT_CONTACT:
			glLineWidth(5.0);
			glColor3f(0.8f, 0.5f, 0.1f);
			break;
		case CT_CONTAIN:
			glLineWidth(5.0);
			glColor3f(0.8f, 0.8f, 0.0f);
			break;
		case CT_PROXIMITY:
			glLineWidth(2.0);
			glColor3f(0.8, 0.1, 0.8);  // magenta
			break;
		case CT_SYMMETRY: case CT_PROX_SYM:
			glLineWidth(3.0);
			glColor3f(0.2, 0.8, 0.8);
			break;
		case CT_SUP_SYM: case CT_CONTACT_SYM:
			glLineWidth(3.0);
			glColor3f(0.2, 0.8, 0.2);
			break;
		case CT_WEAK:
			glLineWidth(1.0);
			glColor3f(0.4, 0.4, 0.9);
			break;
		}

		MathLib::Vector3 center[2];
		center[0] = m_scene->getModelOBBCenter(e->v1);
		center[1] = m_scene->getModelOBBCenter(e->v2);

		glBegin(GL_LINES);
		glVertex3dv(center[0].v);
		glVertex3dv(center[1].v);
		glEnd();
	}

	glPopAttrib();
}

void SceneGraph::computeOnTopList()
{
	// build on top list from computed support info
	m_onTopList.clear();
	m_onTopList.resize(m_nodeNum, -1);

	std::vector<std::vector<int>> SuppGiverList(this->Size());	// support giver list, models that are being supported
	for (unsigned int ei = 0; ei < this->ESize(); ei++) {
		if (this->GetEdge(ei)->t == SceneGraph::CT_SUPPORT) {
			CModel *pM1 = m_scene->getModel(this->GetEdge(ei)->v1);
			CModel *pM2 = m_scene->getModel(this->GetEdge(ei)->v2);

			//if pM1 is higher than pM2, then pM2 is in support list of pM1
			if (pM1->getOBB().BottomHeightDiff(pM2->getOBB(), m_scene->getUprightVec()) > 0.0) {
				SuppGiverList[this->GetEdge(ei)->v1].push_back(this->GetEdge(ei)->v2);
			}
			else {
				SuppGiverList[this->GetEdge(ei)->v2].push_back(this->GetEdge(ei)->v1);
			}
		}
	}

	for (unsigned int i = 0; i < SuppGiverList.size(); i++)
	{
		for (int j = 0; j < SuppGiverList[i].size(); j++)
		{
			m_onTopList[i] = SuppGiverList[i][j];
		}
	}
}

int SceneGraph::pruneSupportRel()
{
	// collect direct support info.
	std::vector<std::vector<int>> SuppList(this->Size());	// support giver list, models that are being supported
	for (unsigned int ei = 0; ei < this->ESize(); ei++) {
		if (this->GetEdge(ei)->t == CT_SUPPORT) {
			CModel *pM1 = m_scene->getModel(this->GetEdge(ei)->v1);
			CModel *pM2 = m_scene->getModel(this->GetEdge(ei)->v2);

			//if pM1 is higher than pM2, then pM2 is in support list of pM1
			if (pM1->getOBB().BottomHeightDiff(pM2->getOBB(), m_scene->getUprightVec()) > 0.0) {
				SuppList[this->GetEdge(ei)->v1].push_back(this->GetEdge(ei)->v2);
			}
			else {
				SuppList[this->GetEdge(ei)->v2].push_back(this->GetEdge(ei)->v1);
			}
		}
	}

	// check nodes with more than more supporters (if multiple nodes support this node)
	// filter those who doesn't contact
	for (unsigned int i = 0; i < SuppList.size(); i++)
	{
		if (SuppList[i].size() > 1)
		{
			CModel *pM1 = m_scene->getModel(i);
			for (int j = 0; j < SuppList[i].size(); j++)
			{
				CModel *pM2 =  m_scene->getModel(SuppList[i][j]);

				// use a relative large support threshold for conservative pruning
				if (!pM1->IsSupport(pM2, true, 10 * m_SuppThresh, m_scene->getUprightVec()))
				{
					this->DeleteEdge(i, SuppList[i][j]);
				}
				else
				{
					//m_onTopList[i] = SuppList[i][j];
				}
			}
		}
	}

	return 0;
}

