#include "LayoutPlanner.h"
#include "SceneSemGraph.h"
#include "Utility.h"


LayoutPlanner::LayoutPlanner()
{
}


LayoutPlanner::~LayoutPlanner()
{
}

void LayoutPlanner::initPlaceByAlignRelation()
{
	// geometry align of the matched nodes
	for (int mi = 0; mi < m_matchedSg->m_nodeNum; mi++)
	{
		SemNode& m_matchedSgNode = m_matchedSg->m_nodes[mi];
		if (!m_matchedSgNode.isAligned && m_matchedSgNode.nodeType == "pairwise_relationship")
		{
			int mRefNodeId, mActiveNodeId;

			if (!m_matchedSg->findRefNodeForRelationNode(m_matchedSgNode, mRefNodeId, mActiveNodeId))
			{
				break;
			}

			int currRefNodeId = m_matchedSg->m_toNewSgNodeIdMap[mRefNodeId];
			int currActiveNodeId = m_matchedSg->m_toNewSgNodeIdMap[mActiveNodeId];

			// find ref models
			MetaModel &mRefModel = m_matchedSg->getModelWithNodeId(mRefNodeId);
			MetaModel &tarRefModel = m_currSg->getModelWithNodeId(currRefNodeId);

			MetaModel &currActiveModel = m_currSg->getModelWithNodeId(currActiveNodeId);
			MetaModel &mActiveModel = m_matchedSg->getModelWithNodeId(mActiveNodeId);

			// compute transformation matrix based on the ref models
			// initial alignment; align the rotation etc.	
			mat4 alignTransMat = computeTransMat(mRefModel, tarRefModel);

			// find the target position on new ref obj using the U, V w.r.t the original parent
			vec3 mUVH = mActiveModel.parentPlaneUVH;

			vec3 initPositionInScene = currActiveModel.position; // get the pos of model in current scene

			// find the position after initial alignment
			vec3 alignedPosition = TransformPoint(alignTransMat, initPositionInScene); // position after initial alignment

			SuppPlane& tarRefSuppPlane = tarRefModel.suppPlane;
			vec3 targetPosition = tarRefSuppPlane.getPointByUV(mUVH.x, mUVH.y); // position in the current scene, support plane is already transformed
			//for (int ci = 0; ci < 4; ci++)
			//{
			//	qDebug() << QString("corner%1 %2 %3 %4").arg(ci).arg(tarRefSuppPlane.m_corners[ci].x).arg(tarRefSuppPlane.m_corners[ci].y).arg(tarRefSuppPlane.m_corners[ci].z) << "\n";
			//}

			vec3 translationVec = targetPosition - alignedPosition;

			mat4 adjustTransMat;
			adjustTransMat = adjustTransMat.translate(translationVec);

			mat4 finalTransMat = adjustTransMat*alignTransMat;

			// transform active model by initial alignment
			// initial alignment will make the relative orientation between the new active model and the target active model right
			currActiveModel.updateWithTransform(finalTransMat);

			//qDebug() << QString("Preview:%1 Query anchor:%2").arg(m_matchedSg->m_matchListId).arg(toQString(mRefModel.catName));
			//mRefModel.transformation.print();
			//qDebug() << QString("Preview:%1 Current anchor:%2").arg(m_matchedSg->m_matchListId).arg(toQString(tarRefModel.catName));
			//tarRefModel.transformation.print();

			//qDebug() << QString("Preview:%1 Current active:%2").arg(m_matchedSg->m_matchListId).arg(toQString(newActiveModel.catName));
			//qDebug() << QString("alignedPos");
			//alignedPosition.print();
			//qDebug() << QString("targetPos");
			//targetPosition.print();
			//qDebug() << QString("Parent UV:%1 %2").arg(mUVH.x).arg(mUVH.y);
			//
			//qDebug() << QString("TSG-Anchor:%1 USG-Anchor:%2 Current:%3").arg(toQString(mRefModel.catName)).arg(toQString(tarRefModel.catName)).arg(toQString(newActiveModel.catName));
		}
	}
}

mat4 LayoutPlanner::computeTransMat(const MetaModel &fromModel, const MetaModel &toModel)
{
	mat4 rotMat = GetRotationMatrix(fromModel.frontDir, toModel.frontDir);
	mat4 transMat = GetTransformationMat(rotMat, fromModel.position, toModel.position);

	return transMat;
}
