/***************************************************************************
OBB_estimator.h  - Compute Axis Aligned Bounding Box for mesh
----------------------------------------------------------------------------
Note: 
----------------------------------------------------------------------------
begin                : jun 2007
copyright            : (C) 2007 by Kevin (Kai) Xu - NUDT603
email                : kevin.kai.xu@gmail.com
***************************************************************************/
#pragma once
#include "../utilities/utility.h"
#include "../utilities/mathlib.h"
#include "../utilities/Eigen3x3.h"
#include "qglviewer/qglviewer.h"

#include <algorithm>
#include <vector>
#include <functional>

// AABB Box
//    4------0
//   /|     /|
//  5-|----1 |
//  | 7----|-3
//  |/     |/
//  6------2

//static int 

#define BB_G_FACE		0x01	// 
#define BB_G_3DEDGE		0x02	// 

#define BB_SIMI_DIST	0x01	// use distance in computing OBB similarity
#define BB_SIMI_SHAPE	0x02	// use shape characteristics in computing OBB similarity
#define BB_SIMI_ORIEN	0x04	// use orientation in computing OBB similarity
#define BB_SIMI_ALL		0x07	// use all in computing OBB similarity

class CAABB
{
public:

	typedef std::vector<MathLib::Vector3> PointSet;

	CAABB(void);
	CAABB(const CAABB& obb);
	CAABB(const MathLib::Vector3 &c, const MathLib::Vector3 &s);
	virtual ~CAABB();

	void SetData(const MathLib::Vector3 &c, const MathLib::Vector3 &s);
	void SetDataM(const MathLib::Vector3 &min, const MathLib::Vector3 &max);
	void updateDataAS(void);	// update with axes and center as known
	void updateDataP(void);		// update with positions as known

	void TransScl(double dx, double dy, double dz, double s);
	void Merge(const CAABB &newBox);
	void analyzeAxis(void);
	void Unitize(const MathLib::Vector3 &c, double s);
	int CalcDissimilarity(const CAABB &bb, double dDMax, double &d) const;
	double CalcHausdorffDist(const CAABB &bb) const;
	void CalcAnisotropy(MathLib::Vector3 &c) const;
	void WriteData(FILE *fp);
	void WriteData(std::ofstream &ofs);
	void ReadData(std::ifstream &ifs);
	void ReadData(std::ifstream &ifs, const MathLib::Vector3 &uc, double us);
	double Vol(void) const;
	const MathLib::Vector3& C(void) const;
	const double& S(int i) const;
	const MathLib::Vector3& S(void) const;
	const double& HS(int i) const;
	const MathLib::Vector3& HS(void) const;
	const MathLib::Vector3& V(int i) const;
	MathLib::Vector3& V(int i);
	const std::vector<MathLib::Vector3>& V(void) const;
	std::vector<MathLib::Vector3>& V(void);
	MathLib::Vector3 GetFaceCent(int f) const;
	void Face(int ai, int d, std::vector<int> &fv);
	CAABB& operator=(const CAABB& other);
	const MathLib::Vector3& GetMinV(void) const { return vp[6]; }
	const MathLib::Vector3& GetMaxV(void) const { return vp[0]; }
	MathLib::Vector3 GetBottomCenter(void) const { return MathLib::Vector3(cent[0], cent[1], cent[2] - hsize[2]); };

	void DrawBox(bool b3DEdge, bool bFace, bool bGraph, bool bShowStat, bool bHighL, GLfloat *fColor=NULL) const;

 	bool IsInside(const MathLib::Vector3 &p) const;
 	bool IsIntersect(const CAABB &aabb) const;
	bool IsIntersect(const CAABB &aabb, double epsilon) const;

	//////////////Rui
	double CalcDistance(const CAABB &bb) const;

	bool isInXYRange(const std::vector<double> &rangeVals, double boundTh = 0);

public:
	MathLib::Vector3					cent;		// center
	MathLib::Vector3					size;		// 3 sizes
	MathLib::Vector3					hsize;		// 3 half sizes
	std::vector<MathLib::Vector3>	vp;			// 8 vertices (same order as psBoxV; see Utility.h)
	double						vol;		// volume
	double						dl;			// diagonal length
	double						er;			// edge radius
	int						ca;			// characteristic axis
	double						cas;		// characteristic axis strength
	double						as, ap, al;
	
	static GLuint			s_edl;		// 3d edge (cylinder) display list (static member shared by all instances)
};
