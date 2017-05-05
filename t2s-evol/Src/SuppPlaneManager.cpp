#include "SuppPlaneManager.h"
#include "SuppPlane.h"
#include <QFile>

SuppPlaneManager::SuppPlaneManager()
{
	m_largestPlaneId = -1;
}

SuppPlaneManager::SuppPlaneManager(const QString &s)
	:m_modelNameStr(s)
{
	loadSuppPlane();
	m_largestPlaneId = -1;
}

SuppPlaneManager::SuppPlaneManager(const std::vector<SuppPlane> &suppPlanes)
	:m_suppPlanes(suppPlanes)
{
	m_largestPlaneId = -1;
}

SuppPlaneManager::~SuppPlaneManager()
{
	clearSupportPlanes();
}

void SuppPlaneManager::transformSuppPlanes(const mat4 &transMat)
{
	for (int i = 0; i < m_suppPlanes.size(); i++)
	{
		m_suppPlanes[i].transform(transMat);
	}
}

bool SuppPlaneManager::loadSuppPlane()
{
	QString suppPlaneFilename;
	suppPlaneFilename = "./SceneDB/models_supp/" + m_modelNameStr + ".supp";

	QFile suppFile(suppPlaneFilename);

	QTextStream ifs(&suppFile);

	if (!suppFile.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

	std::vector<vec3> suppCorners;

	int planeID = 0;

	while (!ifs.atEnd())
	{
		suppCorners.clear();
		for (int i = 0; i < 4; i++)
		{
			vec3 corner;
			QString buff;
			ifs >> buff;
			if (buff[0].isDigit() || buff[0]=='-')
			{
				corner.x = buff.toDouble();
				ifs >> corner.y >> corner.z;
			}
			else
				break;

			suppCorners.push_back(corner);
		}

		if (!suppCorners.empty())
		{
			//SuppPlane *p = new SuppPlane(suppCorners, obbAxis);
			SuppPlane p(suppCorners);

			p.id = planeID;
			p.m_sceneMetric = params::inst()->globalSceneUnitScale;
			m_suppPlanes.push_back(p);
			planeID++;
		}
		else
		{
			break;
		}

	}

	suppFile.close();
	return true;
}

void SuppPlaneManager::clearSupportPlanes()
{
	m_suppPlanes.clear();
}

SuppPlane& SuppPlaneManager::getLargestAreaSuppPlane()
{
	if (m_largestPlaneId != -1)
	{
		return m_suppPlanes[m_largestPlaneId];
	}

	double maxArea = -1e6;
	int maxPlaneID = -1;

	if (!m_suppPlanes.empty())
	{
		for (unsigned int i = 0; i < m_suppPlanes.size(); i++)
		{
			double currArea = m_suppPlanes[i].getArea();
			if (currArea >= maxArea)
			{
				maxArea = currArea;
				maxPlaneID = i;
			}
		}

		m_largestPlaneId = maxPlaneID;
		return m_suppPlanes[maxPlaneID];
	}

	else
		return SuppPlane();
}
