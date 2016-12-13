#ifndef OBJECT_H
#define OBJECT_H

#include <QThread>

#include "Headers.h"
#include "Geometry.h"
#include "ObjLoader.h"
#include "Mesh.h"
#include "VertexBufferObject.h"

class Shader;
class VertexBufferObject;

struct ObjectMetaData 
{
    VertexBufferObject::DATA *data;
    uint *indices;
    int nrData;
    int nrIndices;
    BoundingBox boundingBox;
};

class ObjectThread : public QThread
{
    Q_OBJECT

    public: 
        ObjectThread(const QString &fileName, const vec3 &pos, const vec3 &scale, const vec4 &rot, bool normalize, vector<ObjectMetaData> &meshData, vector<ObjectMetaData> &lineData, vector<ObjectMetaData> &normalData)
            : m_fileName(fileName), m_rotation(rot), m_position(pos), m_scale(scale), m_normalize(normalize), m_meshData(meshData), m_lineData(lineData), m_normalData(normalData) {}
        ~ObjectThread() {};

        void load();
        void run();
        void normalizeGeometry(vector<vector<Vertex>> &vertices, const vec3 &translate, const vec3 &scale, const vec4 &rotate);        
        void buildMeshData(vector<Vertex> &vertices, vector<uint> &indices);
        void buildLinesData(vector<Vertex> &vertices, vector<uint> &indices);
        void buildNormalsData(vector<Vertex> &vertices, vector<uint> &indices);

    private:
        QString m_fileName;
        vec3 m_position;
        vec3 m_rotation;
        vec3 m_scale;
        BoundingBox m_bb;

        vector<ObjectMetaData> &m_meshData;
        vector<ObjectMetaData> &m_lineData;
        vector<ObjectMetaData> &m_normalData;

    public:
        vector<Material> m_materials;
        vector<vector<Vertex>> m_allVertices;
        vector<vector<uint>> m_allIndices;

        bool m_normalize;
};

class Object : QObject
{
    Q_OBJECT

public:
    Object(const QString &fileName, bool normalize = true, bool buildLineVBO = false, bool buildNormalVBO = false, const vec3 &pos = vec3(), const vec3 &scale = vec3(1, 1, 1), const vec4 &rot = vec4(), const vec4 &color = vec4(1, 1, 1, 1));
    ~Object();

    void start();

    void render(const Transform &trans, const mat4 &model = mat4::identitiy(), const Material &material = Material(), bool applyShadow = true);
    void renderDepth(const Transform &trans, const mat4 &model = mat4::identitiy());

    bool m_isSelected;
    vec3 m_position;
    vec4 m_rotation;
    vec3 m_scale;
    vec4 m_color;
    bool m_normalize;
    bool m_lines;
    bool m_normals;

    BoundingBox m_bb;
    void move(int x, int y, const vec3 &dir, const vec3 &up, const vec3 &right, const vec3 &pos);

private:
    void prepareData(const QString &fileName);
    void buildBuffers();
    void buildVBOMesh(vector<Vertex> &vertices, vector<uint> &indices);
    void buildVBOLines(vector<Vertex> &vertices, vector<uint> &indices);
    void buildVBONormals(vector<Vertex> &vertices, vector<uint> &indices);
    void normalizeGeometry(vector<vector<Vertex>> &vertices, const vec3 &translate, const vec3 &scale, const vec4 &rotate);        

    void buildBuffersFromData();
    void buildVBOMeshFromData(ObjectMetaData &meta);
    void buildVBOLinesFromData(ObjectMetaData &meta);
    void buildVBONormalsFromData(ObjectMetaData &meta);

public:
    QString m_fileName;

    vector<VertexBufferObject *> m_vbosTriangles;
    vector<VertexBufferObject *> m_vbosLines;
    vector<VertexBufferObject *> m_vbosNormals;

    int m_nrTriangles;
    int m_nrVertices;    

    vector<Material> m_materials;

    vector<vector<Vertex>> m_allVertices;
    vector<vector<uint>> m_allIndices;

    vector<ObjectMetaData> m_meshData;
    vector<ObjectMetaData> m_lineData;
    vector<ObjectMetaData> m_normalData;

    float selected(Picking &pick, const Transform &trans, int sw, int sh, int mx, int my);

    ObjectThread m_objectThread;

public slots:
    void loadingDone();
};

#endif