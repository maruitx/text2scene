#include "OBBEstimator.h"
#include "BestFit.h"

using namespace MathLib;

COBBEstimator::COBBEstimator()
{
	m_pOBB = NULL;
	m_ptsNum = 0;
}

COBBEstimator::COBBEstimator(COBB *pOBB)
{
	m_pOBB = pOBB;
}

COBBEstimator::COBBEstimator(const PointSet *pPS, COBB *pOBB)
{
	m_PntSet.push_back(pPS);
	m_pOBB = pOBB;

	m_ptsNum = pPS->size();
}

COBBEstimator::~COBBEstimator()
{
	// Do NOT delete m_pOBB
}

void COBBEstimator::ClearPointSet(void)
{
	m_PntSet.clear();
	m_ptsNum = 0;
}

void COBBEstimator::AddPointSet(const PointSet *pPS)
{
	m_PntSet.push_back(pPS);
	m_ptsNum += pPS->size();
}

void COBBEstimator::SetOBB(COBB *pOBB)
{
	m_pOBB = pOBB;
}

//// Compute OBB using method: Estimate convex hull first and then compute principal axes
//// using infinite integral over the convex hull polyhedron
//int COBBEstimator::ComputeOBB_CHInf(void)
//{
//	if (computeConvexHull() != 0) {
//		return -1;
//	}
//	if (computeOBBFromCH() != 0) {
//		return -1;
//	}
//	return 0;
//}

int COBBEstimator::ComputeOBB_Min(int iFixAxis/*=-1*/)
{
	//if (computeConvexHull() != 0) {
	//	return -1;
	//}
	if (computeMinOBBByRot(iFixAxis) != 0) {
		return -1;
	}
	return 0;
}

// Compute OBB using method: Perform PCA directly over the point set
int COBBEstimator::ComputeOBB_PCA(void)
{
	Vector3	MeanP(0.0, 0.0, 0.0);
	int iNum(0);
	for (unsigned int i=0; i<m_PntSet.size(); i++) {
		const std::vector<Vector3> &ps = *m_PntSet[i];
		for (unsigned int j=0; j<ps.size(); j++) {
			MeanP += ps[j];
		}
		iNum += ps.size();
	}
	MeanP /= (double)iNum;
	Vector3 v;
	Matrix3 CovMat;
	for (unsigned int i=0; i<m_PntSet.size(); i++) {
		const std::vector<Vector3> &ps = *m_PntSet[i];
		for (unsigned int j=0; j<ps.size(); j++) {
			v = ps[j];
			v -= MeanP;
			CovMat.m[0][0] += v[0]*v[0];
			CovMat.m[0][1] += v[0]*v[1];
			CovMat.m[0][2] += v[0]*v[2];
			CovMat.m[1][0] += CovMat.m[0][1];
			CovMat.m[1][1] += v[1]*v[1];
			CovMat.m[1][2] += v[1]*v[2];
			CovMat.m[2][0] += CovMat.m[0][2];
			CovMat.m[2][1] += CovMat.m[1][2];
			CovMat.m[2][2] += v[2]*v[2];
		}
	}
	CovMat /= (double)iNum;

	// Eigen decomposition
	Vector3	EigenVal;	// eigenvalues
	Matrix3	EigenVec;	// eigenvectors
	CEigen3x3<double>::dsyevq3(CovMat.m, EigenVec.m, EigenVal.v);
	std::vector<Vector3> axis(3);
	Vector3 cent, size;
	axis[0].set(EigenVec.m[0][0], EigenVec.m[1][0], EigenVec.m[2][0]);
	axis[1].set(EigenVec.m[0][1], EigenVec.m[1][1], EigenVec.m[2][1]);
	axis[2].set(EigenVec.m[0][2], EigenVec.m[1][2], EigenVec.m[2][2]);
	axis[0].normalize();
	axis[1].normalize();
	axis[2].normalize();
	cent = MeanP;

	// Compute size
	for (int d=0; d<3; d++) {
		float dNegScl(0.0f), dPosScl(0.0f);
		for (unsigned int i=0; i<m_PntSet.size(); i++) {
			const std::vector<Vector3> &ps = *m_PntSet[i];
			for (unsigned int j=0; j<ps.size(); j++) {
				float scl = axis[d].dot(ps[j]-cent);
				if (scl > 0.0) {
					dPosScl = std::max(dPosScl, scl);
				} else {
					dNegScl = std::max(dNegScl, -scl);
				}
			}
		}
		cent += axis[d]*(0.5*(dPosScl-dNegScl));
		size[d] = (dPosScl+dNegScl);
	}
	m_pOBB->SetData(cent, axis, size);
	return 0;
}

