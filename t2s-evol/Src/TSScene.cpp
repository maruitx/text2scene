#include "TSScene.h"
#include "Utility.h"
#include "Model.h"
#include "SceneSemGraph.h"
#include "CollisionManager.h"
#include "LayoutPlanner.h"


TSScene::TSScene(unordered_map<string, Model*> &models)
	:m_models(models),
	m_sceneBB(vec3(math_maxfloat), vec3(math_minfloat)),
	m_modelNum(0),
	m_frameCount(0),
	m_loadedModelNum(0),
	m_sceneLoadingDone(false),
	m_sceneLayoutDone(false),
	m_isRenderRoom(true),
	m_isLoadFromFile(false),
	m_floorHeight(0),
	m_allConstraintsExtracted(false),
	m_ssg(NULL), 
    m_camTrans(0.0f, 0.0f, 0.0f), 
    m_renderMode(0)
{
	m_collisionManager = new CollisionManager(this);
}

TSScene::TSScene(unordered_map<string, Model*> &models, const QString &fileName)
: m_models(models),
  m_sceneBB(vec3(math_maxfloat), vec3(math_minfloat)),
  m_modelNum(0),
  m_frameCount(0),
  m_loadedModelNum(0),
  m_sceneLoadingDone(false),
  m_sceneLayoutDone(false),
  m_isRenderRoom(true),
  m_isLoadFromFile(false),
  m_floorHeight(0),
  m_allConstraintsExtracted(false),
  m_ssg(NULL), 
  m_camTrans(0.0f, 0.0f, 0.0f), 
  m_renderMode(0)
{
	loadSceneFile(fileName);
	m_modelNum = m_metaScene.m_metaModellList.size();

	m_collisionManager = new CollisionManager(this);
	m_explictConstraints.resize(m_modelNum);
	m_implicitConstraints.resize(m_modelNum);
	m_overHangPositions.resize(m_modelNum);
}

TSScene::TSScene(unordered_map<string, Model*> &models, SceneSemGraph *ssg)
	:m_models(models),
	m_sceneBB(vec3(math_maxfloat), vec3(math_minfloat)),
	m_frameCount(0),
	m_loadedModelNum(0),
	m_sceneLoadingDone(false),
	m_sceneLayoutDone(false),
	m_isRenderRoom(true),
	m_isLoadFromFile(false),
	m_floorHeight(0),
	m_allConstraintsExtracted(false),
	m_ssg(ssg), 
	m_metaScene(ssg->m_graphMetaScene),
    m_camTrans(0.0f, 0.0f, 0.0f), 
    m_renderMode(0)
{
	m_modelNum = m_metaScene.m_metaModellList.size();
	m_collisionManager = new CollisionManager(this);

	m_explictConstraints.resize(m_modelNum);
	m_implicitConstraints.resize(m_modelNum);
	m_overHangPositions.resize(m_modelNum);
}

TSScene::~TSScene()
{
	if (m_collisionManager)
	{
		delete m_collisionManager;
	}
}

