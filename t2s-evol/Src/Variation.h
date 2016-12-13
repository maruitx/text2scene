#ifndef VARIATION_H
#define VARIATION_H

#include "Headers.h"
#include "Mesh.h"

class Object;

struct MetaData
{
    string fileName;
    mat4 trans;
    Material mat;
    string id;
    bool visible;
    vec3 pos;
};

class Variation
{
public:
    Variation(unordered_map<string, Object*> &objects);
    ~Variation();

    void render(const Transform &trans, bool applyShadow = true);
    void renderDepth(const Transform &trans);

    void makeExplode(float s);
    void makeBright(float s);
    void makeScale(float s);

private:
    vector<MetaData> m_metaData;
    unordered_map<string, Object*> &m_objects;
    QString m_modelRepository;
};

#endif

 