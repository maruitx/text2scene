#include "SuppPlane.h"

const double GridSize = 0.05;   // 5cm, NEED TO CONSIDER SCENE METRIC!!
const double OffSet = 0.005;  // offset to avoid collision 


SuppPlane::SuppPlane(const SuppPlane &p)
	: m_corners(p.m_corners), m_axis(p.m_axis), m_length(p.m_length), m_width(p.m_width), m_normal(p.m_normal), m_isInited(p.m_isInited), m_offset(p.m_offset)
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

	m_offset = OffSet / m_sceneMetric;
}

vec3 SuppPlane::getPointByUV(double u, double v)
{
	vec3 pt = m_corners[0] + u*m_axis[0]*m_length  + v*m_axis[1]*m_width + m_offset*m_normal;
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

vec3 SuppPlane::randomSamplePointByUV(const vec3 &uvh, const double xRange, const double yRange, const double zRange)
{
	double u = uvh.x;
	double v = uvh.y;

	vec3 sampleCenter = getPointByUV(u, v);

	double uVar, vVar, zVar;
	uVar = xRange / m_sceneMetric;
	vVar = yRange / m_sceneMetric;
	zVar = zRange / m_sceneMetric;

	std::vector<double> shiftVals(3);
	GenNRandomDouble(-1, 1, shiftVals);

	vec3 sampledPos = sampleCenter + vec3(shiftVals[0]*uVar, shiftVals[1]*vVar, shiftVals[2]*zVar);
	return sampledPos;
}

vec3 SuppPlane::randomGaussSamplePtByUV(const vec3 &uvh, const std::vector<double> &stdDevs)
{
	double u = uvh.x;
	double v = uvh.y;

	vec3 sampleCenter = getPointByUV(u, v);
	std::vector<double> dMeans(2, 0);

	std::vector<double> shiftVals;
	GenNNormalDistribution(dMeans, stdDevs, shiftVals);

	vec3 sampledPos = sampleCenter + vec3(shiftVals[0] / m_sceneMetric, shiftVals[1] / m_sceneMetric, 0);
	return sampledPos;
}

vec3 SuppPlane::randomSamplePoint(double boundWidth)
{
	std::vector<double> newUV(2);
	GenNRandomDouble(-1+boundWidth, 1-boundWidth, newUV);

	vec3 sampledPos = getPointByUV(newUV[0], newUV[1]);
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