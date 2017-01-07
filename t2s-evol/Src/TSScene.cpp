#include "TSScene.h"
#include "Utility.h"
#include "Model.h"


TSScene::TSScene(unordered_map<string, Model*> &models)
	:m_models(models),
	m_sceneBB(vec3(math_maxfloat), vec3(math_minfloat)),
	m_frameCount(0),
	m_loadedModelNum(0),
	m_ssg(NULL), 
    m_camTrans(0.0f, 0.0f, 0.0f)
{
	
}

TSScene::TSScene(unordered_map<string, Model*> &models, const QString &fileName)
: m_models(models),
  m_sceneBB(vec3(math_maxfloat), vec3(math_minfloat)), 
  m_frameCount(0),
  m_loadedModelNum(0),
  m_ssg(NULL), 
  m_camTrans(0.0f, 0.0f, 0.0f)
{
	loadSceneFile(fileName);
	countLoadedModelNum();
}

TSScene::TSScene(unordered_map<string, Model*> &models, MetaScene &ms)
	:m_models(models),
	m_metaScene(ms),
	m_sceneBB(vec3(math_maxfloat), vec3(math_minfloat)),
	m_frameCount(0),
	m_loadedModelNum(0),
	m_ssg(NULL), 
    m_camTrans(0.0f, 0.0f, 0.0f)
{
	m_modelNum = m_metaScene.m_metaModellList.size();
	countLoadedModelNum();
}

TSScene::~TSScene()
{
}

void TSScene::loadSceneFile(const QString filename, int obbOnly /*= false*/)
{
	cout << "Loading Scene File ... ";
	QFile inFile(filename);
	QTextStream ifs(&inFile);

	if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) return;

	QFileInfo sceneFileInfo(inFile.fileName());
	m_metaScene.m_sceneFileName = sceneFileInfo.baseName();   // scene_01.txt
	m_metaScene.m_sceneFilePath = sceneFileInfo.absolutePath();

	int cutPos = sceneFileInfo.absolutePath().lastIndexOf("/");
	m_metaScene.m_sceneDBPath = sceneFileInfo.absolutePath().left(cutPos);

	QString databaseType;
	QString modelFileName;

	ifs >> databaseType;

	m_metaScene.m_sceneFormat = databaseType;

	if (databaseType == QString("StanfordSceneDatabase"))
	{
		int currModelID = -1;
		m_metaScene.m_modelRepository = m_metaScene.m_sceneDBPath + "/models";

		while (!ifs.atEnd())
		{
			QString currLine = ifs.readLine();
			if (currLine.contains("modelCount "))
			{
				m_modelNum = StringToIntegerList(currLine.toStdString(), "modelCount ")[0];
				m_metaScene.m_metaModellList.resize(m_modelNum);
			}

			if (currLine.contains("newModel "))
			{
				currModelID++;

				std::vector<std::string> parts = PartitionString(currLine.toStdString(), " ");				
				int modelIndex = StringToInt(parts[1]);

				m_metaScene.m_metaModellList[currModelID].id = modelIndex;
				m_metaScene.m_metaModellList[currModelID].name = parts[2];
				m_metaScene.m_metaModellList[currModelID].path = m_metaScene.m_modelRepository.toStdString() + "/" + parts[2] + ".obj";
			}

			if (currLine.contains("transform "))
			{
				std::vector<float> transformVec = StringToFloatList(currLine.toStdString(), "transform ");  // transformation vector in stanford scene file is column-wise
				mat4 transMat(transformVec.data());
				transMat = transMat.transpose();
				m_metaScene.m_metaModellList[currModelID].transformation = transMat;
			}
		}
	}

	cout << "done." << endl;
}

void TSScene::render(const Transform &trans, bool applyShadow)
{
    Transform tt = trans;
    tt.view *= mat4::translate(m_camTrans);
    tt.lightViews[0] *= mat4::translate(m_camTrans);

	int nrLoaded = 0;

	for (int i = 0; i < m_metaScene.m_metaModellList.size(); i++)
	{
		MetaModel &md = m_metaScene.m_metaModellList[i];
		auto &iter = m_models.find(md.name);

		if (iter != m_models.end())
		{
			iter->second->render(tt, md.transformation, applyShadow);
		}
		else if (md.path.size() > 0)
		{
			if (nrLoaded == 0 && m_frameCount % 20 == 0)
			{
				Model *model = new Model(md.path.c_str());
				m_models.insert(make_pair(md.name, model));
			}

			nrLoaded++;

			countLoadedModelNum();
		}
	}

    //renderSceneBB(trans);

	m_frameCount++;
}

