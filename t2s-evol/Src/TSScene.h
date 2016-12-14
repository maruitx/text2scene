#pragma once

#include "Headers.h"

class Object;
class TSModel;

class TSScene
{
public:
	TSScene(unordered_map<string, Object*> &objects);    // init with current loaded object DB
	~TSScene();

	void loadSceneFile(const QString filename, int obbOnly = false);
	Object* searchInObjDB(string modelIdStr, bool &isObjFound);

	void render(const Transform &trans, bool applyShadow);


private:
	unordered_map<string, Object*> &m_objects;   // current loaded object DB
	vector<TSModel*> m_modelList;  // models in current scene


	// File info
	QString m_sceneFileName;
	QString m_sceneFilePath;
	QString m_sceneDBPath;
	QString m_modelRepository;
	QString m_sceneFormat;

	int m_modelNum;
};

