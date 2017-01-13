#ifndef MODEL_H
#define MODEL_H

#include <QThread>

#include "Headers.h"
#include "Geometry.h"
#include "ObjLoader.h"
#include "Mesh.h"
#include "VertexBufferObject.h"

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
	void run();

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
	void render(const Transform &trans, const mat4 &initTrans, bool applyShadow = false, const string &textureDir = "");
	void renderDepth(const Transform &trans, const mat4 &initTrans);
    void buildBBVBO();
	bool checkCollisionBBTriangles(const BoundingBox &bb, const mat4 &transMat, double delta = 0);  // transMat is the transformation to place the model in current scene

    //vec3 m_collisionTrans;
	vec3 m_sceneCenter;
    BoundingBox m_bb;

	bool m_loadingDone;

public slots:
	void loadingDone();	

private:
	ModelThread m_thread;	

	vector<ModelMesh> m_meshes;	
    VertexBufferObject *m_vboBB;
};

#endif