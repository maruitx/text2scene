#pragma once
#include "Headers.h"

class SuppPlane
{
public:
	SuppPlane();
	~SuppPlane();

	SuppPlane(const std::vector<vec3> &corners);

	double getZ();
	double getArea() {return m_width*m_length;}

	vec3 getPointByUV(double u, double v);
	vec3 randomSamplePointByUV(const vec3 &uvh, const double xRange, const double yRange, const double zRange);
	vec3 randomGaussSamplePtByUV(const vec3 &uvh, const std::vector<double> &stdDevs);
	vec3 randomSamplePoint(double boundWidth);

	void computeParas();
	void transform(const mat4 &transMat);

	void initGrid();

	double m_length;  // X range
	double m_width; // Y range
	vec3 m_center;
	vec3 m_normal;

	vector<vec3> m_corners;
	vector<vec3> m_axis; // x-axis (u), y-axis (v)

	double m_sceneMetric;
	double m_offset;

	bool m_isInited;
	//std::vector<std::vector<int>> m_planeGrid; // 

	int id;
};