//int COBBEstimator::computeConvexHull()
//{
//	if (m_PntSet.empty()) {
//		return -1;
//	}
//
//	std::vector<Point_3> PL;	// copy point set into a CGAL point list
//	for (unsigned int i=0; i<m_PntSet.size(); i++) {
//		const std::vector<Vector3> &ps = *m_PntSet[i];
//		for (unsigned int j=0; j<ps.size(); j++) {
//			PL.push_back(Point_3(ps[j][0], ps[j][1], ps[j][2]));
//		}
//	}
// 
//	// Build Delaunay triangulation
//	m_DT.insert(PL.begin(), PL.end());
//	m_DT.incident_vertices(m_DT.infinite_vertex(), std::back_inserter(m_CHV));
//	// Pick out all infinite cells
//	std::vector<Cell_handle> InfCell;
//	m_DT.incident_cells(m_DT.infinite_vertex(), std::back_inserter(InfCell));
//	// Pick out convex hull vertices and faces
//	for (unsigned int i=0; i<InfCell.size(); i++) {
//		int iInfId = InfCell[i]->index(m_DT.infinite_vertex());
//		CDF cdf;
//		int iFaceId(0);
//		for (int j=0; j<4; j++) {
//			if (j != iInfId) {
//				if ((cdf[iFaceId++]=InfCell[i]->vertex(j)) == NULL) {
//					break;
//				}
//			}
//		}
//		if (cdf[iFaceId-1] == NULL) {
//			continue;
//		}
//		m_CHF.push_back(cdf);
//	}
//	return 0;
//}

int COBBEstimator::computeMinOBBByRot(int iFixAxis)
{
	if (m_PntSet.empty()) {
		return -1;
	}

	double *points = new double[3 * m_ptsNum];
	int currId = 0;

	for (unsigned int i = 0; i < m_PntSet.size(); i++)
	{
		const std::vector<Vector3> &ps = *m_PntSet[i];
		for (unsigned int j = 0; j < ps.size(); j++) {
			MathLib::Vector3 p = ps[j];
			points[3 * currId + 0] = p[0];
			points[3 * currId + 1] = p[1];
			points[3 * currId + 2] = p[2];

			currId++;
		}
	}


	double sides[3], matrix[16];
	switch (iFixAxis) {
		case -1: case 0: case 1:
			BEST_FIT::computeBestFitOBB(m_ptsNum, points, 3 * sizeof(double), sides, matrix, BEST_FIT::FS_SLOW_FIT);
			break;
		case 2:
			BEST_FIT::computeBestFitOBB_FixZ(m_ptsNum, points, 3 * sizeof(double), sides, matrix, BEST_FIT::FS_SLOW_FIT);
			break;
		default:
			BEST_FIT::computeBestFitOBB(m_ptsNum, points, 3 * sizeof(double), sides, matrix, BEST_FIT::FS_SLOW_FIT);
			break;
	}
	
	m_pOBB->SetData(sides, matrix);
	SAFE_DELETE_ARRAY(points);
	return 0;
}

