#pragma once
#include "Headers.h"

#include "SuppPlane.h"

class SuppPlaneManager
{
public:
	SuppPlaneManager();
	SuppPlaneManager(const QString &s);

	SuppPlaneManager(const std::vector<SuppPlane> &suppPlanes);
	~SuppPlaneManager();

	void addSupportPlane(const SuppPlane &p) { m_suppPlanes.push_back(p); };
	void clearSupportPlanes();

	std::vector<SuppPlane>& getAllSuppPlanes() { return m_suppPlanes; };
	SuppPlane& getSuppPlane(int id) { return m_suppPlanes[id]; };
	SuppPlane& getLargestAreaSuppPlane();

	bool hasSuppPlane() { return !m_suppPlanes.empty(); };
	int getSuppPlaneNum() { return m_suppPlanes.size(); };

	// transformation
	void transformSuppPlanes(const mat4 &transMat);

	bool loadSuppPlane();

public:
	QString m_modelNameStr;
	std::vector<SuppPlane> m_suppPlanes;
	vec3 m_upRightVec;
	int m_largestPlaneId;
};

