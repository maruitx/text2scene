#include "Variation.h"
#include "Object.h"

Variation::Variation(unordered_map<string, Object*> &objects)
: m_objects(objects), 
  m_modelRepository("Data/Objs/")
  //m_modelRepository("L:/data/models-OBJ/models/")
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
            md.fileName = md.id + ".obj";
        }

        if(r == 1)
        {
            md.id = "cube2";
			md.fileName = md.id + ".obj";
        }

        if(r == 2)
        {
            md.id = "bunny_simple";
			md.fileName = md.id + ".obj";
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
            loadObject(md);
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
            loadObject(md);
        }
    }
}

void Variation::loadObject(const MetaData &md)
{
    Object *obj = new Object(m_modelRepository + QString(md.fileName.c_str()), true, true, true, vec3(), vec3(1.0f), vec4(1, 0, 0, 0));
    m_objects.insert(make_pair(md.id, obj));
    obj->start();
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