//int COBBEstimator::computeOBBFromCH(void)
//{
//	if (m_CHV.empty() || m_CHF.empty()) {
//		return -1;
//	}
//
//	Vector3 v1, v2, v3, v;
//	std::vector<double> FaceArea;
//	std::vector<Vector3> FaceCent;
//	double			dHArea(0.0);
//	Vector3		HCent(0.0, 0.0, 0.0);
//	for (unsigned int i=0; i<m_CHF.size(); i++) {
//		const Point_3 &p1 = m_CHF[i][0]->point();
//		const Point_3 &p2 = m_CHF[i][1]->point();
//		const Point_3 &p3 = m_CHF[i][2]->point();
//		v1.set(p1[0], p1[1], p1[2]);
//		v2.set(p2[0], p2[1], p2[2]);
//		v3.set(p3[0], p3[1], p3[2]);
//		double fArea = TriArea(v1, v2, v3);
//		FaceArea.push_back(fArea);
//		dHArea += fArea;
//		FaceCent.push_back((v1+v2+v3)/3.0);
//		HCent += FaceCent.back()*fArea;
//	}
//	HCent /= dHArea;
//
//	Matrix3	CovMat;
//	for (unsigned int i=0; i<m_CHF.size(); i++) {
//		const Point_3 &p1 = m_CHF[i][0]->point();
//		const Point_3 &p2 = m_CHF[i][1]->point();
//		const Point_3 &p3 = m_CHF[i][2]->point();
//		double dF = FaceArea[i] / 12.0;
//		CovMat.m[0][0] += ( 9.0*FaceCent[i][0]*FaceCent[i][0]
//					   + p1[0]*p1[0] + p2[0]*p2[0] + p3[0]*p3[0] ) * dF;
//		CovMat.m[0][1] += ( 9.0*FaceCent[i][0]*FaceCent[i][1]
//					   + p1[0]*p1[1] + p2[0]*p2[1] + p3[0]*p3[1] ) * dF;
//		CovMat.m[0][2] += ( 9.0*FaceCent[i][0]*FaceCent[i][2]
//					   + p1[0]*p1[2] + p2[0]*p2[2] + p3[0]*p3[2] ) * dF;
//		CovMat.m[1][1] += ( 9.0*FaceCent[i][1]*FaceCent[i][1]
//					   + p1[1]*p1[1] + p2[1]*p2[1] + p3[1]*p3[1] ) * dF;
//		CovMat.m[1][2] += ( 9.0*FaceCent[i][1]*FaceCent[i][2]
//					   + p1[1]*p1[2] + p2[1]*p2[2] + p3[1]*p3[2] ) * dF;
//		CovMat.m[2][2] += ( 9.0*FaceCent[i][2]*FaceCent[i][2]
//					   + p1[2]*p1[2] + p2[2]*p2[2] + p3[2]*p3[2] ) * dF;
//	}
//	CovMat.m[0][0] -= HCent[0]*HCent[0] * dHArea;
//	CovMat.m[0][1] -= HCent[0]*HCent[1] * dHArea;
//	CovMat.m[0][2] -= HCent[0]*HCent[2] * dHArea;
//	CovMat.m[1][0] = CovMat.m[0][1];
//	CovMat.m[1][1] -= HCent[1]*HCent[1] * dHArea;
//	CovMat.m[1][2] -= HCent[1]*HCent[2] * dHArea;
//	CovMat.m[2][0] = CovMat.m[0][2];
//	CovMat.m[2][1] = CovMat.m[1][2];
//	CovMat.m[2][2] -= HCent[2]*HCent[2] * dHArea;
//
//	// Eigen decomposition
//	Vector3	EigenVal;	// eigenvalues
//	Matrix3	EigenVec;	// eigenvectors
//	CEigen3x3<double>::dsyevq3(CovMat.m, EigenVec.m, EigenVal.v);
//	std::vector<Vector3> axis(3);
//	Vector3 cent, size;
//	axis[0].set(EigenVec.m[0][0], EigenVec.m[1][0], EigenVec.m[2][0]);
//	axis[1].set(EigenVec.m[0][1], EigenVec.m[1][1], EigenVec.m[2][1]);
//	axis[2].set(EigenVec.m[0][2], EigenVec.m[1][2], EigenVec.m[2][2]);
//	axis[0].normalize();
//	axis[1].normalize();
//	axis[2].normalize();
//	cent = HCent;
//	// Compute extension
//	for (int d=0; d<3; d++) {
//		double dNegScl(0.0), dPosScl(0.0);
//		for (unsigned int i=0; i<m_CHV.size(); i++) {
//			const Point_3 &p = m_CHV[i]->point();
//			v.set(p[0], p[1], p[2]);
//			double scl = axis[d].dot(v-cent);
//			if (scl > 0.0) {
//				dPosScl = Max(dPosScl, scl);
//			} else {
//				dNegScl = Max(dNegScl, -scl);
//			}
//		}
//		size[d] = (dPosScl+dNegScl);
//		cent += axis[d] * ((dPosScl-dNegScl)*0.5);
//	}
//	m_pOBB->SetData(cent, axis, size);
//	// Extract hull vertices
//	PointSet hv(m_CHV.size());
//	for (unsigned int i=0; i<m_CHV.size(); i++) {
//		const Point_3 &p = m_CHV[i]->point();
//		hv[i].set(p[0], p[1], p[2]);
//	}
//	m_pOBB->SetHullVert(hv);
//
//	return 0;
//}