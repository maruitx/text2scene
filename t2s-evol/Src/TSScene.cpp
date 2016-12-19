#include "TSScene.h"
#include "Utility.h"
#include "TSModel.h"
#include "Model.h"

TSScene::TSScene(unordered_map<string, Model*> &models, const QString &fileName)
: m_models(models),
  m_sceneBB(vec3(math_maxfloat), vec3(math_minfloat)), 
  m_frameCount(0)
{
	loadSceneFile(fileName);
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

	cout << "done." << endl;
}

void TSScene::render(const Transform &trans, bool applyShadow)
{
	int nrLoaded = 0;

	for (int i = 0; i < m_modelList.size(); i++)
	{
		MetaData &md = m_modelList[i];
		auto &iter = m_models.find(md.name);

		if (iter != m_models.end())
		{
			iter->second->render(trans, md.transformation, applyShadow);
		}
		else if (md.path.size() > 0)
		{
			if (nrLoaded == 0 && m_frameCount % 20 == 0)
			{
				Model *model = new Model(md.path.c_str());
				m_models.insert(make_pair(md.name, model));
			}

			nrLoaded++;
		}
	}

	m_frameCount++;
}

void TSScene::renderDepth(const Transform &trans)
{
	int nrLoaded = 0;

	for (int i = 0; i < m_modelList.size(); i++)
	{
		MetaData &md = m_modelList[i];
		auto &iter = m_models.find(md.name);

		if (iter != m_models.end())
		{
			iter->second->renderDepth(trans, md.transformation);
		}
	}
}

void TSScene::makeRandom()
{
	for (int i = 1; i < m_modelList.size(); i++)
	{
		MetaData &md = m_modelList[i];
		
		md.transformation.a14 += rand<float>(-10, 10);
		md.transformation.a24 += rand<float>(-10, 10);
		md.transformation.a34 += rand<float>(-10, 10);
	}
}