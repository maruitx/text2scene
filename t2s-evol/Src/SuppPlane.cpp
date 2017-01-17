#include "SuppPlane.h"

const double GridSize = 0.05;   // 5cm, NEED TO CONSIDER SCENE METRIC!!


SuppPlane::SuppPlane(const SuppPlane &p)
	: m_corners(p.m_corners), m_axis(p.m_axis), m_length(p.m_length), m_width(p.m_width), m_normal(p.m_normal), m_isInited(p.m_isInited)
{

}


SuppPlane::SuppPlane()
{
	m_corners.resize(4);
	m_normal = m_center = vec3();
	m_length = 0;
	m_width = 0;
	m_axis.resize(2);

	m_isInited = false;
}

SuppPlane::SuppPlane(const std::vector<vec3> &corners)
{
	m_corners.resize(4);
	m_axis.resize(2);

	m_corners[0] = corners[0];
	m_corners[1] = corners[1];
	m_corners[2] = corners[2];
	m_corners[3] = corners[3];


	computeParas();
	initGrid();
}


SuppPlane::~SuppPlane()
{
}

void SuppPlane::computeParas()
{
	m_center = (m_corners[0] + m_corners[1] + m_corners[2] + m_corners[3])*0.25;
	m_length = (m_corners[1] - m_corners[0]).length();
	m_width = (m_corners[2] - m_corners[1]).length();

	m_axis[0] = (m_corners[1] - m_corners[0]);
	m_axis[1] = (m_corners[3] - m_corners[0]);

	m_axis[0].normalize();
	m_axis[1].normalize();

	m_normal = m_axis[0].cross(m_axis[1]);

	m_sceneMetric = params::inst()->globalSceneUnitScale;

	m_isInited = true;
}

vec3 SuppPlane::getPointByUV(double u, double v)
{
	vec3 pt = m_corners[0] + u*m_axis[0]*m_length  + v*m_axis[1]*m_width;
	return pt; 
}

void SuppPlane::tranfrom(const mat4 &transMat)
{
	if (m_corners.empty())
	{
		return;
	}

	for (int i = 0; i < m_corners.size(); i++)
	{
		m_corners[i] = TransformPoint(transMat, m_corners[i]);
	}

	computeParas();
}

vec3 SuppPlane::randomSamplePointByUVH(const vec3 &uvh)
{
	double u = uvh.x;
	double v = uvh.y;

	vec3 sampleCenter = getPointByUV(u, v);
	double uVar, vVar, zVar;
	uVar = 0.15 / m_sceneMetric;
	vVar = 0.1 / m_sceneMetric;
	zVar = 0.05 / m_sceneMetric;

	//vec3 sft = GenShiftWithNormalDistribution(uVar, vVar, 0);
	std::vector<double> sft(3);
	GenNRandomDouble(-1, 1, sft);

	vec3 sampledPos = sampleCenter + vec3(sft[0]*uVar, sft[1]*vVar, sft[2]*zVar);

	return sampledPos;
}

void SuppPlane::initGrid()
{
	m_planeGrid.clear();

	int xGridNum, yGridNum;

	xGridNum = (int)(m_length / (GridSize / m_sceneMetric));
	yGridNum = (int)(m_width / (GridSize / m_sceneMetric));

	m_planeGrid.resize(yGridNum, std::vector<int>(xGridNum, 0));

}