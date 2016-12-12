#include "../common/utilities/utility.h"


class CandidateModel;

class Category
{
public:
	Category();
	Category(const QString &s);
	~Category();
	CandidateModel* sampleInstance();
	void setCatName(const QString &n) { m_catName = n; };
	QString getCatName() { return m_catName; };

	void addInstance(CandidateModel *m) { modelInstances.push_back(m); };
	int getInstanceNum() { return modelInstances.size(); };

	void setCatgoryLevel(int l){ m_categoryLevel = l; };
	int getCatgoryLevel() { return m_categoryLevel; };
	void addSubCatNames(const QString &s) { m_subCatNames.push_back(s); };

	bool isInModelBlackList(const QString &s);
	bool isSharedModel(const QString &s);

	std::vector<CandidateModel*> modelInstances;

private:
	QString m_catName;
	std::vector<QString> m_subCatNames;
	

	QStringList m_modelBlackList;
	QStringList m_sharedModelList;

	int m_categoryLevel;
};