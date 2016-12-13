#include "modelDatabase.h"
#include "category.h"
#include "../common/geometry/CModel.h"
#include "../common/utilities/utility.h"
#include <QDir>
#include <QFile>
#include <QTextStream>

ModelDatabase::ModelDatabase()
{
	// load model DB path
	QString currPath = QDir::currentPath();
	std::string dbPathFileName = currPath.toStdString() + "/ModelDBPath.txt";
	auto lines = GetFileLines(dbPathFileName, 0);
	m_dbPath = QString(lines[0].c_str());
	m_dbMetaFileType = QString(lines[1].c_str());

	m_parentCatNum = 0;
	m_modelNum = 0;
}

ModelDatabase::ModelDatabase(const QString &dbPath)
{
	m_dbPath = dbPath;

	m_parentCatNum = 0;
	m_modelNum = 0;
}

ModelDatabase::~ModelDatabase()
{

}

void ModelDatabase::loadModelTsv(const QString &modelsTsvFile)
{
	auto lines = GetFileLines(modelsTsvFile.toStdString(), 3);

	for (const std::string &line : lines)
	{
		auto parts = PartitionString(line, "\t");

		QString modelIdStr = QString(parts[0].c_str());
		MetaModel *cm = new MetaModel(modelIdStr);			
		dbMetaModels[modelIdStr] = cm;

		if (parts.size() >= 2 && parts[1].size() > 0)
		{
			QString catName = QString(parts[1].c_str());			

			if (parts.size() >= 3)
			{
				catName = QString(parts[2].c_str()); // overwrite category
			}

			catName = catName.toLower();

			// remove "_" in catName
			catName.remove("_");

			cm->setCatName(catName);

			if (dbCategories.count(catName) == 0)
			{
				Category *cat = new Category(catName);
				dbCategories[catName] = cat;
			}
		}

		dbCategories[cm->getCatName()]->addInstance(cm);
	}
}

void ModelDatabase::readModelScaleFile(const QString &filename)
{
	auto lines = GetFileLines(filename.toStdString(), 0);

	//std::map<std::string, float> modelScaleById;
	for (int lineIndex = 1; lineIndex < lines.size(); lineIndex++)
	{
		//wss.383955142f43ca0b4063d41fae33f144, 0.026278285548332416, 1, sceneScales, , diagonal
		auto parts = PartitionString(lines[lineIndex], ",");
		if (parts.size() >= 2)
		{
			QString modelIdStr = QString(parts[0].c_str());
			if (dbMetaModels.count(modelIdStr) > 0)
			{
				dbMetaModels[modelIdStr]->setScale(StringToFloat(parts[1]));
			}
		}
	}
}

bool ModelDatabase::loadSceneSpecifiedModelFile(const QString &filename, QStringList &objNameStrings, bool isSharedModelFile)
{
	QFile inFile(filename);
	QTextStream ifs(&inFile);

	if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

	QString currLine;

	while (!ifs.atEnd())
	{
		currLine = ifs.readLine();
		QStringList lineparts = currLine.split("_");

		QString modelIdStr = lineparts[lineparts.size() - 1];
		MetaModel *cm = new MetaModel(modelIdStr);
		dbMetaModels[modelIdStr] = cm;

		QString catName = lineparts[0];

		if (lineparts.size() > 1)
		{
			objNameStrings.push_back(lineparts[1]);
		}		

		if (catName.contains("room"))
		{
			catName = "room";
		}

		else if (catName[catName.size() - 1].isDigit())   // get rid of the digit in the object category 
		{
			catName = catName.left(catName.size() - 1);
		}

		if (catName == "openbook")
		{
			catName = "book";
		}

		if (isSharedModelFile)
		{
			catName = QString("shared_" + catName);
		}
		
		cm->setCatName(catName);

		if (dbCategories.count(catName) == 0)
		{
			Category *cat = new Category(catName);
			dbCategories[catName] = cat;
		}

		dbCategories[cm->getCatName()]->addInstance(cm);
	}

	inFile.close();
	return true;
}

CModel* ModelDatabase::getModelByCat(const QString &catName)
{
	Category* currCat = getCategory(catName);
	MetaModel* candiModel = currCat->sampleInstance();

	QString modelIdStr = candiModel->getIdStr();
	
	CModel *m = getModelById(modelIdStr);

	return m;
}

