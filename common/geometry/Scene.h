#pragma once

#include "qglviewer/qglviewer.h"
#include "StarlabDrawArea.h"
#include "Model.h"  // starlab Model class

#include "CModel.h"

class SceneGraph;

class CScene
{
public:

	CScene();
	~CScene();

	void loadSceneFile(const QString filename, int obbOnly, int loadForRendering);

	void computeAABB();
	void updateSeneAABB(CAABB addedBox) { m_AABB.Merge(addedBox); };
	MathLib::Vector3 getMinVert() { return m_AABB.GetMinV(); };
	MathLib::Vector3 getMaxVert() { return m_AABB.GetMaxV(); };

	//void insertModel(QString modelFileName);
	//void insertModel(CModel *m);

	void setSceneName(const QString &sceneName) { m_sceneFileName = sceneName; };
	QString getSceneName() { return m_sceneFileName; };
	QString getFilePath() { return m_sceneFilePath; };
	QString getSceneDbPath() { return m_sceneDBPath; };
	QString getModelDBPath() { return m_modelDBPath; };
	QString getSceneFormat() { return m_sceneFormat; };

	void setSceneDrawArea(Starlab::DrawArea *a) { m_drawArea = a; };
	MathLib::Vector3 getUprightVec() { return m_uprightVec; };

	CModel* getModel(int id) { return m_modelList[id]; };
	QString getModelCatName(int modelID) { return m_modelList[modelID]->getCatName(); };
	int getModelIdByName(const QString &s) { return m_modelNameIdMap[s] - 1; }; 
	std::vector<int> getModelIdWithCatName(QString s, bool usingSynset = true);
	MathLib::Vector3 getModelAABBCenter(int m_id) { return m_modelList[m_id]->getAABBCenter(); };
	MathLib::Vector3 getModelOBBCenter(int m_id) { return m_modelList[m_id]->getOBBCenter(); };

	QVector<QString> getModelNameList();
	int getModelNum() { return m_modelList.size(); };

	int getRoomID() { return m_roomID; };

	// support plane
	//void buildModelSuppPlane();
	double getFloorHeight();

	void updateSceneGraph(int modelID);
	void buildSupportHierarchy();
	void setSupportChildrenLevel(CModel *m);
	//int findPlaneSuppPlaneID(int childModelID, int parentModelID);

	void updateSceneGraph(int modelID, int suppModelID, int suppPlaneID);
	//void updateSupportHierarchy(int modelID, int suppModelID, int suppPlaneID);

	// collision
	void prepareForIntersect();
	bool isSegIntersectModel(MathLib::Vector3 &startPt, MathLib::Vector3 &endPt, int modelID, double radius = 0);


	// rendering
	void draw();
	void buildModelDislayList(int showDiffColor = 1, int showFaceCluster = 0);
	void setShowModelOBB(bool s) { m_showModelOBB = s; };
	//void setShowSuppPlane(bool s) { m_showSuppPlane = s; };
	void setShowSceneGraph(bool s) { m_showSceneGaph = s; };
	void setShowModelFrontDir(bool s) { m_showModelFrontDir = s; };
	//void setShowSuppChildOBB(bool s) { m_showSuppChildOBB = s; }

	void updateDrawArea() { m_drawArea->updateGL(); };

private:
	Starlab::DrawArea *m_drawArea;

	int m_modelNum;
	QVector<CModel *> m_modelList;
	QMap<QString, int> m_modelNameIdMap;  // mapped id is 1 larger than model id since no instance found will get id = 0
	std::vector<QString> m_modelCatNameList;

	CAABB m_AABB;
	double m_metric;  // metric to convert from loaded coordinates to real world metric

	MathLib::Vector3 m_uprightVec;
	//SuppPlane *m_floor;
	double m_floorHeight;

	SceneGraph *m_sceneGraph;

	// File info
	QString m_sceneFileName;
	QString m_sceneFilePath;
	QString m_sceneDBPath;
	QString m_modelDBPath;
	QString m_sceneFormat;

	int m_roomID;

	// rendering options
	bool m_showSuppPlane;
	bool m_showModelOBB;
	bool m_showSceneGaph;
	bool m_showModelFrontDir;
	bool m_showSuppChildOBB;
};