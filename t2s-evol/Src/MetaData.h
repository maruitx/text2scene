# pragma once

#include "Headers.h"
#include "Mesh.h"

#include "SuppPlane.h"

class MetaModel
	{
		public:
			MetaModel() : name(""), id(0), transformation(mat4::identitiy()), collisionTransVec(vec3()), material(), visible(true), path(""), textureDir(""), 
				frontDir(vec3(0, -1, 0)), upDir(vec3(0, 0, 1)), position(vec3(0, 0, 0)), suppPlane(SuppPlane()), parentPlaneUVH(vec3(0.5,0.5,0)), 
				isInitLoaded(false), isAlreadyPlaced(false), isSelected(false), renderMode(0), trialNum(0)
			{
                //replace for proper selection rendering;
               // isSelected = rand() % 2;
            }
			MetaModel(const MetaModel &md) { 
				name = md.name; id = md.id; transformation = md.transformation; collisionTransVec = md.collisionTransVec; material = md.material; visible = md.visible; path = md.path; textureDir = md.textureDir; 
				frontDir = md.frontDir; upDir = md.upDir; position = md.position; suppPlane = md.suppPlane; parentPlaneUVH = md.parentPlaneUVH;
				isInitLoaded = md.isInitLoaded; isAlreadyPlaced = md.isAlreadyPlaced; renderMode = md.renderMode;  isSelected = md.isSelected; trialNum = md.trialNum;
			};
			MetaModel &operator = (const MetaModel &md) { 
				name = md.name; id = md.id; transformation = md.transformation; collisionTransVec = md.collisionTransVec; material = md.material; visible = md.visible; path = md.path; textureDir = md.textureDir;
				frontDir = md.frontDir; upDir = md.upDir; position = md.position; suppPlane = md.suppPlane; parentPlaneUVH = md.parentPlaneUVH;
				isInitLoaded = md.isInitLoaded; isAlreadyPlaced = md.isAlreadyPlaced;  isSelected = md.isSelected; renderMode = md.renderMode; trialNum = md.trialNum; return *this;
			}

			string name;
			int id;
			mat4 transformation;
			vec3 collisionTransVec;

			Material material;		
			bool visible;
			string path;
			string textureDir;

			vec3 frontDir;
			vec3 upDir;
			vec3 position;

			SuppPlane suppPlane;
			vec3 parentPlaneUVH;

			bool isInitLoaded; // whether the model is loaded at the beginning
			bool isAlreadyPlaced; // whether the model is already placed in the scene

            bool isSelected;
			int renderMode;

			int trialNum; // trialNum for re-location because of collision
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