void TSScene::renderSceneBB(const Transform &trans)
{
    Transform tt = trans;
    tt.view *= mat4::translate(m_camTrans);

    vec3 mi = m_sceneBB.mi();
    vec3 ma = m_sceneBB.ma();

    vec3 a = vec3(mi.x, mi.y, mi.z);
    vec3 b = vec3(ma.x, mi.y, mi.z);
    vec3 c = vec3(ma.x, mi.y, ma.z);
    vec3 d = vec3(mi.x, mi.y, ma.z);

    vec3 e = vec3(mi.x, ma.y, mi.z);
    vec3 f = vec3(ma.x, ma.y, mi.z);
    vec3 g = vec3(ma.x, ma.y, ma.z);
    vec3 h = vec3(mi.x, ma.y, ma.z);

    glEnableFixedFunction(tt);

        glBegin(GL_LINES);
        glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
            glVertex3f(a.x, a.y, a.z);
            glVertex3f(b.x, b.y, b.z);
            glVertex3f(b.x, b.y, b.z);
            glVertex3f(c.x, c.y, c.z);
            glVertex3f(c.x, c.y, c.z);
            glVertex3f(d.x, d.y, d.z);
            glVertex3f(d.x, d.y, d.z);
            glVertex3f(a.x, a.y, a.z);

            glVertex3f(e.x, e.y, e.z);
            glVertex3f(f.x, f.y, f.z);
            glVertex3f(f.x, f.y, f.z);
            glVertex3f(g.x, g.y, g.z);
            glVertex3f(g.x, g.y, g.z);
            glVertex3f(h.x, h.y, h.z);
            glVertex3f(h.x, h.y, h.z);
            glVertex3f(e.x, e.y, e.z);

            glVertex3f(a.x, a.y, a.z);
            glVertex3f(e.x, e.y, e.z);
            glVertex3f(b.x, b.y, b.z);
            glVertex3f(f.x, f.y, f.z);
            glVertex3f(c.x, c.y, c.z);
            glVertex3f(g.x, g.y, g.z);
            glVertex3f(d.x, d.y, d.z);
            glVertex3f(h.x, h.y, h.z);

        glEnd();

    glDisableFixedFunction();
}

void TSScene::renderDepth(const Transform &trans)
{
    Transform tt = trans;
    tt.view *= mat4::translate(m_camTrans);

	int nrLoaded = 0;

	for (int i = 0; i < m_metaScene.m_metaModellList.size(); i++)
	{
		MetaModel &md = m_metaScene.m_metaModellList[i];
		auto &iter = m_models.find(md.name);

		if (iter != m_models.end())
		{
			iter->second->renderDepth(tt, md.transformation);
		}
	}
}

void TSScene::makeRandom()
{
	for (int i = 1; i < m_metaScene.m_metaModellList.size(); i++)
	{
		MetaModel &md = m_metaScene.m_metaModellList[i];
		
		md.transformation.a14 += rand<float>(-10, 10);
		md.transformation.a24 += rand<float>(-10, 10);
		md.transformation.a34 += rand<float>(-10, 10);
	}
}

void TSScene::countLoadedModelNum()
{
	m_loadedModelNum = 0;

	for (int i = 0; i < m_metaScene.m_metaModellList.size(); i++)
	{
		MetaModel &md = m_metaScene.m_metaModellList[i];
		auto &iter = m_models.find(md.name);

		if (iter != m_models.end())
		{
			m_loadedModelNum++;
		}
	}

	if (m_loadedModelNum == m_metaScene.m_metaModellList.size())
	{
		cout << "\nFinish loading models for "<<m_metaScene.m_sceneFileName.toStdString()<<"\n";
	}
}

void TSScene::computeSceneBB()
{
    if(m_models.size() == 0)
        return;

    vec3 smi = vec3(math_maxfloat);
    vec3 sma = vec3(math_minfloat);    

	for (int i = 0; i < m_metaScene.m_metaModellList.size(); i++)
	{
		MetaModel &md = m_metaScene.m_metaModellList[i];
		auto &iter = m_models.find(md.name);

		if (iter != m_models.end())
		{
            BoundingBox bb = iter->second->m_bb;

            vec3 mi = bb.mi();
            vec3 ma = bb.ma();

	        mat4 viewTrans = mat4::scale(params::inst()->globalSceneScale) * mat4::rotateX(-90);
	        mat4 m = viewTrans * md.transformation;

            mi = m * mi;
            ma = m * ma;

            if(smi.x > mi.x)
                smi.x = mi.x;
            if(smi.y > mi.y)
                smi.y = mi.y;
            if(smi.z > mi.z)
                smi.z = mi.z;

            if(smi.x > ma.x)
                smi.x = ma.x;
            if(smi.y > ma.y)
                smi.y = ma.y;
            if(smi.z > ma.z)
                smi.z = ma.z;

            if(sma.x < ma.x)
                sma.x = ma.x;
            if(sma.y < ma.y)
                sma.y = ma.y;
            if(sma.z < ma.z)
                sma.z = ma.z;    

            if(sma.x < mi.x)
                sma.x = mi.x;
            if(sma.y < mi.y)
                sma.y = mi.y;
            if(sma.z < mi.z)
                sma.z = mi.z;    
        }
        
    }
   
    vec3 t = smi + (sma-smi) * 0.5;         

    t.x = -t.x;
    t.z = -t.z;
    t.y = 0;

    m_sceneBB.setMinMax(smi, sma);
    m_camTrans = t;
}