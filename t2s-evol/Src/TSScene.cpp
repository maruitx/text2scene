#include "TSScene.h"
#include "Utility.h"
#include "TSModel.h"

TSScene::TSScene(unordered_map<string, Object*> &objects)
: m_objects(objects)
{

}


TSScene::~TSScene()
{
}

void TSScene::loadSceneFile(const QString filename, int obbOnly /*= false*/ )
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
			}

			if (currLine.contains("newModel "))
			{
				std::vector<std::string> parts = PartitionString(currLine.toStdString(), " ");
				int modelIndex = StringToInt(parts[1]);

				bool isObjFound = false;

				Object* currObj = searchInObjDB(parts[2], isObjFound);

				if (isObjFound)
				{
					TSModel *newModel = new TSModel(currObj);
					
					m_modelList.push_back(newModel);
				}
				else
				{
					Object* newObj = new Object(m_modelRepository + "/" + QString(parts[2].c_str()) + ".obj", true, true, true, vec3(), vec3(1.0f), vec4(), vec4());
					TSModel *newModel = new TSModel(newObj);

					m_modelList.push_back(newModel);

					m_objects.insert(make_pair(parts[2], newObj));   // add object to DB
					newObj->start();
				}
		
				currModelID += 1;

			}

			if (currLine.contains("transform "))
			{
				std::vector<float> transformVec = StringToFloatList(currLine.toStdString(), "transform ");  // transformation vector in stanford scene file is column-wise
				mat4 transMat(transformVec.data());

				transMat = transMat.transpose();

				m_modelList[currModelID]->setInitTrans(transMat);
			}
		}
	}
}

Object* TSScene::searchInObjDB(string modelIdStr, bool &isObjFound)
{
	auto &iter = m_objects.find(modelIdStr);

	if (iter != m_objects.end())
	{
		isObjFound = true;
		return iter->second;
	}
	else
	{
		isObjFound = false;
		return nullptr;
	}
}

void TSScene::render(const Transform &trans, bool applyShadow)
{
	for (int i = 0; i < m_modelNum; i++)
	{
		Material mt;
		mt.initRandom();
		m_modelList[i]->render(trans, mt, applyShadow);
	}
}
