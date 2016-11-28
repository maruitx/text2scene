#pragma once

//#include "SurfaceMeshHelper.h"
#include "../utilities/mathlib.h"
#include "OBB.h"

#include <Opcode.h>

// simple triangle mesh structure

class CMesh
{

public:

	CMesh(QString path, QString name); 
	~CMesh();

	bool readObjFile(const std::string &filename, const double metric = 1.0, QString sceneDbType = QString());
	void saveObjFile(const std::string &filename);

	void draw(QColor c);
	void draw(const std::vector<int> &faceIndicators);

	void computeFaceNormal();
	void computeMinMaxVerts();
	MathLib::Vector3 getMinVert() { return m_minVert; };
	MathLib::Vector3 getMaxVert() { return m_maxVert; };

	std::vector<MathLib::Vector3>& getfaceNormals() { return m_faceNormals; };
	std::vector<MathLib::Vector3>& getVertices() { return m_vertices; };
	std::vector<std::vector<int>>& getFaces() { return m_faces; };
	MathLib::Vector3 getFaceCenter(int fid);
	MathLib::Vector3 getFaceNormal(int fid){ return m_faceNormals[fid]; };

	int getVertsNum() { return m_vertices.size(); };
	std::vector<MathLib::Vector3> getVerts(const std::vector<int> &ids);


	void transformMesh(const MathLib::Matrix4d &transMat);
	MathLib::Vector3 transformVert(const MathLib::Vector3 &vert, const MathLib::Matrix4d &transMat);

	// collision
	bool isSegIntersect(const MathLib::Vector3 &startPt, const MathLib::Vector3 &endPt, const double radius = 0, MathLib::Vector3 &intersectPoint = MathLib::Vector3(0, 0, 0));
	bool isOBBIntersect(const COBB &testOBB);

	void buildOpcodeModel();
	void updateOpcodeModel();


private:
	QString m_name;

	std::vector<MathLib::Vector3> m_vertices;
	std::vector<std::vector<int>> m_faces;
	std::vector<MathLib::Vector3> m_faceNormals;

	MathLib::Vector3 m_minVert;
	MathLib::Vector3 m_maxVert;

	//
	Opcode::Model *m_OpcodeModel;
	Opcode::MeshInterface *m_meshIterface;
	unsigned int *m_opFaces;
};