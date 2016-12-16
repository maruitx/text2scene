#include "TSScene.h"
#include "Utility.h"
#include "TSModel.h"

TSScene::TSScene(unordered_map<string, Object*> &objects)
: m_objects(objects), 
  m_sceneBB(vec3(math_maxfloat), vec3(math_minfloat))
{

}


TSScene::~TSScene()
{
}

void TSScene::loadSceneFile(const QString filename, int obbOnly /*= false*/)
{
	QFile inFile(filename);
	QTextStream ifs(&inFile);

	if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) return;

	QFileInfo sceneFileInfo(inFile.fileName());
	m_sceneFileName = sceneFileInfo.baseName();   // scene_01.txt
	m_sceneFilePath = sceneFileInfo.absolutePath();

	int cutPos = sceneFileInfo.absolutePath().lastIndexOf("/");
	m_sceneDBPath = sceneFileInfo.absolutePath().left(cutPos);

	QString databaseType;
	QString modelFileName;

	ifs >> databaseType;

	m_sceneFormat = databaseType;

	if (databaseType == QString("StanfordSceneDatabase"))
	{
		int currModelID = -1;
		m_modelRepository = m_sceneDBPath + "/models";

		while (!ifs.atEnd())
		{
			QString currLine = ifs.readLine();
			if (currLine.contains("modelCount "))
			{
				m_modelNum = StringToIntegerList(currLine.toStdString(), "modelCount ")[0];
				m_modelList.resize(m_modelNum);
			}

			if (currLine.contains("newModel "))
			{
				currModelID++;

				std::vector<std::string> parts = PartitionString(currLine.toStdString(), " ");				
				int modelIndex = StringToInt(parts[1]);

				m_modelList[currModelID].id = modelIndex;
				m_modelList[currModelID].name = parts[2];
				m_modelList[currModelID].path = m_modelRepository.toStdString() + "/" + parts[2] + ".obj";
			}

			if (currLine.contains("transform "))
			{
				std::vector<float> transformVec = StringToFloatList(currLine.toStdString(), "transform ");  // transformation vector in stanford scene file is column-wise
				mat4 transMat(transformVec.data());
				transMat = transMat.transpose();
				m_modelList[currModelID].transformation = transMat;
			}
		}
	}
}

//void TSScene::loadSceneFile(const QString filename, int obbOnly /*= false*/ )
//{
//	QFile inFile(filename);
//	QTextStream ifs(&inFile);
//
//	if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) return;
//
//	QFileInfo sceneFileInfo(inFile.fileName());
//	m_sceneFileName = sceneFileInfo.baseName();   // scene_01.txt
//	m_sceneFilePath = sceneFileInfo.absolutePath();
//
//	int cutPos = sceneFileInfo.absolutePath().lastIndexOf("/");
//	m_sceneDBPath = sceneFileInfo.absolutePath().left(cutPos);
//
//	QString databaseType;
//	QString modelFileName;
//
//	ifs >> databaseType;
//
//	m_sceneFormat = databaseType;
//
//	if (databaseType == QString("StanfordSceneDatabase"))
//	{
//		int currModelID = -1;
//
//		m_modelRepository = m_sceneDBPath + "/models";
//
//		while (!ifs.atEnd())
//		{
//			QString currLine = ifs.readLine();
//			if (currLine.contains("modelCount "))
//			{
//				m_modelNum = StringToIntegerList(currLine.toStdString(), "modelCount ")[0];
//			}
//
//			if (currLine.contains("newModel "))
//			{
//				std::vector<std::string> parts = PartitionString(currLine.toStdString(), " ");
//				int modelIndex = StringToInt(parts[1]);
//
//				bool isObjFound = false;
//
//				Object* currObj = searchInObjDB(parts[2], isObjFound);
//
//				if (isObjFound)
//				{
//					TSModel *newModel = new TSModel(currObj);					
//					m_modelList.push_back(newModel);
//				}
//				else
//				{
//					Object* newObj = new Object(m_modelRepository + "/" + QString(parts[2].c_str()) + ".obj", false, true, true, vec3(), vec3(1.0f), vec4(0, 0, 0, 0), vec4(1, 1, 1, 1));
//					//Object* newObj = new Object(m_modelRepository + "/" + QString(parts[2].c_str()) + ".obj", false, true, true, vec3(), vec3(0.1f), vec4(1, 0, 0, 90), vec4(1, 1, 1, 1));
//					TSModel *newModel = new TSModel(newObj);
//
//					m_modelList.push_back(newModel);
//
//					m_objects.insert(make_pair(parts[2], newObj));   // add object to DB
//					newObj->start();
//				}
//		
//				currModelID += 1;
//
//			}
//
//			if (currLine.contains("transform "))
//			{
//				std::vector<float> transformVec = StringToFloatList(currLine.toStdString(), "transform ");  // transformation vector in stanford scene file is column-wise
//				mat4 transMat(transformVec.data());
//
//				transMat = transMat.transpose();
//
//				m_modelList[currModelID]->setInitTrans(transMat);
//			}
//		}
//	}
//}

//Object* TSScene::searchInObjDB(string modelIdStr, bool &isObjFound)
//{
//	auto &iter = m_objects.find(modelIdStr);
//
//	if (iter != m_objects.end())
//	{
//		isObjFound = true;
//		return iter->second;
//	}
//	else
//	{
//		isObjFound = false;
//		return nullptr;
//	}
//}

void TSScene::render(const Transform &trans, bool applyShadow)
{
	for (int i = 0; i < m_modelList.size(); i++)
	{
		MetaData &md = m_modelList[i];
		auto &iter = m_objects.find(md.name);

		if (iter != m_objects.end())
		{
			vec3 ma = m_sceneBB.ma();
			vec3 mi = m_sceneBB.mi();

			vec3 t = -mi - ma * 0.5;
			t = vec3(t.x, t.y, 0);

			iter->second->render(trans, md.transformation, t, md.material, applyShadow);
		}
		else
		{
			loadObject(md);
		}
	}
}

void TSScene::loadObject(const MetaData &md)
{
	Object *obj = new Object(QString(md.path.c_str()), false, true, true, vec3(), vec3(1), vec4(0, 0, 0, 0), vec4(1, 1, 1, 1), md.transformation);
	m_objects.insert(make_pair(md.name, obj));

	mat4 m = md.transformation;

	vec3 omi = m * obj->m_bb.mi();
	vec3 oma = m * obj->m_bb.ma();

	vec3 smi = m_sceneBB.mi();
	vec3 sma = m_sceneBB.ma();

	if (omi.x < smi.x)
		smi.x = omi.x;
	if (omi.y < smi.y)
		smi.y = omi.y;
	if (omi.z < smi.z)
		smi.z = omi.z;

	if (oma.x > sma.x)
		sma.x = oma.x;
	if (oma.y > sma.y)
		sma.y = oma.y;
	if (oma.z > sma.z)
		sma.z = oma.z;

	m_sceneBB.setMinMax(smi, sma);

	//obj->start();
}