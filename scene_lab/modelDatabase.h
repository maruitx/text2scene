#pragma once
#include "../common/utilities/utility.h"

class CModel;
class Category;

class CandidateModel
{
public:
	CandidateModel();
	CandidateModel(const QString &s);

	void setCatName(const QString &s) { m_categoryName = s; };
	QString getCatName() { return m_categoryName; };

	void addCatName(const QString &s) { m_categoryNames.push_back(s); };

	void setScale(double s) { m_scale = s; };
	double getScale() { return m_scale; };

	QString getIdStr() { return m_idStr; };

private:
	QString m_idStr;
	QString m_categoryName;

	QVector<QString> m_categoryNames;

	double m_scale;
};

class ModelDatabase{
public:
	ModelDatabase();
	ModelDatabase(const QString &dbPath);
	~ModelDatabase();

	QString getDBPath() { return m_dbPath; };

	void loadModelTsv(const QString &modelsTsvFile);
	void readModelScaleFile(const QString &filename);

	bool loadSceneSpecifiedModelFile(const QString &filename, QStringList &objNameStrings = QStringList(), bool isSharedModelFile = false);
	bool isCatInDB(QString catname);

	void extractScaledAnnoModels();
	void extractModelWithTexture();

	void loadShapeNetSemTxt();

	CModel* getModelById(QString idStr);
	CModel* getModelByCat(const QString &catName);	
	Category* getCategory(QString catName);

	QString getModelCat(const QString &idStr);

	QString getModelIdStr(int id);
	int getModelNum();

	int getParentCatNum() { return m_parentCatNum; };

	std::map<QString, CandidateModel*> dbCandiModels; // <modelIdStr, CandidateModel>
	std::map<QString, Category*> dbCategories;  // <categoryName, categoryStruct>


private:
	QString m_dbPath;
	QString m_dbMetaFileType;

	int m_modelNum;
	int m_parentCatNum;
};