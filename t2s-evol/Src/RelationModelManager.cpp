#include "RelationModelManager.h"


RelationModelManager::RelationModelManager()
{
	loadRelationModels();
}

RelationModelManager::~RelationModelManager()
{

}

void RelationModelManager::loadRelationModels()
{
	QString sceneDBPath = "./SceneDB";
	QString filename = sceneDBPath + "/Group.model";

	QFile inFile(filename);
	QTextStream ifs(&inFile);

	if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) return;

	while (!ifs.atEnd())
	{
		QString currLine = ifs.readLine();
		std::vector<std::string> parts = PartitionString(currLine.toStdString(), ",");
		std::vector<std::string> subParts = PartitionString(parts[0], "_");

		QString relationName = toQString(subParts[0]);
		QString anchorObjName = toQString(subParts[1]);
		GroupRelationModel *newGroupModel = new GroupRelationModel(anchorObjName, relationName);
		newGroupModel->m_numInstance = StringToInt(parts[1]);
		m_groupRelModels[newGroupModel->m_groupKey] = newGroupModel;

		currLine = ifs.readLine();
		if (currLine.contains("occurrence"))
		{
			int occurModelNum = StringToIntegerList(currLine.toStdString(), "occurrence ")[0];
			for (int i=0; i < occurModelNum; i++)
			{
				currLine = ifs.readLine();
				parts = PartitionString(currLine.toStdString(), ",");
				subParts = PartitionString(parts[0], "_");

				OccurrenceModel *newOccurModel = new OccurrenceModel(toQString(subParts[0]), StringToInt(subParts[1]));
				newOccurModel->m_occurProb = StringToFloat(parts[1]);

				newGroupModel->m_occurModels[newOccurModel->m_occurKey] = newOccurModel;
			}
		}

		currLine = ifs.readLine();
		if (currLine.contains("pairwise"))
		{
			int pairModelNum = StringToIntegerList(currLine.toStdString(), "pairwise ")[0];
			
			for (int i=0; i < pairModelNum; i++)
			{
				currLine = ifs.readLine();   // relation key
				parts = PartitionString(currLine.toStdString(), ",");
				subParts = PartitionString(parts[1], "_");

				QString anchorObjName = toQString(subParts[0]);
				QString actObjName = toQString(subParts[1]);
				QString conditionName = toQString(subParts[2]);
				QString relationName = toQString(subParts[3]);

				PairwiseRelationModel *newRelModel = new PairwiseRelationModel(anchorObjName, actObjName, conditionName, relationName);
				newGroupModel->m_pairwiseModels[newRelModel->m_relationKey] = newRelModel;

				currLine = ifs.readLine();   // gaussian num
				std::vector<int> intList = StringToIntegerList(currLine.toStdString(), "");
				int gaussNum = intList[0];
				int instanceNum = intList[1];

				newRelModel->m_numGauss = gaussNum;
				newRelModel->m_numInstance = instanceNum;

				if (gaussNum == 0)
				{
					currLine = ifs.readLine();
					parts = PartitionString(currLine.toStdString(), ",");

					newRelModel->m_instances.resize(instanceNum);

					for (int t= 0 ; t <instanceNum; t++)
					{
						// observations
						std::vector<float> floatList = StringToFloatList(parts[t], "");
						RelativePos *newRelPos = new RelativePos();
						newRelPos->pos = vec3(floatList[0], floatList[1], floatList[2]);
						newRelPos->theta = floatList[3];
						newRelPos->m_anchorObjName = anchorObjName;
						newRelPos->m_actObjName = actObjName;
						newRelPos->m_conditionName = conditionName;
						
						newRelModel->m_instances[t] = newRelPos;
					}
				}
				else
				{
					newRelModel->m_gaussians.resize(gaussNum);

					for (int g = 0; g < gaussNum; g++)
					{
						currLine = ifs.readLine();

						parts = PartitionString(currLine.toStdString(), ",");
						int gaussDim = StringToInt(parts[0]);
						double gaussWeight = StringToFloat(parts[1]);

						GaussianModel newGauss;
						newGauss.dim = gaussDim;
						newGauss.weight = gaussWeight;
						newGauss.mean = Eigen::VectorXd(gaussDim);
						newGauss.covarMat = Eigen::MatrixXd(gaussDim, gaussDim);

						std::vector<float> floatList = StringToFloatList(parts[2], "");
						
						for (int d=0; d<gaussDim; d++)
						{
							newGauss.mean[d] = floatList[d];   // have to set the size before you can use the comma initializer.
						}

						floatList = StringToFloatList(parts[3], "");
						for (int c = 0; c < gaussDim; c++)
						{
							for (int r = 0; r < gaussDim; r++)
							{
								newGauss.covarMat(r, c) = floatList[c*gaussDim +r];
							}
							 
						}										
						//newGauss.covarMat.transposeInPlace(); //  comma initializer in Eigen is row-wise
						newRelModel->m_gaussians[g] = newGauss;


					}
				}
			} 
		}
	}
}

