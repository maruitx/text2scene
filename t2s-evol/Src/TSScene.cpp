#include "TSScene.h"
#include "Utility.h"
#include "Model.h"
#include "SceneSemGraph.h"


TSScene::TSScene(unordered_map<string, Model*> &models)
	:m_models(models),
	m_sceneBB(vec3(math_maxfloat), vec3(math_minfloat)),
	m_modelNum(0),
	m_frameCount(0),
	m_loadedModelNum(0),
	m_isLoadingDone(false),
	m_isRenderRoom(true),
	m_isLoadFromFile(false),
	m_ssg(NULL), 
    m_camTrans(0.0f, 0.0f, 0.0f), 
    m_renderMode(0)
{
	
}

TSScene::TSScene(unordered_map<string, Model*> &models, const QString &fileName)
: m_models(models),
  m_sceneBB(vec3(math_maxfloat), vec3(math_minfloat)),
  m_modelNum(0),
  m_frameCount(0),
  m_loadedModelNum(0),
  m_isLoadingDone(false),
  m_isRenderRoom(true),
  m_isLoadFromFile(false),
  m_ssg(NULL), 
  m_camTrans(0.0f, 0.0f, 0.0f), 
  m_renderMode(0)
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
	m_isLoadingDone(false),
	m_isRenderRoom(true),
	m_isLoadFromFile(false),
	m_ssg(NULL), 
    m_camTrans(0.0f, 0.0f, 0.0f), 
    m_renderMode(0)
{
	m_modelNum = m_metaScene.m_metaModellList.size();
	countLoadedModelNum();
}

TSScene::~TSScene()
{
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

	m_metaScene.m_sceneDBPath = QString(params::inst()->sceneDirectory.c_str());

	QString databaseType;
	QString modelFileName;

	ifs >> databaseType;

	m_metaScene.m_sceneFormat = databaseType;

	//if (databaseType == QString("StanfordSceneDatabase")) {};

	{
		int currModelID = -1;
		m_metaScene.m_modelRepository = QString(params::inst()->modelDirectory.c_str());

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

				string objPathName = m_metaScene.m_modelRepository.toStdString() + "/" + parts[2] + ".obj";
				m_metaScene.m_metaModellList[currModelID].path = objPathName;

				// check whether obj file exist in current DB
				// if not exist, check ShapeNetSem DB for this model
				if (!fileExists(objPathName))
				{
					objPathName = params::inst()->shapeNetSemDirectory + "models-OBJ/models/" + parts[2] + ".obj";
					string texDir = params::inst()->shapeNetSemDirectory + "models-textures/textures/";

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
				int renderMode = StringToIntegerList(currLine.toStdString(), "renderingMode ")[0];  // transformation vector in stanford scene file is column-wise
				m_metaScene.m_metaModellList[currModelID].isSelected = renderMode;
			}
		}
	}

	m_isLoadFromFile = true;
	cout << "done." << endl;
}

