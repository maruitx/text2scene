# pragma once

#include "Headers.h"
#include "Mesh.h"

#include "SuppPlane.h"
#include <Eigen/Dense>

class MetaModel
	{
		public:
			MetaModel() : name(""), catName(""), id(0), transformation(mat4::identitiy()), material(), visible(true), path(""), textureDir(""),
				frontDir(vec3(0, -1, 0)), upDir(vec3(0, 0, 1)), position(vec3(0, 0, 0)), theta(0),
				suppPlane(SuppPlane()), parentPlaneUVH(vec3(0.5,0.5,0)), 
				isInitLoaded(false), isAlreadyPlaced(false), isBvhReady(false), isSelected(false), renderMode(0), 
				isConstranitsExtracted(false), explicitAnchorId(-1), layoutPassScore(0), layoutScore(0), trialNum(0)
			{
                //replace for proper selection rendering;
               // isSelected = rand() % 2;
            }
			MetaModel(const MetaModel &md) { 
				name = md.name; catName = md.catName; id = md.id; transformation = md.transformation; material = md.material; visible = md.visible; path = md.path; textureDir = md.textureDir;
				frontDir = md.frontDir; upDir = md.upDir; position = md.position; theta = md.theta;
				suppPlane = md.suppPlane; parentPlaneUVH = md.parentPlaneUVH;
				isInitLoaded = md.isInitLoaded; isAlreadyPlaced = md.isAlreadyPlaced; isBvhReady = md.isBvhReady;
				renderMode = md.renderMode;  isSelected = md.isSelected; 
				isConstranitsExtracted = md.isConstranitsExtracted; explicitAnchorId = md.explicitAnchorId; layoutPassScore = md.layoutPassScore; layoutScore = md.layoutScore; trialNum = md.trialNum;
			};
			MetaModel &operator = (const MetaModel &md) { 
				name = md.name; catName = md.catName; id = md.id; transformation = md.transformation; 
				material = md.material; visible = md.visible; path = md.path; textureDir = md.textureDir;
				frontDir = md.frontDir; upDir = md.upDir; position = md.position; theta = md.theta;
				suppPlane = md.suppPlane; parentPlaneUVH = md.parentPlaneUVH;
				isInitLoaded = md.isInitLoaded; isAlreadyPlaced = md.isAlreadyPlaced;  isBvhReady = md.isBvhReady;
				isSelected = md.isSelected; renderMode = md.renderMode; 
				isConstranitsExtracted = md.isConstranitsExtracted; explicitAnchorId = md.explicitAnchorId; layoutPassScore = md.layoutPassScore; layoutScore = md.layoutScore; trialNum = md.trialNum; return *this;
			}

			string name;  // file name (hash string) of the model
			string catName; // category of model
			int id;
			mat4 transformation;  // transform from loaded model to current model

			Material material;		
			bool visible;
			string path;
			string textureDir;

			vec3 frontDir;  // transformed dir in current scene
			vec3 upDir;     // transformed dir in current scene
			vec3 position;  // transformed model pos in current scene
			double theta;  // angle between anchor's front to current model's front

			SuppPlane suppPlane;  // transformed model support plane in current scene
			vec3 parentPlaneUVH;

			bool isInitLoaded; // whether the model is loaded at the beginning
			bool isAlreadyPlaced; // whether the model is already placed in the scene
			bool isBvhReady;  // whether the BVH is ready for collision detection

            bool isSelected;
			int renderMode;

			bool isConstranitsExtracted;
			int explicitAnchorId;

			Eigen::VectorXd tempPlacement;
			double layoutPassScore;
			double layoutScore;
			int trialNum; // trialNum for re-location because of collision

			void updateWithTransform(const mat4 &transMat)
			{
				position = transMat*position;				
				frontDir = TransformVector(transMat, frontDir);
				upDir = TransformVector(transMat, upDir);

				frontDir.normalize();
				upDir.normalize();

				suppPlane.tranfrom(transMat);

				transformation = transMat*transformation;

				isBvhReady = false;
			}
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