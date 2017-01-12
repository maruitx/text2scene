# pragma once

#include "Headers.h"
#include "Mesh.h"

class MetaModel
	{
		public:
			MetaModel() : name(""), id(0), transformation(mat4::identitiy()), material(), visible(true), path(""), textureDir(""), frontDir(vec3(0, -1, 0)), upDir(vec3(0, 0, 1)), position(vec3(0, 0, 0)), isInited(false) {}
			MetaModel(const MetaModel &md) { name = md.name; id = md.id; transformation = md.transformation; material = md.material; visible = md.visible; path = md.path; textureDir = md.textureDir; frontDir = md.frontDir; upDir = md.upDir; position = md.position; isInited = md.isInited; };
			MetaModel &operator = (const MetaModel &md) { name = md.name; id = md.id; transformation = md.transformation; material = md.material; visible = md.visible; path = md.path; textureDir = md.textureDir; isInited = md.isInited;  return *this; }

			string name;
			int id;
			mat4 transformation;
			Material material;		
			bool visible;
			string path;
			string textureDir;

			vec3 frontDir;
			vec3 upDir;
			vec3 position;

			bool isInited;
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