CModel* ModelDatabase::getModelById(QString idStr)
{
	CModel *m = new CModel();
	
	if (idStr.contains("wss."))
	{
		idStr = idStr.remove("wss.");
	}
	//m->loadModel(m_dbPath + "/wss.models/models/" + idStr +".obj", candiModel->getScale());

	m->loadModel(m_dbPath + "/" + idStr + ".obj", 1.0, 0, "SimpleSceneFormat");  // load model and cat name in .anno file

	//QString catName = getModelCat("wss." + idStr);
	//m->setCatName(catName);   // set cat name in DB csv file
	
	return m;
}

Category* ModelDatabase::getCategory(QString catName)
{
	//if (catName == "mouse")
	//{
	//	catName = QString("computermouse");
	//}

	//if (catName == "wine glass")
	//{
	//	catName = QString("drinkingutensil");
	//}

	if (dbCategories.count(catName) == 0)
	{
		QString synName;

		if (catName == "officechair")
		{
			synName = "diningchair";
		}

		if (catName == "diningchair")
		{
			synName = "officechair";
		}

		if (catName == "diningtable")
		{
			synName = "officedesk";
		}

		if (catName == "officedesk")
		{
			synName = "diningtable";
		}

		return dbCategories.find(synName)->second;
	}

	return dbCategories.find(catName)->second;
}

bool ModelDatabase::isCatInDB(QString catname)
{
	if (dbCategories.count(catname) == 0)
	{
		if (catname == "coffeetable")
		{
			catname = "sidetable";
			return dbCategories.count(catname) != 0;
		}

		if (catname=="officechair")
		{
			catname = "diningchair";
			return dbCategories.count(catname) != 0;
		}

		return false;
	}
	else
		return true;
}

// add models to annotated models: 1. create modelname.arv under interaction map
void ModelDatabase::extractScaledAnnoModels()
{
	int cutPos = m_dbPath.lastIndexOf("/");
	QString dbParentPath = m_dbPath.left(cutPos);

	QString interFolder = QString(dbParentPath + "/interaction_maps");

	QDir interFolderDir(interFolder);

	if (interFolderDir.exists())
	{
		QStringList fileList = interFolderDir.entryList();
		QStringList modelNames;

		foreach(QString s, fileList)
		{
			if (s.contains(".arv"))
			{
				modelNames.push_back(s.remove(".arv"));
			}
		}

		QString selModelFileName = QString(dbParentPath + "/ann_model_categories.tsv");
		QString selModelScaleName = QString(dbParentPath + "/ann_model_scales.csv");

		QFile modelFile(selModelFileName);
		QTextStream ofs_model(&modelFile);

		QFile scaleFile(selModelScaleName);
		QTextStream ofs_scale(&scaleFile);
		

		if (!modelFile.open(QIODevice::ReadWrite | QIODevice::Text)) return;
		if (!scaleFile.open(QIODevice::ReadWrite | QIODevice::Text)) return;


		foreach(QString s, modelNames)
		{
			MetaModel *m = dbMetaModels[s];

			if (m!=NULL)    // skip those with no category in model_categories.tsv, e.g. room
			{
				ofs_model << m->getIdStr() << "\t" << m->getCatName() << "\n";
				ofs_scale << m->getIdStr() << "," << m->getScale() << "\n";
			}
		}

		modelFile.close();
		scaleFile.close();

		foreach(QString s, modelNames)
		{
			QString modelIdStr = s;

			if (modelIdStr.contains("wss."))
			{
				modelIdStr = modelIdStr.remove("wss.");
			}

			QString modelFileName = QString(dbParentPath + "/models_scaled/" + modelIdStr + ".obj");

			QFile modelFile(modelFileName);
			if (!modelFile.exists())
			{
				CModel *m = new CModel();

				if (!modelIdStr.contains("room"))   // do not scale room, since no scale info in scale file
				{
					m->loadModel(dbParentPath + "/wss.models/models/" + modelIdStr + ".obj", dbMetaModels[s]->getScale());
					m->saveModel(dbParentPath + "/models_scaled/" + modelIdStr + ".obj");
				}
			}
		}

		Simple_Message_Box("Extract scaled models done!");
	}

	else
	{
		Simple_Message_Box("Cannot find interaction Map folder");
	}
}

