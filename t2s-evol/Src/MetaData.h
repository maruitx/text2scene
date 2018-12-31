# pragma once

#include "Headers.h"
#include "Mesh.h"

#include "SuppPlaneManager.h"
#include "SuppPlane.h"
#include <Eigen/Dense>

class MetaModel
	{
		public:
			MetaModel() : name(""), catName(""), id(0), transformation(mat4::identitiy()), material(), visible(true), path(""), textureDir(""),
				frontDir(vec3(0, -1, 0)), upDir(vec3(0, 0, 1)), position(vec3(0, 0, 0)), theta(0),
				parentPlaneUVH(vec3(0.5,0.5,0)),
				isInitLoaded(false), isAlreadyPlaced(false), isSkipped(false), isTransformedByCommand(false), isJustRollbacked(false), isConstraintExtracted(false), isJustReplaced(false),
				isBvhReady(false), zAdjusted(false), isSelected(false), renderMode(0), 
				explicitAnchorId(-1), layoutPassScore(-100), layoutScore(-100), trialNum(0)
			{
                //replace for proper selection rendering;
               // isSelected = rand() % 2;
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
			vec3 position;  // transformed model pos in current scene; in world frame
			double theta;  // angle between anchor's front to current model's front; from -pi to pi

			SuppPlane bbTopPlane;  // transformed model support plane in current scene
			vec3 parentPlaneUVH;  // UV and height w.r.t to the bbTopPlane and height is not normalized
			SuppPlaneManager suppPlaneManager;
			

			bool isInitLoaded; // whether the model is loaded at the beginning
			bool isAlreadyPlaced; // whether the model is already placed in the scene
			bool isSkipped; // is skipped for placement due to collision or relation break after trial number of tests

			bool isTransformedByCommand;

			bool isJustRollbacked; // whether the model is just rollback for placement
			bool isConstraintExtracted;
			bool isJustReplaced; // whether the model is just replaced by another instance

			bool isBvhReady;  // whether the BVH is ready for collision detection
			bool zAdjusted;

            int isSelected;
			int renderMode;

			int explicitAnchorId;

			Eigen::VectorXd tempPlacement;  // temp data used for roll back
			mat4 tempTransform;
			double layoutPassScore;
			double layoutScore;
			int trialNum; // trialNum for re-location because of collision

//			void updateWithTransform(const mat4 &transMat, vec3 anchorFrontDir=vec3(), vec3 sceneUpDir= vec3())
			void updateWithTransform(const mat4 &transMat)
			{
				position = transMat*position;				
				frontDir = TransformVector(transMat, frontDir);
				upDir = TransformVector(transMat, upDir);

				frontDir.normalize();
				upDir.normalize();

				bbTopPlane.transform(transMat);
				suppPlaneManager.transformSuppPlanes(transMat);

				transformation = transMat*transformation;

				//if (anchorFrontDir!=vec3() && sceneUpDir!= vec3())
				//{
				//	theta = GetRotAngleR(anchorFrontDir, frontDir, sceneUpDir);
				//}

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