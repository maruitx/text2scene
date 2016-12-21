# pragma once

#include "Headers.h"
#include "Mesh.h"

class MetaModel
	{
		public:
			MetaModel() : name(""), id(0), transformation(mat4::identitiy()), material(), visible(true), path("") {}
			MetaModel(const MetaModel &md) { name = md.name; id = md.id; transformation = md.transformation; material = md.material; visible = md.visible; path = md.path; }
			MetaModel &operator = (const MetaModel &md) { name = md.name; id = md.id; transformation = md.transformation; material = md.material; visible = md.visible; path = md.path;  return *this; }

			string name;
			int id;
			mat4 transformation;
			Material material;		
			bool visible;
			string path;
	};

class MetaScene
{
public:
	MetaScene() {};
	~MetaScene(){};

	vector<MetaModel> m_metaModellList;
	QString m_sceneFormat;

	// File info
	QString m_sceneFileName;
	QString m_sceneFilePath;
	QString m_sceneDBPath;
	QString m_modelRepository;

};