// extract annotated model
void ModelDatabase::extractModelWithTexture()
{
	int cutPos = m_dbPath.lastIndexOf("/");
	QString dbParentPath = m_dbPath.left(cutPos);

	QString interFolder = QString(dbParentPath + "/interaction_maps");

	QDir interFolderDir(interFolder);

	if (interFolderDir.exists())
	{
		QStringList fileList = interFolderDir.entryList();
		QStringList modelNames;

		foreach(QString s, fileList)
		{
			if (s.contains(".arv"))
			{
				modelNames.push_back(s.remove(".arv"));
			}
		}

		foreach(QString s, modelNames)
		{
			QString modelIdStr = s;

			if (modelIdStr.contains("wss."))
			{
				modelIdStr = modelIdStr.remove("wss.");
			}

			QString tarModelFileName = QString(dbParentPath + "/models_with_texture/" + modelIdStr + ".obj");
			QString originModelFileName = QString(dbParentPath + "/wss.models/models/" + modelIdStr + ".obj");

			QString tarMtlFileName = QString(dbParentPath + "/models_with_texture/" + modelIdStr + ".mtl");
			QString originMtlFileName = QString(dbParentPath + "/wss.models/models/" + modelIdStr + ".mtl");

			QFile tarModelFile(tarModelFileName);

			if (!tarModelFile.exists())  // skip file that has been copied
			{
				QFile::copy(originModelFileName, tarModelFileName);

				QFile originMtlFile(originMtlFileName);

				if (originMtlFile.open(QIODevice::ReadOnly | QIODevice::Text))
				{
					QFile::copy(originMtlFileName, tarMtlFileName);
					QTextStream ofs_originMtl(&originMtlFile);

					QString currLine;

					while (!ofs_originMtl.atEnd())
					{
						currLine = ofs_originMtl.readLine();

						if (currLine.contains(".jpg"))
						{
							QStringList strList = currLine.split(" ");
							QString imgName = strList[1];   //e.g. map_Ka 0d10c44f0bd46c38.jpg

							QString tarImgFileName = dbParentPath + "/models_with_texture/" + "/" + imgName;
							QString originImgFileName = dbParentPath + "/wss.models/models/" + "/" + imgName;

							QFile::copy(originImgFileName, tarImgFileName);
						}
					}

					originMtlFile.close();
				}
			}	
		}

		Simple_Message_Box("Extract textured models done!");
	}
}

int ModelDatabase::getModelNum()
{
	return dbMetaModels.size();
}

QString ModelDatabase::getModelIdStr(int id)
{
	std::map<QString, MetaModel*>::iterator it = dbMetaModels.begin();

	std::advance(it, id);

	return it->first;
}

QString ModelDatabase::getModelCat(const QString &idStr)
{
	MetaModel *cm = dbMetaModels[idStr];
	return cm->getCatName();
}

void ModelDatabase::loadShapeNetSemTxt()
{
	QString shapeNetSemTxtFileName = m_dbPath + "/" + m_dbMetaFileType + ".txt";

	auto lines = GetFileLines(shapeNetSemTxtFileName.toStdString(), 3);

	// parsing from second line
	for (int i = 1; i < lines.size(); i++)
	{
		auto parts = PartitionString(lines[i], ",", "\"");

		QString modelIdStr = QString(parts[0].c_str());
		
		modelIdStr.remove("wss.");

		MetaModel *candiModel = new MetaModel(modelIdStr);

		if (parts[6] != "")   // some model's scale is empty
		{
			candiModel->setScale(QString(parts[6].c_str()).toDouble());  // the 7-th entry in each line is the unit(scale)
		}
		
		dbMetaModels[modelIdStr] = candiModel;
		m_modelNum++;

		if (parts.size() >= 2 && parts[1].size() > 0)
		{
			QString catNames = QString(parts[1].c_str());

			if (catNames.contains("\""))
			{
				catNames.remove("\"");
			}

			catNames = catNames.toLower();

			// split cat names
			auto catNameList = PartitionString(catNames.toStdString(), ",");

			for (int c = 0; c < catNameList.size(); c++)
			{
				QString currCatName = QString(catNameList[c].c_str());
				candiModel->addCatName(currCatName); // model could have multiple category names

				if (dbCategories.count(currCatName) == 0)
				{
					Category *cat = new Category(currCatName);
					cat->setCatgoryLevel(c);
					dbCategories[currCatName] = cat;
					dbCategories[currCatName]->addInstance(candiModel);

					if (c == 0)
					{
						m_parentCatNum++;
					}
				}
				else
				{
					dbCategories[currCatName]->addInstance(candiModel);
				}
			}

			// set sub-category
			for (int c = 1; c < catNameList.size(); c++)
			{
				dbCategories[catNameList[0].c_str()]->addSubCatNames(QString(catNameList[c].c_str()));
			}
		}
	}
}

MetaModel::MetaModel()
{
	m_idStr = "";
	m_categoryName = "";
	m_scale = 1.0;
}

MetaModel::MetaModel(const QString &s)
{
	m_idStr = s;
	m_categoryName = "";
	m_scale = 1.0;
}