void TSScene::loadSceneFile(const QString &filename)
{
	m_metaScene.m_metaModellList.clear();

	cout << "Loading Scene File ... ";
	QFile inFile(filename);
	QTextStream ifs(&inFile);

	if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) return;

	string initTextureDir = params::inst()->textureDirectory;

	QFileInfo sceneFileInfo(inFile.fileName());
	m_metaScene.m_sceneFileName = sceneFileInfo.baseName();   // scene_01.txt
	m_metaScene.m_sceneFilePath = sceneFileInfo.absolutePath();

	m_metaScene.m_sceneDBPath = toQString(params::inst()->sceneDirectory);

	QString databaseType;
	QString modelFileName;

	ifs >> databaseType;

	m_metaScene.m_sceneFormat = databaseType;

	//if (databaseType == QString("StanfordSceneDatabase")) {};

	{
		int currModelID = -1;
		m_metaScene.m_modelRepository = toQString(params::inst()->modelDirectory);

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
				m_metaScene.m_metaModellList[currModelID].isAlreadyPlaced = true;

				string objPathName = m_metaScene.m_modelRepository.toStdString() + "/" + parts[2] + ".obj";
				m_metaScene.m_metaModellList[currModelID].path = objPathName;

				// check whether obj file exist in current DB
				// if not exist, check ShapeNetSem DB for this model
				if (!fileExists(objPathName))
				{
					objPathName = params::inst()->shapeNetSemDirectory + "models-OBJ/models/" + parts[2] + ".obj";
					//string texDir = params::inst()->shapeNetSemDirectory + "models-textures/textures/";
					string texDir = params::inst()->shapeNetSemDirectory + "models-OBJ/models/";

					// if exist in ShapeNetSem, update obje file, but the texture db is unchanged
					if (fileExists(objPathName))
					{
						m_metaScene.m_metaModellList[currModelID].path = objPathName;
						m_metaScene.m_metaModellList[currModelID].textureDir = texDir;

						cout << "\nModel " << parts[2] << " is not in StanfordDB, but is in ShapeNetSem\n";
					}

					else
					{
						cout << "\nCannot load model " << parts[2] << " even from ShapeNetSem\n";
					}
				}
			}

			if (currLine.contains("transform "))
			{
				std::vector<float> transformVec = StringToFloatList(currLine.toStdString(), "transform ");  // transformation vector in stanford scene file is column-wise
				mat4 transMat(transformVec.data());
				transMat = transMat.transpose();
				m_metaScene.m_metaModellList[currModelID].transformation = transMat;
			}

			if (currLine.contains("renderingMode "))
			{
				int renderMode = StringToIntegerList(currLine.toStdString(), "renderingMode ")[0];  //
				m_metaScene.m_metaModellList[currModelID].isSelected = renderMode;
			}
		}
	}

	m_modelNum = m_metaScene.m_metaModellList.size();

	m_explictConstraints.resize(m_modelNum);
	m_implicitConstraints.resize(m_modelNum);
	m_overHangPositions.resize(m_modelNum);

	m_isLoadFromFile = true;
	m_sceneLoadingDone = false;
	m_floorHeight = 0;
	cout << "done." << endl;
}

void TSScene::render(const Transform &trans, bool applyShadow)
{
    Transform tt = trans;
    tt.view *= mat4::translate(m_camTrans);
    tt.lightViews[0] *= mat4::translate(m_camTrans);

	// load models
	if (!m_sceneLoadingDone)
	{
		for (int i = 0; i < m_metaScene.m_metaModellList.size(); i++)
		{
			MetaModel &md = m_metaScene.m_metaModellList[i];
			auto &iter = m_models.find(md.name);

			// if a model is not loaded
			if (iter == m_models.end() && md.path.size() > 0)// && m_frameCount % 20 == 0)
			{
				// if model exist
				if (fileExists(md.path.c_str()) && md.name != "roomDefault")
				{
					if (!m_isRenderRoom && md.name.find("room") != std::string::npos) continue;

					Model *model = new Model(md.path.c_str());
					m_models.insert(make_pair(md.name, model));

					m_sceneLoadingDone = false; // when new insert a model, slow loading in another thread
				}
			}
		}

		computeSceneBB();
		countLoadedModelNum();
	}

	// start computing layout after all models are loaded
	// keep computing layout if layout is not done
	if (!m_isLoadFromFile && !m_sceneLayoutDone && m_sceneLoadingDone)
	{
		m_layoutPlanner->computeLayout(this);
		m_sceneLayoutDone = isLayoutDone();

		updateFloorHeight();

		if (m_sceneLayoutDone)
		{
			computeSceneBB();
		}
	}

	// render models
	for (int i=0; i <  m_metaScene.m_metaModellList.size(); i++)
	{
		MetaModel &md = m_metaScene.m_metaModellList[i];

		// if set as not render room, and current room is room, then skip
		if (!m_isRenderRoom && md.name.find("room") != std::string::npos) continue;

		Model *currModel = getModel(md.name);

		// only render model that passes the layout algorithm
		if (currModel!= NULL && currModel->m_loadingDone && md.isAlreadyPlaced)
		{
			currModel->render(tt, md.transformation, applyShadow, md.textureDir, m_renderMode, md.isSelected);
		}
	}

    //renderSceneBB(trans);

	m_frameCount++;
}

