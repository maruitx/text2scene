#pragma once

#include "AABB.h"
#include "OBB.h"
#include "qgl.h"
#include "Eigen/Dense"
#include "../utilities/mathlib.h"

class CMesh;
//class SuppPlaneManager;
//class SuppPlane;


class CModel
{
public:
	CModel();
	~CModel();

	bool loadModel(QString filename, double metric = 1.0, int loadForRendering = 0, QString sceneDbType = QString());
	void saveModel(QString filename);
	QString getModelFileName() { return m_fileName; };
	QString getModelFilePath() { return m_filePath; };
	QString getNameStr() { return m_nameStr; };

	//void loadAnnoFile();
	//void saveAnnoFile();
	//std::vector<int> computeClosestFaceIds(const std::vector<int> &obbFaceIds);
	//std::vector<int> computeCloseOBBIdsByTriIds(const std::vector<int> &triFaceIds);
	//std::vector<int> getAnnoTriIds() { return m_annoTriIds; };
	MathLib::Vector3 getFaceCenter(int fid);
	MathLib::Vector3 getFaceNormal(int fid);

	void setID(int i) { m_id = i; };
	int getID() { return m_id; };

	void setJobName(const QString &s) { m_jobName = s; }
	QString getJobName() { return m_jobName; };

	void setCatName(const QString &l) { m_catName = l; };
	QString getCatName() { return m_catName; };

	CMesh* getMesh() { return m_mesh; };

	// aabb
	void computeAABB();
	CAABB getAABB() { return m_AABB; };
	std::vector<double> getAABBXYRange();
	MathLib::Vector3 getAABBCenter() { return  m_AABB.C(); };
	double getModelAABBSize() { return m_AABB.Vol(); };
	MathLib::Vector3 getModelPosAABB() { return m_AABB.GetBottomCenter(); }; // center of bottom plane

	MathLib::Vector3 getMinVert() { return m_AABB.GetMinV(); };
	MathLib::Vector3 getMaxVert() { return m_AABB.GetMaxV(); };

	// obb
	void computeOBB(int fixAxis = -1);
	int loadOBB(const QString &sPathName = QString());
	int saveOBB(const QString &sPathName = QString());
	bool IsSupport(CModel *pOther, bool roughOBB, double dDistT, const MathLib::Vector3 &Upright);
	double getOBBBottomHeight();
	double getOBBHeight();
	MathLib::Vector3 getOBBCenter() { return m_OBB.C(); };
	COBB& getOBB() { return m_OBB; };
	std::vector<MathLib::Vector3> getOBBAxis() { return m_OBB.A(); };
	MathLib::Vector3 getModelPosOBB();
	MathLib::Vector3 getModelTopCenter();
	MathLib::Vector3 getModelRightCenter();
	MathLib::Vector3 getModeLeftCenter();
	MathLib::Vector3 getModelAlongDirOBBFaceCenter();
	MathLib::Vector3 getModelNormalDirOBBFaceCenter();
	MathLib::Vector3 getAlongDirOBBAxis();

	void selectOBBFace(const MathLib::Vector3 &origin, const MathLib::Vector3 &dir);
	MathLib::Vector3 getFrontNormal();
	MathLib::Vector3 getOBBFrontFaceCenter();

	double getOBBDiagLength();  // for compute the scaling of model

	// action
	void setStatus(int i);
	int getStatus() { return m_status; };

	// collision
	//void prepareForIntersect(ozcollide::AABBTreePolyBuilder *builder);
	void prepareForIntersect();
	void updateForIntersect();
	bool isSegIntersectMesh(const MathLib::Vector3 &startPt, const MathLib::Vector3 &endPt, double radius = 0, MathLib::Vector3 &intersectPoint = MathLib::Vector3(0,0,0));
	bool isOBBIntersectMesh(const COBB &testOBB);

	//// support plane
	//void initSuppPlaneManager();
	//void buildSuppPlane();
	//bool hasSuppPlane();
	//SuppPlane* getLargestSuppPlane();
	//double getLargestSuppPlaneHeight();
	//SuppPlane* getSuppPlane(int i);
	//int getSuppPlaneNum();

	// transformation
	void transformModel(const MathLib::Matrix4d &transMat);
	void transformModel(double tarOBBDiagLen, const MathLib::Vector3 &tarOBBPos, const MathLib::Vector3 &tarFrontDir);
	void computeTransMat(double tarOBBDiagLen, const MathLib::Vector3 &tarOBBPos, const MathLib::Vector3 &tarFrontDir);
	MathLib::Matrix4d& getLastTransMat() { return m_lastTransMat; };
	MathLib::Matrix4d& getFullTransMat() { return m_fullTransMat; };

	MathLib::Matrix4d& getInitTransMat() { return m_initTransMat; };
	void setInitTransMat(const MathLib::Matrix4d &transMat) { m_initTransMat = transMat; };
	void revertLastTransform();

	// rendering options
	void buildDisplayList(int showDiffColor = 1, int showFaceCluster = 0);
	void draw(bool showModel = true, bool showOBB = false, bool showSuppPlane = false, bool showFrontDir =false, bool showSuppChildOnly=false);
	void drawFrontDir();


	// support relationships
	bool isSupportChild(int id);
	int suppParentID;
	int parentSuppPlaneID;   // on which support plane of the parent
	std::vector<int> suppChindrenList;
	int supportLevel;

	std::vector<std::vector<int>> suppGridPos; // grid pos that is filled by current model

	void setVisible(bool v) { m_isVisible = v; };
	bool isVisible() { return m_isVisible; };

	void setBusy(bool b) { m_isBusy = b; };
	bool isBusy(){return m_isBusy; };

private:
	QString m_fileName;
	QString m_filePath;
	QString m_nameStr;   // name string with out the cat at head
	QString m_catName;

	int m_id;
	double m_metric;
	// m_sceneUpRightVec;

	//std::vector<int> m_annoOBBFaceIds;
	//std::vector<int> m_annoTriIds;

	double m_initOBBDiagLen;
	MathLib::Vector3 m_initOBBPos;
	MathLib::Vector3 m_initFrontDir;
	MathLib::Matrix4d m_lastTransMat;
	MathLib::Matrix4d m_fullTransMat;

	MathLib::Matrix4d m_initTransMat;  // transformation matrix for objs in initial scene

	CMesh *m_mesh;
	CAABB m_AABB;
	COBB m_OBB;
	bool m_hasOBB;

	GLuint m_displayListID;
	int m_status;  // whether model is fixed or movable

	//SuppPlaneManager *m_suppPlaneManager;
	std::vector<int> m_faceIndicators;

	bool m_readyForInterTest;

	// rendering options
	bool m_showDiffColor;
	//bool m_showFaceClusters;

	QString m_jobName;
	int m_outputStatus;

	bool m_isVisible;

	bool m_isBusy;
};