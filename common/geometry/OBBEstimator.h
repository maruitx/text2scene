/***************************************************************************
OBB_estimator.h  - Compute Oriented Bounding Box for mesh
----------------------------------------------------------------------------
Note: 
----------------------------------------------------------------------------
begin                : jun 2007
copyright            : (C) 2007 by Kevin (Kai) Xu - NUDT603
email                : kevin.kai.xu@gmail.com
***************************************************************************/
#pragma once
#include "../utilities/utility.h"
#include "../utilities/Eigen3x3.h"
#include "OBB.h"
//#include <CGAL/Simple_cartesian.h>
//#include <CGAL/Filtered_kernel.h>
//#include <CGAL/Delaunay_triangulation_3.h>
//#include <CGAL/point_generators_3.h>
//#include <CGAL/copy_n.h>


#include <fstream>
 
class COBBEstimator
{
public:
	//typedef CGAL::Simple_cartesian<double>				SK;
	//typedef CGAL::Filtered_kernel<SK>				FK;
	//struct K : public FK {};
	//typedef K::Point_3								Point_3;
	//typedef K::Triangle_3							Triangle_3;
	//typedef CGAL::Delaunay_triangulation_3<K>		Delaunay;
	//typedef Delaunay::Vertex_handle					Vertex_handle;
	//typedef Delaunay::Cell_handle					Cell_handle;
	typedef std::vector<MathLib::Vector3>					PointSet;

	//class CDF {
	//public:
	//	CDF() {v[0]=v[1]=v[2]=0;};
	//	~CDF() {};
	//	CDF(Vertex_handle v1, Vertex_handle v2, Vertex_handle v3) {v[0]=v1; v[1]=v2; v[2]=v3;};
	//	Vertex_handle v[3];
	//	// Data access using indices
	//	Vertex_handle&       operator[](int i)       { return (v[i]); }
	//	const Vertex_handle& operator[](int i) const { return (v[i]); }
	//};

private:
	// polygonal mesh
	std::vector<const PointSet*>		m_PntSet;
	COBB								*m_pOBB;	// resultant OBB
	//Delaunay							m_DT;
	//std::vector<Vertex_handle>			m_CHV;
	//std::vector<CDF>					m_CHF;
	int m_ptsNum;


public:
	// life cycle
	COBBEstimator();
	COBBEstimator(COBB *pOBB);
	COBBEstimator(const PointSet *pPS, COBB *pOBB);
	~COBBEstimator();

	void ClearPointSet(void);
	void AddPointSet(const PointSet *pPS);
	void SetOBB(COBB *pOBB);

	//int ComputeOBB_CHInf(void);
	int ComputeOBB_PCA(void);
	int ComputeOBB_Min(int iFixAxis=-1);

public:
	//int computeConvexHull(void);
	//int computeOBBFromCH(void);
	int computeMinOBBByRot(int iFixAxis);
};
