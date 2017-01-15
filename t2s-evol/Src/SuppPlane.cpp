#include "SuppPlane.h"


SuppPlane::SuppPlane()
{
	m_corners.resize(4);
	m_normal = m_center = vec3();
	m_length = 0;
	m_width = 0;
	m_axis.resize(2);
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
}

vec3 SuppPlane::getPointByUV(double u, double v)
{
	vec3 pt = m_corners[0] + u*m_axis[0]*m_length  + v*m_axis[1]*m_width;
	return pt; 
}
