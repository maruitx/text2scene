#include "Variation.h"
#include "Object.h"

Variation::Variation(unordered_map<string, Object*> &objects)
: m_objects(objects), 
  m_modelRepository("Data/Objs/")
{
    int s = 5;
    float step = 1.0f;

    float t = -s/2.0f + step/2.0f;
    float x = t, y = -step, z = t;    

    for(int i=0; i<125; ++i)
    {        
        if(i % (s*s) == 0)
        {
            x = t;
            z = t;

            y += step;
        }
        else if (i % s == 0)
        {
            x = t;
            z += step;
        }
        else
        {
            x += step;
        }

        int r = rand() % 3;

        MetaData md;

        
        md.mat.initRandom();
        md.trans = mat4::translate(x, y, z);
        md.pos = vec3(x, y, z);

        if(r == 0)
        {
            md.id = "cube1";
            md.fileName = "cube1.obj";
        }

        if(r == 1)
        {
            md.id = "cube2";
            md.fileName = "cube2.obj";
        }

        if(r == 2)
        {
            md.id = "bunny_simple";
            md.fileName = "bunny_simple.obj";
        }

        m_metaData.push_back(md);
    }
}

Variation::~Variation()
{
}

void Variation::render(const Transform &trans, bool applyShadow)
{
    for(int i=0; i<m_metaData.size(); ++i)
    {
        MetaData &md = m_metaData[i];

        auto &iter = m_objects.find(md.id);

        if(iter != m_objects.end())
        {
            iter->second->render(trans, md.trans, md.mat, applyShadow);
        }
        else
        {
            Object *obj = new Object(m_modelRepository + QString(md.fileName.c_str()), true, true, true, vec3(), vec3(1.0f));
            m_objects.insert(make_pair(md.id, obj));
            obj->start();
        }
    }
}

void Variation::renderDepth(const Transform &trans)
{
    for (int i = 0; i<m_metaData.size(); ++i)
    {
        MetaData &md = m_metaData[i];

        auto &iter = m_objects.find(md.id);

        if (iter != m_objects.end())
        {
            iter->second->renderDepth(trans, md.trans);
        }
        else
        {
            Object *obj = new Object(m_modelRepository + QString(md.fileName.c_str()), true, true, true, vec3(), vec3(1.0f));
            m_objects.insert(make_pair(md.id, obj));
            obj->start();
        }
    }
}

void Variation::makeExplode(float s)
{
    for(int i=0; i<m_metaData.size(); ++i)
    {
        MetaData &md = m_metaData[i];
        vec3 p = md.pos  * 2;
        md.trans = mat4::translate(p);        
    }
}

void Variation::makeBright(float s)
{
    for(int i=0; i<m_metaData.size(); ++i)
    {
        MetaData &md = m_metaData[i];
        md.mat.Kd *= s;
    }
}

void Variation::makeScale(float s)
{
    for(int i=0; i<m_metaData.size(); ++i)
    {
        MetaData &md = m_metaData[i];
        vec3 p = md.pos;
        md.trans = mat4::translate(p) * mat4::scale(s, s, s);
    }
}
