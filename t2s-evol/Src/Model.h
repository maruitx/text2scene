#ifndef MODEL_H
#define MODEL_H

#include <QThread>

#include "Headers.h"
#include "Geometry.h"
#include "ObjLoader.h"
#include "Mesh.h"
#include "VertexBufferObject.h"

#include "box_bvh.h"

class MeshBvh;
class TriangleMesh;

class Shader;
class VertexBufferObject;

class ModelMesh
{
public:
	ModelMesh(vector<VertexBufferObject::DATA> &vertices, vector<uint> &indices, const Material &material);
	~ModelMesh();

	void buildVBO();
	void computeNormals();
	
	void render(const Transform &trans, Shader *shader, const string &textureDir = "");
	void renderDepth(const Transform &trans, const mat4 &model = mat4::identitiy());

	vector<VertexBufferObject::DATA> m_vertices;
	vector<uint> m_indices;

private:
	VertexBufferObject *m_vbo;
	Material m_material;
};

class ModelThread : public QThread
{
	Q_OBJECT

public: 
	ModelThread(const string &fileName, vector<ModelMesh> &meshes, BoundingBox &bb);
	void load(const string &fileName);
	void buildBVH();

	void run();

	MeshBvh *m_meshBvh;
	TriangleMesh *m_triMesh;

private:
	string m_fileName;
	vector<ModelMesh> &m_meshes;
	BoundingBox &m_bb;
};

class Model : public QObject
{
	Q_OBJECT

public:
	Model(const string &fileName);
	void render(const Transform &trans, const mat4 &initTrans, bool applyShadow = false, const string &textureDir = "", int renderMode = 0, int isSelected = 0);
	void renderDepth(const Transform &trans, const mat4 &initTrans);
    void buildBBVBO();
	bool checkCollisionBBTriangles(const BoundingBox &testBB, const mat4 &testModelTransMat, const mat4 &refModelTransMat);  // transMat is the transformation to place the model in current scene
	bool checkCollisionTrianglesTriangles(Model *testModel, const mat4 &testModelTransMat, const mat4 &refModelTransMat);

	bool isTriDegenerate(const vec3 &v1, const vec3 &v2, const vec3 &v3);

	vector<ModelMesh>& getModelMeshs() { return m_meshes; };

	void getBvhLeafNodes(std::vector<BvhLeafNode> &nodes);
	void getTriangle(int index, vec3 &p, vec3 &q, vec3 &r);

    //vec3 m_collisionTrans;
	vec3 m_sceneCenter;
    BoundingBox m_bb; // non-transformed model BB

	bool m_loadingDone;

public slots:
	void loadingDone();	

private:
	ModelThread m_thread;	

	vector<ModelMesh> m_meshes;	
    VertexBufferObject *m_vboBB;

	MeshBvh *m_meshBvh;
	TriangleMesh *m_triMesh;
};

#endif