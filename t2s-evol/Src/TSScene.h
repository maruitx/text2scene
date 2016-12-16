#pragma once

#include "Headers.h"
#include "Mesh.h"

class Object;
class TSModel;

class TSScene
{
public:
	class MetaData
	{
		public:
			MetaData() : name(""), id(0), transformation(mat4::identitiy()), material(), visible(true), path("") {}
			MetaData(const MetaData &md) { name = md.name; id = md.id; transformation = md.transformation; material = md.material; visible = md.visible; path = md.path; }
			MetaData &operator = (const MetaData &md) { name = md.name; id = md.id; transformation = md.transformation; material = md.material; visible = md.visible; path = md.path;  return *this; }

			string name;
			int id;
			mat4 transformation;
			Material material;		
			bool visible;
			string path;
	};

	TSScene(unordered_map<string, Object*> &objects);    // init with current loaded object DB
	~TSScene();

	void loadSceneFile(const QString filename, int obbOnly = false);
	//Object* searchInObjDB(string modelIdStr, bool &isObjFound);

	void loadObject(const MetaData &md);
	void render(const Transform &trans, bool applyShadow);


private:
	unordered_map<string, Object*> &m_objects;   // current loaded object DB
	vector<MetaData> m_modelList;  // models in current scene


	// File info
	QString m_sceneFileName;
	QString m_sceneFilePath;
	QString m_sceneDBPath;
	QString m_modelRepository;
	QString m_sceneFormat;

	BoundingBox m_sceneBB;

	int m_modelNum;
};