bool TSScene::isLayoutDone()
{
	int count = 0;
	for (int i = 0; i < m_metaScene.m_metaModellList.size(); i++)
	{
		MetaModel &md = m_metaScene.m_metaModellList[i];
		if (md.isAlreadyPlaced || md.isSkipped)
		{
			count++;
		}
	}

	if (count == m_metaScene.m_metaModellList.size())
	{
		cout << "Preview " << m_previewId << ": layout finished." << endl;
		return true;
	}
	else
	{
		return false;
	}
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

		if (iter != m_models.end() && md.isAlreadyPlaced)
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
	if (m_metaScene.m_metaModellList.size() == 0)
	{
		m_sceneLoadingDone = false;
		return;
	}

	m_loadedModelNum = 0;

	for (int i = 0; i < m_metaScene.m_metaModellList.size(); i++)
	{
		MetaModel &md = m_metaScene.m_metaModellList[i];
		auto &iter = m_models.find(md.name);

		if (iter != m_models.end() && iter->second->m_loadingDone)
		{
			m_loadedModelNum++;
		}
	}

	if (m_loadedModelNum == m_metaScene.m_metaModellList.size())
	{
		m_sceneLoadingDone = true;
		QString sceneType;

		if (m_ssg != NULL)
		{
			sceneType = " from Matched DBSSG:" + m_ssg->m_graphMetaScene.m_sceneFileName;			
		}
		else
		{
			sceneType = " from DBScene: " + m_metaScene.m_sceneFileName;		
		}

		cout << "\nFinish loading models for Preview:" << m_previewId << sceneType.toStdString() << "\n";
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

	        mat4 viewTrans = mat4::scale(params::inst()->globalSceneViewScale) * mat4::rotateX(-90);
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

void TSScene::loadModel(MetaModel m)
{
	m_metaScene.m_metaModellList.push_back(m);
	m_modelNum++;
}

void TSScene::updateFloorHeight()
{
	double minZ = 1e6;
	// use the min Z value of all models as floor height
	for (int i=0; i < m_metaScene.m_metaModellList.size(); i++)
	{
		MetaModel &md = m_metaScene.m_metaModellList[i];
		auto& iter = m_models.find(md.name);

		if (iter!=m_models.end())
		{
			Model *model = iter->second;
			vec3 bbMin = TransformPoint(md.transformation, model->m_bb.mi());

			if (bbMin.z < minZ)
			{
				minZ = bbMin.z;
			}
		}
	}

	m_floorHeight = minZ - 0.01;

	if (m_floorHeight < 0)
	{
		m_floorHeight = 0;
	}
}

void TSScene::updateRoomModel(MetaModel m)
{
	if (m.isInitLoaded)
	{
		for (int i = 0; i < m_metaScene.m_metaModellList.size(); i++)
		{
			// if room exist, update with loaded room
			QString modelName = toQString(m_metaScene.m_metaModellList[i].name);
			if (modelName.contains("room"))
			{
				m_metaScene.m_metaModellList[i] = m;
				if (m_ssg != NULL)
				{
					m_ssg->m_graphMetaScene.m_metaModellList[i] = m;
				}

				return;
			}
		}

		// add room if it is not exist
		loadModel(m);
	}
}

void TSScene::toggleRenderMode()
{
    loop(m_renderMode, 0, 1, 1);
}

Model* TSScene::getModel(const string &name)
{
	auto &iter = m_models.find(name);
	if (iter != m_models.end())
	{
		return iter->second;
	}
	else
		return NULL;
}

bool TSScene::computeZForModel(int currModelId, int parentModelId, vec3 startPt, double &newZ)
{
	MetaModel &md = m_metaScene.m_metaModellList[currModelId];
	double sceneMetric = params::inst()->globalSceneUnitScale;
	if (md.catName == "tv")
	{
		startPt.z = 1.5 / sceneMetric;
	}

	if (md.catName == "speaker")
	{
		MetaModel &parentMd = m_metaScene.m_metaModellList[parentModelId];
		if (parentMd.catName!="desk")
		{
			startPt.z = 1.5 / sceneMetric;  // for the case that spearker in on tv stand
		}
	}

	double elevationVal = 0.2 / sceneMetric;

	float3 rayStartPt = make_float3(startPt.x, startPt.y, startPt.z + elevationVal);
	float3 downDir = make_float3(0, 0, -1);
	Ray downRay(rayStartPt, downDir);

	if (m_collisionManager->isRayIntersect(downRay, parentModelId, newZ))
	{

		double h = md.parentPlaneUVH.z;
		if (h < 0.01 / sceneMetric && h > 0)
		{
			newZ += h; // add the original H to the parent top Plane
		}
		newZ += 0.005 / sceneMetric;

		adjustZForSpecificModel(md, newZ);
		return true;
	}

	return false;
}

void TSScene::adjustZForSpecificModel(const MetaModel &currMd, double &z)
{
	if (m_layoutPlanner->m_specialModels.count(toQString(currMd.name)))
	{
		z += m_layoutPlanner->m_specialModels[toQString(currMd.name)] / params::inst()->globalSceneUnitScale;
	}
}

