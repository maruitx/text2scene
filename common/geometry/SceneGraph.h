#pragma once
#include "UDGraph.h"
#include "../utilities/utility.h"

class CScene;

class SceneGraph : public CUDGraph
{
public:
	typedef enum {
		CT_SUPPORT = 0,
		CT_CONTACT,
		CT_CONTAIN,
		CT_PROXIMITY,
		CT_SYMMETRY,
		CT_WEAK,
		CT_SUP_SYM,
		CT_CONTACT_SYM,
		CT_PROX_SYM,
		CT_WEAK_SYM
	} ConnType;

	SceneGraph();
	SceneGraph(CScene *s);
	~SceneGraph();

	void buildGraph();
	void updateGraph(int modelID); // update graph after add a new model
	void updateGraph(int modelID, int suppModelID);  // update graph with known support model

	void computeOnTopList();
	std::vector<int>& getOnTopList() { return m_onTopList; };

	void drawGraph();

	int readGraph(const QString &filename);
	int saveGraph(const QString &filename);

	double getSuppTh() { return m_SuppThresh; };


private:
	int extractSupportRel(); //
	int pruneSupportRel();
	int updateSupportRel(int modelID); // update support relationship after insert a new model into the scene

private:
	CScene *m_scene;
	int m_nodeNum;

	std::vector<int>				m_onTopList;			// on top list
	//CDistMat<double>					m_modelDistMat;			// model center distance matrix
	//CDistMat<double>					m_ConnStrenMat;			// connection strength matrix

	//std::vector<std::vector<int>>	m_SymGrp;
	//std::vector<int>				m_SymMap;

	double m_SuppThresh;
};