void TSScene::render(const Transform &trans, bool applyShadow)
{
    Transform tt = trans;
    tt.view *= mat4::translate(m_camTrans);
    tt.lightViews[0] *= mat4::translate(m_camTrans);

	int nrLoaded = 0;

	int trialNumLimit = 10;

	for (int i = 0; i < m_metaScene.m_metaModellList.size(); i++)
	{
		MetaModel &md = m_metaScene.m_metaModellList[i];
		auto &iter = m_models.find(md.name);

		// if set as not render room, and current room is room, then skip
		if (!m_isRenderRoom && md.name.find("room")!=std::string::npos)
		{
			continue;
		}

		if (iter != m_models.end())
		{
			if (iter->second->m_loadingDone)
			{
				if (!m_isLoadFromFile && checkCollision(iter->second, iter->second->m_bb, i) && md.trialNum < trialNumLimit)
				{
					resolveCollision(i);
					md.trialNum++;

					if (md.trialNum == trialNumLimit)
					{
						qDebug() << QString("   Preview %1 Reach test trial limit; Place model anyway; Collision may exist").arg(m_previewId);
						md.isAlreadyPlaced = true; // reach trial limit, although collision happens still set it to be placed
					}
				}
				else
				{
					md.isAlreadyPlaced = true;
					iter->second->render(tt, md.transformation, applyShadow, md.textureDir, m_renderMode, md.isSelected);
				}
			}	
		}
		else if (md.path.size() > 0)
		{
			if (nrLoaded == 0 && m_frameCount % 20 == 0)
			{
				// if model exist
				if (fileExists(md.path.c_str()) && md.name != "roomDefault")
				{
					Model *model = new Model(md.path.c_str());
					m_models.insert(make_pair(md.name, model));

					m_isLoadingDone = false; // when new insert a model, slow loading in another thread
					
					nrLoaded++;					
				}	
			}			
		}
	}

	computeSceneBB();
    //renderSceneBB(trans);

	if (!m_isLoadingDone)
	{
		countLoadedModelNum();
	}

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
	if (m_metaScene.m_metaModellList.size() == 0)
	{
		m_isLoadingDone = true;
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
		m_isLoadingDone = true;
		cout << "\nFinish loading models for Preview:"<<m_previewId<<" Matched DBSSG:"<<m_ssg->m_metaScene.m_sceneFileName.toStdString()<<"\n";
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

void TSScene::updateRoomModel(MetaModel m)
{
	if (m.isInitLoaded)
	{
		for (int i = 0; i < m_metaScene.m_metaModellList.size(); i++)
		{
			// if room exist, update with loaded room
			QString modelName = QString(m_metaScene.m_metaModellList[i].name.c_str());
			if (modelName.contains("room"))
			{
				m_metaScene.m_metaModellList[i] = m;
				if (m_ssg != NULL)
				{
					m_ssg->m_metaScene.m_metaModellList[i] = m;
				}

				return;
			}
		}

		// add room if it is not exist
		loadModel(m);
	}
}

bool TSScene::checkCollision(Model *testModel, const BoundingBox &testBB, int textModelIdx)
{
	bool isCollide = false;

	// do not check for model that is already placed in the scene
	if (m_metaScene.m_metaModellList[textModelIdx].isAlreadyPlaced)
	{
		return false;
	}

	// trans mat that brings model at pos in current scene
	mat4 testModelTransMat = m_metaScene.m_metaModellList[textModelIdx].transformation;
	
	// compute the new transformed AABB
	BoundingBox transTestBB = testBB.transformBoudingBox(testModelTransMat);
	vec3 testModelMi = transTestBB.mi();
	vec3 testModelMa = transTestBB.ma();

	double delta = 0.01 / params::inst()->globalSceneUnitScale;

	QString collisionType;

	for (int i = 0; i < m_metaScene.m_metaModellList.size(); i++)
	{
		bool isModelAlreadyPlaced = m_metaScene.m_metaModellList[i].isAlreadyPlaced;

		// only check collision with model that is already placed in the scene
		if (i != textModelIdx && isModelAlreadyPlaced)
        {          		
		    MetaModel &md = m_metaScene.m_metaModellList[i];
		    auto &iter = m_models.find(md.name);

            if (iter != m_models.end())
            {
				// compute the new transformed BB
				BoundingBox transRefBB = iter->second->m_bb.transformBoudingBox(md.transformation);
				vec3 refModelMi = transRefBB.mi();
				vec3 refModelMa = transRefBB.ma();

                bool isAABBCollide = intersectAABB(testModelMi, testModelMa, refModelMi, refModelMa, delta);
                bool isOBBMeshCollide = false;
				bool isMeshMeshCollide = false;

                if(isAABBCollide)
                {

					collisionType = "AABB(true)";
					// test transformed BB to model with current scene transformation
					isOBBMeshCollide = iter->second->checkCollisionBBTriangles(testBB, testModelTransMat, md.transformation); 

					if (isOBBMeshCollide)
					{
						collisionType += "_OBBMesh(true)";
						isMeshMeshCollide = iter->second->checkCollisionTrianglesTriangles(testModel, testModelTransMat, md.transformation);
						
						if (isMeshMeshCollide)
						{
							collisionType += "_MeshMesh(true)";
						}
					}
                }

				//isCollide = isAABBCollide && isOBBMeshCollide;
				isCollide = isAABBCollide && isOBBMeshCollide && isMeshMeshCollide;				

				if (isCollide)
				{
					qDebug() << QString(" Collide in Preview:%1 Type: %2 DBSSG:%3").arg(m_previewId).arg(collisionType).arg(m_ssg->m_metaScene.m_sceneFileName);
					return isCollide;
				}
            }
        }
    }

	return false;
}

bool TSScene::intersectAABB(const vec3 &miA, const vec3 &maA, const vec3 &miB, const vec3 &maB, double delta)
{
	return (miA.x + delta <= maB.x && maA.x  - delta >= miB.x) &&
		(miA.y + delta <= maB.y && maA.y - delta >= miB.y) &&
		(miA.z + delta <= maB.z && maA.z - delta >= miB.z);
}

bool TSScene::resolveCollision(int modelId)
{
	MetaModel &currMd = m_metaScene.m_metaModellList[modelId];
	mat4 transMat;
	vec3 translateVec;

	int parentNodeId = m_ssg->findParentNodeId(modelId);
	int parentModelId = m_ssg->m_objectGraphNodeIdToModelSceneIdMap[parentNodeId];

	QString sampleType;
	double sceneMetric = params::inst()->globalSceneUnitScale;

	if (parentNodeId != -1)
	{
		MetaModel &parentMd = m_metaScene.m_metaModellList[parentModelId];

		SuppPlane &parentSuppPlane = parentMd.suppPlane;
		if (parentSuppPlane.m_isInited)
		{
			vec3 currUVH = currMd.parentPlaneUVH; // UV, and H w.r.t to parent support plane
			sampleType = " on parent-" + m_ssg->m_nodes[parentNodeId].nodeName;
			vec3 newPos = parentSuppPlane.randomSamplePointByUVH(currUVH, 0.2, 0.2, 0);
			translateVec = newPos - currMd.position;
		}
	}
	else
	{
		//translateVec = GenShiftWithNormalDistribution(0.3 / sceneMetric, 0.3 / sceneMetric);
		std::vector<double> sft(2);
		double xVar = 0.2;
		double yVar = 0.2;

		sampleType = "on floor";
		GenNRandomDouble(-1, 1, sft);
		translateVec = translateVec + vec3(xVar*sft[0] / sceneMetric, yVar*sft[1] / sceneMetric, 0);
	}

	transMat = transMat.translate(translateVec);

	currMd.position = transMat*currMd.position;
	currMd.transformation = transMat*currMd.transformation;
	currMd.frontDir = TransformVector(currMd.transformation, currMd.frontDir);
	currMd.upDir = TransformVector(currMd.transformation, currMd.upDir);
	currMd.suppPlane.tranfrom(transMat);

	qDebug() << QString("  Resolve trial:%1 Preview:%2 Type:%3 Vec:(%4,%5,%6) Name:%7").arg(currMd.trialNum).arg(m_previewId).arg(sampleType)
		.arg(translateVec.x*sceneMetric).arg(translateVec.y*sceneMetric).arg(translateVec.z*sceneMetric)
		.arg(QString(m_ssg->m_metaScene.m_metaModellList[modelId].catName.c_str()));

	return true;
}

void TSScene::toggleRenderMode()
{
    loop(m_renderMode, 0, 1, 1);
}

