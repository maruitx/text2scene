#include "OBB.h"
#include "TriTriIntersect.h"
#include <fstream>
#include <Set>

#include "OBBEstimator.h"

GLuint COBB::s_edl = 0;

static bool CAgreater(const std::pair<double, std::pair<int, int>> &ca1, const std::pair<double, std::pair<int, int>> &ca2) { return ca1.first > ca2.first; }

COBB::COBB(void)
{
	axis.resize(3);
	vp.resize(8);
	ca = 0;
	sr = 0.0;
	dl = vol = cas = 0.0;
	er = 0.02;
	as = ap = al = 0.0;
}

COBB::COBB(const COBB& obb)
{
	*this = obb;
}

COBB::COBB(const MathLib::Vector3 &c, const std::vector<MathLib::Vector3> &a, const MathLib::Vector3 &s)
{
	axis.resize(3);
	vp.resize(8);

	SetData(c, a, s);
}

COBB::COBB(const std::vector<MathLib::Vector3> &corners)
{
	axis.resize(3);
	vp.resize(8);

	for (int i = 0; i < 8; i++)
	{
		vp[i] = corners[i];
	}

	updateDataP();
}

COBB::~COBB()
{
}

void COBB::Unitize(const MathLib::Vector3 &c, double s)
{
	for (unsigned int i = 0; i < 8; i++) {
		vp[i] -= c;
		vp[i] *= s;
	}
	updateDataP();
}

void COBB::SetAAData(const MathLib::Vector3 &c, const MathLib::Vector3 &s)
{
	// Axis-aligned data
	cent = c;
	axis[0].set(1.0, 0.0, 0.0);
	axis[1].set(0.0, 1.0, 0.0);
	axis[2].set(0.0, 0.0, 1.0);
	size = s;
	updateDataAS();
}

void COBB::SetData(double *sides, double *matrix)
{
	if (sides == NULL || matrix == NULL) {
		return;
	}

	size.set(sides[0], sides[1], sides[2]);
	cent.set(matrix[12], matrix[13], matrix[14]);
	axis[0].set(matrix[0], matrix[1], matrix[2]);
	axis[1].set(matrix[4], matrix[5], matrix[6]);
	axis[2].set(matrix[8], matrix[9], matrix[10]);
	updateDataAS();
}

void COBB::SetData(const MathLib::Vector3 &c, const std::vector<MathLib::Vector3> &a, const MathLib::Vector3 &s)
{
	if (a.size() != 3) {
		return;
	}

	cent = c;
	for (int i = 0; i < 2; i++) {
		axis[i] = a[i];
		axis[i].normalize();
	}
	axis[2] = axis[0].cross(axis[1]);
	size = s;
	updateDataAS();
}

void COBB::SetData(const MathLib::Vector3 &c, const MathLib::Vector3 &e1, const MathLib::Vector3 &e2, const MathLib::Vector3 &e3)
{
	cent = c;
	size[0] = e1.magnitude();
	size[1] = e2.magnitude();
	size[2] = e3.magnitude();
	if (size[0] == 0 || size[1] == 0 || size[2] == 0) {
		return;
	}
	axis[0] = e1 / size[0];
	axis[1] = e2 / size[1];
	axis[2] = axis[0].cross(axis[1]);
	updateDataAS();
}

void COBB::Transform(const MathLib::Matrix4d &m)
{
	for (unsigned int i = 0; i < 8; i++) {
		vp[i] = m.transform(vp[i]);
	}
	updateDataP();
}

// Update with positions as known
void COBB::updateDataP(void)
{
	axis[0] = vp[2] - vp[6];
	size[0] = axis[0].magnitude();
	if (!MathLib::IsZero(size[0])) {
		axis[0] /= size[0];
	}
	axis[1] = vp[7] - vp[6];
	size[1] = axis[1].magnitude();
	if (!MathLib::IsZero(size[1])) {
		axis[1] /= size[1];
	}
	axis[2] = vp[5] - vp[6];
	size[2] = axis[2].magnitude();
	if (!MathLib::IsZero(size[2])) {
		axis[2] /= size[2];
	}
	// guarantee orthogonal
	axis[2] = axis[0].cross(axis[1]);
	axis[1] = axis[2].cross(axis[0]);
	cent.setzero();
	for (int i = 0; i < 8; i++) {
		cent += vp[i];
	}
	cent /= 8.0;
	hsize[0] = 0.5*size[0];
	hsize[1] = 0.5*size[1];
	hsize[2] = 0.5*size[2];
	vol = size[0] * size[1] * size[2];
	dl = std::sqrt(size[0] * size[0] + size[1] * size[1] + size[2] * size[2]);
}

// Update with axes and center as known
// ??? : better not perform re-ordering... problems would come
void COBB::updateDataAS(void)
{
	// Update the rest stuff
	hsize[0] = 0.5*size[0];
	hsize[1] = 0.5*size[1];
	hsize[2] = 0.5*size[2];
	vp[0] = cent + axis[0] * hsize[0] + axis[1] * hsize[1] + axis[2] * hsize[2];
	vp[1] = cent + axis[0] * hsize[0] - axis[1] * hsize[1] + axis[2] * hsize[2];
	vp[2] = cent + axis[0] * hsize[0] - axis[1] * hsize[1] - axis[2] * hsize[2];
	vp[3] = cent + axis[0] * hsize[0] + axis[1] * hsize[1] - axis[2] * hsize[2];
	vp[4] = cent - axis[0] * hsize[0] + axis[1] * hsize[1] + axis[2] * hsize[2];
	vp[5] = cent - axis[0] * hsize[0] - axis[1] * hsize[1] + axis[2] * hsize[2];
	vp[6] = cent - axis[0] * hsize[0] - axis[1] * hsize[1] - axis[2] * hsize[2];
	vp[7] = cent - axis[0] * hsize[0] + axis[1] * hsize[1] - axis[2] * hsize[2];
	vol = size[0] * size[1] * size[2];
	dl = std::sqrt(size[0] * size[0] + size[1] * size[1] + size[2] * size[2]);
}

void COBB::TransScl(double dx, double dy, double dz, double s)
{
	for (unsigned int j = 0; j < 8; j++) {
		vp[j][0] += dx;
		vp[j][1] += dy;
		vp[j][2] += dz;
		vp[j][0] *= s;
		vp[j][1] *= s;
		vp[j][2] *= s;
	}
	// inversely computing the cent
	cent.set(0.0, 0.0, 0.0);
	for (unsigned int j = 0; j < 8; j++) {
		cent += vp[j];
	}
	cent /= 8.0f;
	size *= s;
	updateDataAS();
}

void COBB::Blend(const COBB &obb) {
	cent = (obb.cent + cent);
	cent *= 0.5;
	int i0 = obb.GetClosestAxis(axis[0]);
	if (obb.axis[i0].dot(axis[0]) > 0) {
		axis[0] = (obb.axis[i0] + axis[0]);
	}
	else {
		axis[0] = (obb.axis[i0] - axis[0]);
	}
	axis[0].normalize();
	int i1 = obb.GetClosestAxis(axis[1]);
	if (obb.axis[i1].dot(axis[1]) > 0) {
		axis[1] = (obb.axis[i1] + axis[1]);
	}
	else {
		axis[1] = (obb.axis[i1] - axis[1]);
	}
	axis[1] = axis[1] - axis[0] * (axis[0].dot(axis[1]));
	axis[1].normalize();
	axis[2] = axis[0].cross(axis[1]);
	axis[2].normalize();
	int i2 = 3 - i0 - i1;
	size[0] = (obb.size[i0] + size[0]);
	size[1] = (obb.size[i1] + size[1]);
	size[2] = (obb.size[i2] + size[2]);
	size *= 0.5;
	updateDataAS();
}

void COBB::analyzeAxis(void) {
	// Analyze characteristic axis
	// ??? TODO: deal with un-re-ordered axes
	al = (size[0] - size[1]) / (size[0] + size[1] + size[2]);		// linear
	ap = 2.0f*(size[1] - size[2]) / (size[0] + size[1] + size[2]);	// planar
	as = 3.0f*size[2] / (size[0] + size[1] + size[2]);			// spherical

	ca = (al >= ap) ? 0 : 2;
	cas = (al >= ap) ? al : ap;
}

int COBB::GetClosestAxis(const MathLib::Vector3 &v) const {
	MathLib::Vector3 vec(v);
	vec.normalize();
	double dMaxCosA(0.0);
	int iMAId(-1);
	for (int i = 0; i<3; i++) {
		double dCosA = MathLib::Abs(axis[i].dot(vec));
		if (dCosA > dMaxCosA) {
			dMaxCosA = dCosA;
			iMAId = i;
		}
	}
	return iMAId;
}

MathLib::Vector3 COBB::GetFaceCent(int f) const
{
	if (f < 0 || f >= boxNumQuadFace) {
		Simple_Message_Box(QString("COBB::GetFaceCent: Invaid face id!"));
		return MathLib::Vector3(0, 0, 0);
	}
	MathLib::Vector3 cent;
	for (int i = 0; i < 4; i++) {
		cent += vp[boxQuadFace[f][i]];
	}
	return cent / 4.0;
}

void COBB::CalcAxisMatchMap(const COBB &bb, std::vector<int> &m) const
{
	/*m.resize(3);
	m[0] = bb.GetClosestAxis(axis[0]);
	m[1] = bb.GetClosestAxis(axis[1]);
	m[2] = 3-m[0]-m[1];*/
	typedef std::pair<int, int> INTP;
	std::vector<std::pair<double, INTP>> MaxCosA(3);
	for (int i = 0; i < 3; i++) {
		MaxCosA[i].first = 0.0;
		for (int j = 0; j<3; j++) {
			double dCosA = MathLib::Abs(axis[i].dot(bb.axis[j]));
			if (dCosA > MaxCosA[i].first) {
				MaxCosA[i].first = dCosA;
				MaxCosA[i].second.first = i;
				MaxCosA[i].second.second = j;
			}
		}
	}
	m.resize(3);
	std::sort(MaxCosA.begin(), MaxCosA.end(), CAgreater);
	int Mi1 = MaxCosA[0].second.first;
	int Mj1 = MaxCosA[0].second.second;
	m[Mi1] = Mj1;
	int Mi2 = MaxCosA[1].second.first;
	int Mj2 = MaxCosA[1].second.second;
	m[Mi2] = Mj2;
	int Mi3 = 3 - Mi1 - Mi2;
	int Mj3 = 3 - Mj1 - Mj2;
	m[Mi3] = Mj3;
}

void COBB::MatchAxis(const COBB &bb)
{
	typedef std::pair<int, int> INTP;
	std::vector<std::pair<double, INTP>> MaxCosA(3);
	for (int i = 0; i < 3; i++) {
		MaxCosA[i].first = 0.0;
		for (int j = 0; j<3; j++) {
			double dCosA = MathLib::Abs(axis[i].dot(bb.axis[j]));
			if (dCosA > MaxCosA[i].first) {
				MaxCosA[i].first = dCosA;
				MaxCosA[i].second.first = i;
				MaxCosA[i].second.second = j;
			}
		}
	}
	std::vector<MathLib::Vector3> taxis(3);
	MathLib::Vector3 tsize;
	std::sort(MaxCosA.begin(), MaxCosA.end(), CAgreater);
	int Mi1 = MaxCosA[0].second.first;
	int Mj1 = MaxCosA[0].second.second;
	taxis[Mj1] = (axis[Mi1].dot(bb.axis[Mj1]) > 0.0) ? axis[Mi1] : -axis[Mi1];
	tsize[Mj1] = size[Mi1];
	int Mi2 = MaxCosA[1].second.first;
	int Mj2 = MaxCosA[1].second.second;
	taxis[Mj2] = (axis[Mi2].dot(bb.axis[Mj2]) > 0.0) ? axis[Mi2] : -axis[Mi2];
	tsize[Mj2] = size[Mi2];
	int Mi3 = 3 - Mi1 - Mi2;
	int Mj3 = 3 - Mj1 - Mj2;
	taxis[Mj3] = (axis[Mi3].dot(bb.axis[Mj3]) > 0.0) ? axis[Mi3] : -axis[Mi3];
	tsize[Mj3] = size[Mi3];
	axis = taxis;
	size = tsize;
	updateDataAS();
}

void COBB::CalcAxisMatchBB(const COBB &bbr, COBB &bbo, std::vector<int> &m) const
{
	typedef std::pair<int, int> INTP;
	std::vector<std::pair<double, INTP>> MaxCosA(3);
	for (int i = 0; i < 3; i++) {
		MaxCosA[i].first = 0.0;
		for (int j = 0; j<3; j++) {
			double dCosA = MathLib::Abs(axis[i].dot(bbr.axis[j]));
			if (dCosA > MaxCosA[i].first) {
				MaxCosA[i].first = dCosA;
				MaxCosA[i].second.first = i;
				MaxCosA[i].second.second = j;
			}
		}
	}
	bbo = *this;
	std::vector<MathLib::Vector3> &taxis = bbo.axis;
	MathLib::Vector3 &tsize = bbo.size;
	m.resize(3);
	std::sort(MaxCosA.begin(), MaxCosA.end(), CAgreater);
	int Mi1 = MaxCosA[0].second.first;
	int Mj1 = MaxCosA[0].second.second;
	taxis[Mj1] = (axis[Mi1].dot(bbr.axis[Mj1]) > 0.0) ? axis[Mi1] : -axis[Mi1];
	tsize[Mj1] = size[Mi1];
	m[Mi1] = Mj1;
	int Mi2 = MaxCosA[1].second.first;
	int Mj2 = MaxCosA[1].second.second;
	taxis[Mj2] = (axis[Mi2].dot(bbr.axis[Mj2]) > 0.0) ? axis[Mi2] : -axis[Mi2];
	tsize[Mj2] = size[Mi2];
	m[Mi2] = Mj2;
	int Mi3 = 3 - Mi1 - Mi2;
	int Mj3 = 3 - Mj1 - Mj2;
	taxis[Mj3] = (axis[Mi3].dot(bbr.axis[Mj3]) > 0.0) ? axis[Mi3] : -axis[Mi3];
	tsize[Mj3] = size[Mi3];
	m[Mi3] = Mj3;
	bbo.updateDataAS();
}

void COBB::Anisotropy(MathLib::Vector3 &c) const
{
	std::vector<double> s(3);
	s[0] = size[0];
	s[1] = size[1];
	s[2] = size[2];
	std::sort(s.begin(), s.end(), std::greater<double>());
	double sum = s[0] + s[1] + s[2];
	c[0] = (s[0] - s[1]) / sum;
	c[1] = 2.0*(s[1] - s[2]) / sum;
	c[2] = 3.0*s[2] / sum;
}

void COBB::GetLongestAxis(MathLib::Vector3 &a) const
{
	double dMaxS = std::max(size[0], std::max(size[1], size[2]));
	if (dMaxS == size[0]) {
		a = axis[0];
	}
	else if (dMaxS == size[1]) {
		a = axis[1];
	}
	else {
		a = axis[2];
	}
}

void COBB::GetShortestAxis(MathLib::Vector3 &a) const
{
	double dMinS = std::min(size[0], std::min(size[1], size[2]));
	if (dMinS == size[0]) {
		a = axis[0];
	}
	else if (dMinS == size[1]) {
		a = axis[1];
	}
	else {
		a = axis[2];
	}
}

void COBB::GetMidLenAxis(MathLib::Vector3 &a) const
{
	double dMaxS = std::max(size[0], std::max(size[1], size[2]));
	double dMinS = std::min(size[0], std::min(size[1], size[2]));

	if (dMaxS != size[0] && dMinS != size[0]) {
		a = axis[0];
	}
	else if (dMaxS != size[1] && dMinS != size[1]) {
		a = axis[1];
	}
	else {
		a = axis[2];
	}
}



//int COBB::CalcDissimilarity(const COBB &bb, double &d, double dDMax/*=0.0*/, char flags/*=BB_SIMI_ALL*/) const
//{
//	d = 0.0;
//	double wd(10.0), ws(4.0), wo(0.0), wsum(0.0);
//	if (flags&BB_SIMI_DIST && dDMax>0.0) {
//		d += wd * cent.distance(bb.cent)/dDMax;
//		wsum += wd;
//	}
//	if (flags&BB_SIMI_SHAPE) {
//		MathLib::Vector3 c1, c2;
//		CalcAnisotropy(c1);
//		bb.CalcAnisotropy(c2);
//		d += ws * c1.distance(c2)/1.4142135623731;
//		wsum += ws;
//	}
//	if (wsum != 0.0) {
//		d /= wsum;
//		return 0;
//	} else {
//		return -1;
//	}
//}

double COBB::HausdorffDist_Proj(const COBB &bb, const MathLib::Vector3 &dir) const
{
	MathLib::Vector3 pd = dir;
	pd.normalize();
	COBB pbb1, pbb2;
	for (unsigned int i = 0; i < vp.size(); i++) {
		pbb1.vp[i] = vp[i] - pd.dot(vp[i]);
	}
	for (unsigned int i = 0; i < bb.vp.size(); i++) {
		pbb2.vp[i] = bb.vp[i] - pd.dot(bb.vp[i]);
	}
	return pbb1.HausdorffDist(pbb2);
}

double COBB::HausdorffDist(const COBB &bb) const
{
	double d1(0);
	for (unsigned int i = 0; i < vp.size(); i++) {
		double dd(std::numeric_limits<double>::max());
		for (unsigned int j = 0; j < bb.vp.size(); j++) {
			dd = std::min(dd, vp[i].distance(bb.vp[j]));
		}
		d1 = std::max(d1, dd);
	}
	double d2(0);
	for (unsigned int i = 0; i < bb.vp.size(); i++) {
		double dd(std::numeric_limits<double>::max());
		for (unsigned int j = 0; j < vp.size(); j++) {
			dd = std::min(dd, bb.vp[i].distance(vp[j]));
		}
		d2 = std::max(d2, dd);
	}
	return std::max(d1, d2);
}

int COBB::Dissimilarity(const COBB &bb, double dDMax, double &d) const
{
	d = HausdorffDist(bb) / dDMax;
	//	double wd(1.0), ws(0.0);
	//	double dd = CalcHausdorffDist(bb)/dDMax;//cent.distance(bb.cent)/dDMax;

	/*MathLib::Vector3 c1, c2;
	CalcAnisotropy(c1);
	bb.CalcAnisotropy(c2);
	MathLib::Vector3 la1, la2, sa1, sa2;
	GetLongestAxis(la1);
	bb.GetLongestAxis(la2);
	GetShortestAxis(sa1);
	bb.GetShortestAxis(sa2);

	double dcl = (c1[0]-c2[0])*(c1[0]-c2[0]);
	double dcp = (c1[1]-c2[1])*(c1[1]-c2[1]);
	double dl1 = (1.0-c1[0])*(1.0-c1[0]);
	double dl2 = (1.0-c2[0])*(1.0-c2[0]);
	double dp1 = (1.0-c1[1])*(1.0-c1[1]);
	double dp2 = (1.0-c2[1])*(1.0-c2[1]);
	double s = 1.0/(0.2*0.2*0.6*0.6);
	double dsl = MathLib::Acos(MathLib::Abs(la1.dot(la2)))/90.0 * (1.0-exp(-(dcl*0.5+dl1*0.25+dl2*0.25)*s));
	double dsp = MathLib::Acos(MathLib::Abs(sa1.dot(sa2)))/90.0 * (1.0-exp(-(dcp*0.5+dp1*0.25+dp2*0.25)*s));
	double ds = std::min(dsl,dsp);*/

	//	d = dd*wd + ds*ws;
	return 0;
}

void COBB::SetHullVert(const PointSet &ps)
{
	chv.resize(ps.size());
	std::copy(ps.begin(), ps.end(), chv.begin());
}

void COBB::WriteData(FILE *fp)
{
	fprintf(fp, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
		cent[0], cent[1], cent[2],
		axis[0][0], axis[0][1], axis[0][2],
		axis[1][0], axis[1][1], axis[1][2],
		axis[2][0], axis[2][1], axis[2][2],
		size[0], size[1], size[2]);
}

void COBB::WriteData(std::ofstream &ofs)
{
	ofs << cent[0] << " " << cent[1] << " " << cent[2] << " "
		<< axis[0][0] << " " << axis[0][1] << " " << axis[0][2] << " "
		<< axis[1][0] << " " << axis[1][1] << " " << axis[1][2] << " "
		<< axis[2][0] << " " << axis[2][1] << " " << axis[2][2] << " "
		<< size[0] << " " << size[1] << " " << size[2] << std::endl;
}

void COBB::ReadData(std::ifstream &ifs)
{
	ifs >> cent[0] >> cent[1] >> cent[2]
		>> axis[0][0] >> axis[0][1] >> axis[0][2]
		>> axis[1][0] >> axis[1][1] >> axis[1][2]
		>> axis[2][0] >> axis[2][1] >> axis[2][2]
		>> size[0] >> size[1] >> size[2];
	updateDataAS();
}

void COBB::ReadData(std::ifstream &ifs, const MathLib::Vector3 &uc, double us)
{
	ifs >> cent[0] >> cent[1] >> cent[2]
		>> axis[0][0] >> axis[0][1] >> axis[0][2]
		>> axis[1][0] >> axis[1][1] >> axis[1][2]
		>> axis[2][0] >> axis[2][1] >> axis[2][2]
		>> size[0] >> size[1] >> size[2];
	updateDataAS();
	Unitize(uc, us);
}

void COBB::WriteData(FILE *fp, MathLib::Matrix4d &TM)
{
	std::vector<MathLib::Vector3> rvp(8);
	for (unsigned int j = 0; j < 8; j++) {
		rvp[j] = TM.transform(vp[j]);
	}
	// inversely computing the cent
	MathLib::Vector3 rc(0.0, 0.0, 0.0);
	for (unsigned int j = 0; j < 8; j++) {
		rc += rvp[j];
	}
	rc /= 8.0f;
	MathLib::Vector3 ra0 = rvp[0] - rvp[4];
	ra0.normalize();
	MathLib::Vector3 ra1 = rvp[0] - rvp[1];
	ra1.normalize();
	MathLib::Vector3 ra2 = rvp[0] - rvp[3];
	ra2.normalize();
	fprintf(fp, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
		rc[0], rc[1], rc[2],
		ra0[0], ra0[1], ra0[2],
		ra1[0], ra1[1], ra1[2],
		ra2[0], ra2[1], ra2[2],
		size[0], size[1], size[2]);
}

double COBB::Vol(void) const
{
	return vol;
}

const MathLib::Vector3& COBB::C(void) const
{
	return cent;
}

const COBB::PointSet& COBB::HV(void) const
{
	return chv;
}

const COBB::PointSet* COBB::HVPointer(void) const
{
	return &chv;
}

const MathLib::Vector3& COBB::A(int i) const
{
	return axis[i];
}

const std::vector<MathLib::Vector3>& COBB::A() const
{
	return axis;
}

const double& COBB::S(int i) const
{
	return size[i];
}
const MathLib::Vector3& COBB::S(void) const
{
	return size;
}
const double& COBB::HS(int i) const
{
	return hsize[i];
}
const MathLib::Vector3& COBB::HS(void) const
{
	return hsize;
}

const MathLib::Vector3& COBB::V(int i) const
{
	return vp[i];
}

const std::vector<MathLib::Vector3>& COBB::V(void) const
{
	return vp;
}

std::vector<MathLib::Vector3>& COBB::V(void)
{
	return vp;
}

MathLib::Vector3& COBB::V(int i)
{
	return vp[i];
}

void COBB::Face(int ai, int d, std::vector<int> &fv)
{
	fv.resize(4, 0);
	switch (ai) {
	case 0:
		if (d >= 0) {
			fv[0] = 0;
			fv[1] = 1;
			fv[2] = 2;
			fv[3] = 3;
		}
		else {
			fv[0] = 4;
			fv[1] = 7;
			fv[2] = 6;
			fv[3] = 5;
		}
		break;
	case 1:
		if (d >= 0) {
			fv[0] = 0;
			fv[1] = 3;
			fv[2] = 7;
			fv[3] = 4;
		}
		else {
			fv[0] = 1;
			fv[1] = 5;
			fv[2] = 6;
			fv[3] = 2;
		}
		break;
	case 2:
		if (d >= 0) {
			fv[0] = 0;
			fv[1] = 4;
			fv[2] = 5;
			fv[3] = 1;
		}
		else {
			fv[0] = 2;
			fv[1] = 6;
			fv[2] = 7;
			fv[3] = 3;
		}
		break;
	default:
		break;
	}
}

COBB& COBB::operator=(const COBB& other) {
	axis = other.axis;
	size = other.size;
	hsize = other.hsize;
	cent = other.cent;
	vp = other.vp;
	chv = other.chv;
	vol = other.vol;
	dl = other.dl;
	ca = other.ca;
	cas = other.cas;
	as = other.as;
	ap = other.ap;
	al = other.al;
	return *this;
}

int COBB::operator==(const COBB& other) {
	double d[3];
	d[0] = MathLib::Abs(axis[0].dot(other.axis[0]));
	d[1] = MathLib::Abs(axis[0].dot(other.axis[1]));
	d[2] = MathLib::Abs(axis[0].dot(other.axis[2]));
	double dm = __max(d[0], __max(d[1], d[2]));
	if (dm < 0.98) {
		return 0;
	}
	int mi1, mi2, mi3;
	if (dm == d[0]) {
		mi1 = 0;
	}
	else if (dm == d[1]) {
		mi1 = 1;
	}
	else {
		mi1 = 2;
	}
	int di(0);
	int mi[2];
	for (int i = 0; i < 3; i++) {
		if (i == mi1) {
			continue;
		}
		d[di] = MathLib::Abs(axis[1].dot(other.axis[i]));
		mi[di++] = i;
	}
	dm = __max(d[0], d[1]);
	if (dm < 0.98) {
		return 0;
	}
	if (dm == d[0]) {
		mi2 = mi[0];
	}
	else {
		mi2 = mi[1];
	}
	mi3 = 3 - mi1 - mi2;
	dm = MathLib::Abs(axis[2].dot(other.axis[mi3]));
	if (dm < 0.98) {
		return 0;
	}

	if (!MathLib::IsEqual(size[0], other.size[mi1], (size[0] + other.size[mi1])*0.05)
		|| !MathLib::IsEqual(size[1], other.size[mi2], (size[1] + other.size[mi2])*0.05)
		|| !MathLib::IsEqual(size[2], other.size[mi3], (size[2] + other.size[mi3])*0.05)) {
		return 0;
	}
	if (!MathLib::IsEqual(cent[0], other.cent[0], (size[0] + other.size[mi1])*0.05)
		|| !MathLib::IsEqual(cent[1], other.cent[1], (size[0] + other.size[mi1])*0.05)
		|| !MathLib::IsEqual(cent[2], other.cent[2], (size[0] + other.size[mi1])*0.05)) {
		return 0;
	}

	if (vol < other.vol) {
		if (vol / other.vol < 0.9) {
			return 0;
		}
	}
	else {
		if (other.vol / vol < 0.9) {
			return 0;
		}
	}
	return 1;
}
int COBB::operator!=(const COBB& other) { return !(*this == other); }

void COBB::DrawBox(bool bFace, bool bGraph, bool bShowStat, bool bshowAnnoFace, bool bHighL) const
{
	GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GLfloat red[] = { 1.0, 0.3, 0.3, 1.0 };
	GLfloat green[] = { 0.5f, 1.0f, 0.5f, 1.0f };
	GLfloat axis_col[3][4] = { { 1.0f, 0.3f, 0.3f, 1.0f }, { 0.2f, 0.8f, 0.2f, 1.0f }, { 0.1f, 0.1f, 1.0f, 1.0f } };
	GLfloat yellow[] = { 0.9f, 0.9f, 0.0f, 1.0f };

	glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_HINT_BIT | GL_LINE_BIT | GL_CURRENT_BIT);

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glDisable(GL_LIGHTING);
	if (bFace) {
		glColor3fv(black);
		if (bHighL) {
			glLineWidth(2.0);
		}
		else {
			glLineWidth(1.0);
		}
	}
	else {
		if (bHighL) {
			glColor3f(red[0], red[1], red[2]);
		}
		else {
			glColor3f(green[0], green[1], green[2]);
		}
		glLineWidth(2.0);
	}
	glBegin(GL_LINE_LOOP);
	glVertex3f(vp[0][0], vp[0][1], vp[0][2]);
	glVertex3f(vp[1][0], vp[1][1], vp[1][2]);
	glVertex3f(vp[2][0], vp[2][1], vp[2][2]);
	glVertex3f(vp[3][0], vp[3][1], vp[3][2]);
	glEnd();
	glBegin(GL_LINE_LOOP);
	glVertex3f(vp[4][0], vp[4][1], vp[4][2]);
	glVertex3f(vp[5][0], vp[5][1], vp[5][2]);
	glVertex3f(vp[6][0], vp[6][1], vp[6][2]);
	glVertex3f(vp[7][0], vp[7][1], vp[7][2]);
	glEnd();
	glBegin(GL_LINES);
	glVertex3f(vp[0][0], vp[0][1], vp[0][2]);
	glVertex3f(vp[4][0], vp[4][1], vp[4][2]);
	glVertex3f(vp[1][0], vp[1][1], vp[1][2]);
	glVertex3f(vp[5][0], vp[5][1], vp[5][2]);
	glVertex3f(vp[2][0], vp[2][1], vp[2][2]);
	glVertex3f(vp[6][0], vp[6][1], vp[6][2]);
	glVertex3f(vp[3][0], vp[3][1], vp[3][2]);
	glVertex3f(vp[7][0], vp[7][1], vp[7][2]);
	glEnd();


	if (bGraph) {
		for (int i = 0; i < 3; i++) {
			glLineWidth((ca == i) ? 5.0f : 2.5f);
			glColor4fv(&axis_col[i][0]);
			glBegin(GL_LINES);
			glVertex3f(cent[0], cent[1], cent[2]);
			glVertex3f(cent[0] + axis[i][0] * hsize[i],
				cent[1] + axis[i][1] * hsize[i],
				cent[2] + axis[i][2] * hsize[i]);
			glEnd();
		}
		/*for (int i=0; i<8; i++) {
			char sText[32];
			sprintf_s(sText, "%d", i);
			glRasterPos3F(vp[i][0], vp[i][1], vp[i][2]);
			text18(sText);
			}*/
		/*
		if (bShowStat) {
		glDisable(GL_DEPTH_TEST);
		char sText[32];
		glColor4fv(&axis_col[ca][0]);
		sprintf_s(sText, "%.3f", cas);
		glRasterPos3f(cent[0]+axis[ca][0]*hsize[ca], cent[1]+axis[ca][1]*hsize[ca], cent[2]+axis[ca][2]*hsize[ca]);
		text18(sText);
		}
		*/
	}

	//////////////////////////////////////////////////////////////////////////
	if (bFace) {
		glPushAttrib(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_LIGHTING);
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		glEnable(GL_COLOR_MATERIAL);
		if (bHighL) {
			glColor4f(red[0], red[1], green[2], 0.1f);
		}
		else {
			glColor4f(green[0], green[1], green[2], 0.1f);
		}
		glBegin(GL_TRIANGLES);
		for (int i = 0; i < boxNumFace; i++) {

			glNormal3f(axis[boxFaceNormalOrientAlongAxis[i][0]][0] * boxFaceNormalOrientAlongAxis[i][1],
				axis[boxFaceNormalOrientAlongAxis[i][0]][1] * boxFaceNormalOrientAlongAxis[i][1],
				axis[boxFaceNormalOrientAlongAxis[i][0]][2] * boxFaceNormalOrientAlongAxis[i][1]);
			glVertex3dv(vp[boxTriFace[i][0]].v);
			glVertex3dv(vp[boxTriFace[i][1]].v);
			glVertex3dv(vp[boxTriFace[i][2]].v);
		}
		glEnd();
		glPopAttrib();
	}

	// draw selected faces
	if (bshowAnnoFace && selTriFaceIds.size() > 0)
	{
		glPushAttrib(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_LIGHTING);
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		glEnable(GL_COLOR_MATERIAL);

		glBegin(GL_TRIANGLES);
		for (int i = 0; i < boxNumFace; i++) {

			if (std::find(selTriFaceIds.begin(), selTriFaceIds.end(), i) != selTriFaceIds.end())
			{
				glColor4f(red[0], red[1], red[2], 0.5f);
				glNormal3f(axis[boxFaceNormalOrientAlongAxis[i][0]][0] * boxFaceNormalOrientAlongAxis[i][1],
					axis[boxFaceNormalOrientAlongAxis[i][0]][1] * boxFaceNormalOrientAlongAxis[i][1],
					axis[boxFaceNormalOrientAlongAxis[i][0]][2] * boxFaceNormalOrientAlongAxis[i][1]);
				glVertex3dv(vp[boxTriFace[i][0]].v);
				glVertex3dv(vp[boxTriFace[i][1]].v);
				glVertex3dv(vp[boxTriFace[i][2]].v);
			}
		}
		glEnd();
		glPopAttrib();
	}

	//////////////////////////////////////////////////////////////////////////

	glPopAttrib();
}

void COBB::DrawSamples(void)
{
	if (sp.empty()) {
		return;
	}
	GLfloat red[] = { 0.9f, 0.1f, 0.1f, 1.0f };
	glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT | GL_POINT_BIT);
	glDisable(GL_LIGHTING);
	glColor4fv(red);
	glPointSize(3.0f);
	glBegin(GL_POINTS);
	for (unsigned int i = 0; i<sp.size(); i++) {
		glVertex3dv(sp[i].v);
	}
	glEnd();
	glPopAttrib();
}

int COBB::DoSampling(double r)
{
	if (r > size[2]) {
		return -1;
	}
	sp.clear();
	sr = r;
	for (int i = 0; i < 3; i++) {
		int a1 = (i + 1) % 3;
		int a2 = (i + 2) % 3;
		int ns1 = (int)(size[a1] / sr);
		int ns2 = (int)(size[a2] / sr);
		double p1 = -hsize[a1];
		double p2 = -hsize[a2];
		MathLib::Vector3 st = cent + axis[i] * hsize[i];
		for (int j = 0; j < ns1; j++) {
			for (int k = 0; k < ns2; k++) {
				sp.push_back(st + axis[a1] * p1 + axis[a2] * p2);
				p2 += sr;
			}
			p1 += sr;
			p2 = -hsize[a2];
		}
		st = cent - axis[i] * hsize[i];
		p1 = hsize[a1];
		p2 = hsize[a2];
		for (int j = 0; j < ns1; j++) {
			for (int k = 0; k < ns2; k++) {
				sp.push_back(st + axis[a1] * p1 + axis[a2] * p2);
				p2 -= sr;
			}
			p1 -= sr;
			p2 = hsize[a2];
		}
	}
	return 0;
}

double COBB::ConnStrength_Proj(const COBB &obb, const MathLib::Vector3 &dir) const
{
	MathLib::Vector3 pd(dir); pd.normalize();
	const std::vector<MathLib::Vector3> &VI = vp;
	const std::vector<MathLib::Vector3> &AI = axis;
	const std::vector<MathLib::Vector3> &VJ = obb.vp;
	const std::vector<MathLib::Vector3> &AJ = obb.axis;
	MathLib::Vector3 FNI, FNJ;
	double dDMinI(1.0), dDMinJ(1.0);
	int iFI(-1), iFJ(-1);
	for (int i = 0; i < boxNumFace; i++) {
		FNI.set(AI[boxFaceNormalOrientAlongAxis[i][0]][0] * boxFaceNormalOrientAlongAxis[i][1], AI[boxFaceNormalOrientAlongAxis[i][0]][1] * boxFaceNormalOrientAlongAxis[i][1], AI[boxFaceNormalOrientAlongAxis[i][0]][2] * boxFaceNormalOrientAlongAxis[i][1]);
		double dDot = MathLib::Abs(FNI.dot(pd));
		if (dDot < dDMinI) {
			dDMinI = dDot;
			iFI = i;
		}
		FNJ.set(AJ[boxFaceNormalOrientAlongAxis[i][0]][0] * boxFaceNormalOrientAlongAxis[i][1], AJ[boxFaceNormalOrientAlongAxis[i][0]][1] * boxFaceNormalOrientAlongAxis[i][1], AJ[boxFaceNormalOrientAlongAxis[i][0]][2] * boxFaceNormalOrientAlongAxis[i][1]);
		dDot = MathLib::Abs(FNJ.dot(pd));
		if (dDot < dDMinJ) {
			dDMinJ = dDot;
			iFJ = i;
		}
	}
	double dli = VI[boxTriFace[iFI][0]].distance(VI[boxTriFace[iFI][2]]);
	double dlj = VJ[boxTriFace[iFJ][0]].distance(VJ[boxTriFace[iFJ][2]]);
	double hdp = HausdorffDist_Proj(obb, pd);
	double eps = +std::numeric_limits<double>::epsilon();
	double cs = (dli + dlj) / (hdp + eps);
	return cs;
}

double COBB::SqDistance_Approx(const COBB &bb) const
{
	double dT = std::min(dl, bb.dl) * 0.01;
	MathLib::Vector3 cp;	// closest point
	ClosestPoint(bb.cent, cp);
	double sd = bb.cent.squaredistance(cp);
	if (sd < dT) { return sd; }
	for (unsigned int i = 0; i < bb.vp.size(); i++) {
		ClosestPoint(bb.vp[i], cp);
		sd = std::min(sd, bb.vp[i].squaredistance(cp));
		if (sd < dT) { return sd; }
	}
	bb.ClosestPoint(cent, cp);
	sd = std::min(sd, cent.squaredistance(cp));
	if (sd < dT) { return sd; }
	for (unsigned int i = 0; i < vp.size(); i++) {
		bb.ClosestPoint(vp[i], cp);
		sd = std::min(sd, vp[i].squaredistance(cp));
		if (sd < dT) { return sd; }
	}
	return sd;
}

void COBB::ClosestPoint(const MathLib::Vector3 &p, MathLib::Vector3 &cp) const
{
	MathLib::Vector3 d = p - cent;
	cp = cent; // Start at the center point of the OBB.
	for (int i = 0; i < 3; ++i) {
		double pd = d.dot(axis[i]);
		MathLib::Clampf(pd, -hsize[i], hsize[i]);
		cp += axis[i] * pd;
	}
}

double COBB::ConnStrength_HD(const COBB &obb) const
{
	double cs = (dl + obb.dl) / (HausdorffDist(obb) + std::numeric_limits<double>::epsilon());
	return cs;
}

double COBB::ConnStrength_CD(const COBB &obb) const
{
	double cs = (dl + obb.dl) / (SqDistance_Approx(obb) + std::numeric_limits<double>::epsilon());
	return cs;
}

double COBB::TopHeightDiff(const COBB &obb, const MathLib::Vector3 &upright) const
{
	double tmax1(-std::numeric_limits<double>::max());
	for (unsigned int i = 0; i<vp.size(); i++) {
		double t = upright.dot(vp[i] - MathLib::ML_O);
		if (t > tmax1) {
			tmax1 = t;
		}
	}
	double tmax2(-std::numeric_limits<double>::max());
	for (unsigned int i = 0; i<obb.vp.size(); i++) {
		double t = upright.dot(obb.vp[i] - MathLib::ML_O);
		if (t > tmax2) {
			tmax2 = t;
		}
	}
	return MathLib::Abs(tmax1 - tmax2);
}

double COBB::GetBottomHeight(const MathLib::Vector3 &upright) const
{
	double tmin(std::numeric_limits<double>::max());
	for (unsigned int i = 0; i < vp.size(); i++) {
		double t = upright.dot(vp[i] - MathLib::ML_O);
		if (t < tmin) {
			tmin = t;
		}
	}
	return tmin;
}

double COBB::BottomHeightDiff(const COBB &obb, const MathLib::Vector3 &upright) const
{
	double tmin1(std::numeric_limits<double>::max());
	for (unsigned int i = 0; i < vp.size(); i++) {
		double t = upright.dot(vp[i] - MathLib::ML_O);
		if (t < tmin1) {
			tmin1 = t;
		}
	}
	double tmin2(std::numeric_limits<double>::max());
	for (unsigned int i = 0; i < obb.vp.size(); i++) {
		double t = upright.dot(obb.vp[i] - MathLib::ML_O);
		if (t < tmin2) {
			tmin2 = t;
		}
	}
	return (tmin1 - tmin2);
}

bool COBB::IsAbove(const COBB &obb, const MathLib::Vector3 &upright) const
{
	for (unsigned int i = 0; i<obb.vp.size(); i++) {
		if (upright.dot(cent - obb.vp[i]) > 0) {
			return false;
		}
	}
	return true;
}

bool COBB::IsTwoSide(const COBB &obb, const MathLib::Vector3 &p, const MathLib::Vector3 &n) const
{
	double t1 = n.dot(cent - p);
	double t2 = n.dot(obb.cent - p);
	return (t1*t2 < 0);
}

bool COBB::IsOnTop(COBB &obb, const MathLib::Vector3 &upright, double &pd) const
{
	//	double dPD(std::numeric_limits<double>::max());
	if (IsContain(obb)) {
		return false;
	}
	MathLib::Vector3 dir = -upright;
	double dm(0);
	for (unsigned int i = 0; i<vp.size(); i++) {
		double t = dir.dot(vp[i] - cent);
		if (t > dm) {
			dm = t;
		}
	}
	double pdd = pd;
	bool b = obb.PickByRay_Ortho(cent, dir, pdd);
	if (!b || pdd < 0.7*dm) {
		return false;
	}
	else {
		pd = pdd;
		return true;
	}
}

double COBB::ConnStrength_OBB(const COBB &obb, int iFixA/*=-1*/) const
{
	std::vector<MathLib::Vector3> VL;	// vertex list
	std::copy(vp.begin(), vp.end(), std::back_inserter(VL));
	std::copy(obb.vp.begin(), obb.vp.end(), std::back_inserter(VL));
	COBB tobb;
	COBBEstimator OBBE(&VL, &tobb);
	OBBE.ComputeOBB_Min(iFixA);
	return (vol + obb.vol) / tobb.vol;
}

bool COBB::IsContact(const COBB &obb, double ta, double td, MathLib::Vector3 &dir) const
{
	const std::vector<MathLib::Vector3> &VI = vp;
	const std::vector<MathLib::Vector3> &AI = axis;
	const std::vector<MathLib::Vector3> &VJ = obb.vp;
	const std::vector<MathLib::Vector3> &AJ = obb.axis;
	MathLib::Vector3 FNI, FNJ;
	for (int i = 0; i < boxNumFace; i++) {
		FNI.set(AI[boxFaceNormalOrientAlongAxis[i][0]][0] * boxFaceNormalOrientAlongAxis[i][1], AI[boxFaceNormalOrientAlongAxis[i][0]][1] * boxFaceNormalOrientAlongAxis[i][1], AI[boxFaceNormalOrientAlongAxis[i][0]][2] * boxFaceNormalOrientAlongAxis[i][1]);
		for (int j = 0; j < boxNumFace; j++) {
			FNJ.set(AJ[boxFaceNormalOrientAlongAxis[j][0]][0] * boxFaceNormalOrientAlongAxis[j][1], AJ[boxFaceNormalOrientAlongAxis[j][0]][1] * boxFaceNormalOrientAlongAxis[j][1], AJ[boxFaceNormalOrientAlongAxis[j][0]][2] * boxFaceNormalOrientAlongAxis[j][1]);
			// 			if (IsTwoSide(obb,VI[psBoxF[i][0]],FNI)) {
			// 				if (ContactTriTri(VI[psBoxF[i][0]],VI[psBoxF[i][1]],VI[psBoxF[i][2]],FNI,VJ[psBoxF[j][0]],VJ[psBoxF[j][1]],VJ[psBoxF[j][2]],FNJ,td)) {
			// 					dir = FNI;
			// 					return true;
			// 				}
			// 			}
			if (ContactTriTri(VI[boxTriFace[i][0]], VI[boxTriFace[i][1]], VI[boxTriFace[i][2]], FNI, VJ[boxTriFace[j][0]], VJ[boxTriFace[j][1]], VJ[boxTriFace[j][2]], FNJ, ta, td, true)) {
				dir = FNI;
				return true;
			}
		}
	}
	return false;
}

bool COBB::IsSupport(const COBB &obb, double ta, double td, const MathLib::Vector3 &upright) const
{
	const std::vector<MathLib::Vector3> &VI = vp;
	const std::vector<MathLib::Vector3> &AI = axis;
	const std::vector<MathLib::Vector3> &VJ = obb.vp;
	const std::vector<MathLib::Vector3> &AJ = obb.axis;
	MathLib::Vector3 FNI, FNJ;
	for (int i = 0; i<boxNumFace; i++) {
		FNI.set(AI[boxFaceNormalOrientAlongAxis[i][0]][0] * boxFaceNormalOrientAlongAxis[i][1], AI[boxFaceNormalOrientAlongAxis[i][0]][1] * boxFaceNormalOrientAlongAxis[i][1], AI[boxFaceNormalOrientAlongAxis[i][0]][2] * boxFaceNormalOrientAlongAxis[i][1]);
		if (MathLib::Acos(MathLib::Abs(FNI.dot(upright))) > 1.0) {
			continue;
		}
		for (int j = 0; j < boxNumFace; j++) {
			FNJ.set(AJ[boxFaceNormalOrientAlongAxis[j][0]][0] * boxFaceNormalOrientAlongAxis[j][1], AJ[boxFaceNormalOrientAlongAxis[j][0]][1] * boxFaceNormalOrientAlongAxis[j][1], AJ[boxFaceNormalOrientAlongAxis[j][0]][2] * boxFaceNormalOrientAlongAxis[j][1]);
			if (IsTwoSide(obb, VI[boxTriFace[i][0]], upright)) {
				if (ContactTriTri(VI[boxTriFace[i][0]], VI[boxTriFace[i][1]], VI[boxTriFace[i][2]], FNI, VJ[boxTriFace[j][0]], VJ[boxTriFace[j][1]], VJ[boxTriFace[j][2]], FNJ, td)) {
			 					return true;
			 				}
			 			}
			if (ContactTriTri(VI[boxTriFace[i][0]], VI[boxTriFace[i][1]], VI[boxTriFace[i][2]], FNI, VJ[boxTriFace[j][0]], VJ[boxTriFace[j][1]], VJ[boxTriFace[j][2]], FNJ, ta, td, true)) {
				return true;
			}

			if (ContactTriTri(VI[boxTriFace[j][0]], VI[boxTriFace[j][1]], VI[boxTriFace[j][2]], FNJ, VJ[boxTriFace[i][0]], VJ[boxTriFace[i][1]], VJ[boxTriFace[i][2]], FNI, ta, td, true)) {
				return true;
			}
		}
	}
	return false;
}

bool COBB::IsRoughSupport(const COBB &obb)
{
	// if the center falls into the bottom face of the ref obb, then treat the test obb is rough supported
	MathLib::Vector2 testCenter = MathLib::Vector2(obb.cent[0], obb.cent[1]);
	MathLib::Vector2 axisX(axis[0][0], axis[0][1]), axisY(axis[1][0], axis[1][0]);

	MathLib::Vector2 dv = testCenter - MathLib::Vector2(vp[6][0], vp[6][1]);

	double d;
	d = axisX.dot(dv);
	if (d<0.0 || d>size[0]) {
		return false;
	}
	d = axisY.dot(dv);
	if (d<0.0 || d>size[1]) {
		return false;
	}

	return true;
}

bool COBB::IsContain(const COBB &obb) const
{
	for (unsigned int i = 0; i < obb.vp.size(); i++) {
		if (!IsInside(obb.vp[i])) {
			return false;
		}
	}
	return true;
}

bool COBB::IsContain(const COBB &obb, double tp) const
{
	int npd = 11;
	double hx = obb.size[0] / (double)npd;
	double hy = obb.size[1] / (double)npd;
	double hz = obb.size[2] / (double)npd;
	int tni = (double)(npd*npd*npd) * tp;
	int tno = npd*npd*npd - tni;
	MathLib::Vector3 p(0.0, 0.0, 0.0);
	int ni(0), no(0);
	for (p.x = 0.0; p.x <= obb.size[0]; p.x += hx) {
		for (p.y = 0.0; p.y <= obb.size[1]; p.y += hy) {
			for (p.z = 0.0; p.z <= obb.size[2]; p.z += hz) {
				if (IsInside(p)) {
					ni++;
					if (ni >= tni) { // if tp% of the points are inside, then return contained
						return true;
					}
				}
				else {
					no++;
					if (no >= tno) { // if (1-tp)% of the points are outside, then return un-contained
						return false;
					}
				}
			}
		}
	}
	return false;
}

inline bool COBB::IsInside(const MathLib::Vector3 &p) const
{
	double d;
	MathLib::Vector3 dv = p - vp[6];
	d = axis[0].dot(dv);
	if (d<0.0 || d>size[0]) {
		return false;
	}
	d = axis[1].dot(dv);
	if (d<0.0 || d>size[1]) {
		return false;
	}
	d = axis[2].dot(dv);
	if (d<0.0 || d>size[2]) {
		return false;
	}
	return true;
}

bool COBB::IsIntersAACube(const MathLib::Vector3 &c, double s) const
{
	MathLib::Vector3 p;
	double hs = 0.5*s;
	p = c;
	p.x += hs;	p.y += hs;	p.z += hs;
	if (IsInside(p)) {
		return true;
	}
	p = c;
	p.x += hs;	p.y -= hs;	p.z += hs;
	if (IsInside(p)) {
		return true;
	}
	p = c;
	p.x += hs;	p.y -= hs;	p.z -= hs;
	if (IsInside(p)) {
		return true;
	}
	p = c;
	p.x += hs;	p.y += hs;	p.z -= hs;
	if (IsInside(p)) {
		return true;
	}
	p = c;
	p.x -= hs;	p.y += hs;	p.z += hs;
	if (IsInside(p)) {
		return true;
	}
	p = c;
	p.x -= hs;	p.y -= hs;	p.z += hs;
	if (IsInside(p)) {
		return true;
	}
	p = c;
	p.x -= hs;	p.y -= hs;	p.z -= hs;
	if (IsInside(p)) {
		return true;
	}
	p = c;
	p.x -= hs;	p.y += hs;	p.z -= hs;
	if (IsInside(p)) {
		return true;
	}
	return false;
}

void COBB::GetApproxBoxes(std::vector<COBB> &BL) const
{
	BL.clear();
	BL.push_back(*this);
}

bool COBB::PickByRay_Ortho(const MathLib::Vector3 &sp, const MathLib::Vector3 &dir, double &dPD)
{
	MathLib::Vector3	e1, e2, p, s, q;
	double		t(0), u(0), v(0), w(0), tmp(0);
	int		pi = -1;
	const std::vector<MathLib::Vector3> &A = axis;
	MathLib::Vector3 fn;
	for (int i = 0; i < boxNumFace; i++) {
		fn.set(A[boxFaceNormalOrientAlongAxis[i][0]][0] * boxFaceNormalOrientAlongAxis[i][1], A[boxFaceNormalOrientAlongAxis[i][0]][1] * boxFaceNormalOrientAlongAxis[i][1], A[boxFaceNormalOrientAlongAxis[i][0]][2] * boxFaceNormalOrientAlongAxis[i][1]);
		if (MathLib::Acos(fn.dot(dir)) < 179.0) {
			continue;
		}
		const MathLib::Vector3 &v1 = vp[boxTriFace[i][0]];
		const MathLib::Vector3 &v2 = vp[boxTriFace[i][1]];
		const MathLib::Vector3 &v3 = vp[boxTriFace[i][2]];
		e1.set(v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2]);
		e2.set(v3[0] - v1[0], v3[1] - v1[1], v3[2] - v1[2]);
		p = dir.cross(e2);
		tmp = p.dot(e1);
		if (MathLib::IsZero(tmp)) {
			continue;
		}
		tmp = 1.0f / tmp;
		s.set(sp.x - v1[0], sp.y - v1[1], sp.z - v1[2]);
		u = tmp * p.dot(s);
		if (u < 0.0f || u > 1.0f) {
			continue;
		}
		q = s.cross(e1);
		v = tmp * q.dot(dir);
		if (v < 0.0f || v > 1.0f) {
			continue;
		}
		w = u + v;
		if (w > 1.0f) {
			continue;
		}
		t = tmp * q.dot(e2);
		if (t > 0 && t < dPD) {
			pi = i;
			dPD = t;
		}
	}

	// save selected faces
	if (pi != -1)
	{
		std::vector<int>::iterator id_pos;
		id_pos = std::find(selTriFaceIds.begin(), selTriFaceIds.end(), pi);
		if (id_pos != selTriFaceIds.end())
		{
			selTriFaceIds.erase(id_pos);
		}

		id_pos = std::find(selTriFaceIds.begin(), selTriFaceIds.end(), boxTriFacePair[pi]);
		if (id_pos != selTriFaceIds.end())
		{
			selTriFaceIds.erase(id_pos);
		}

		else
		{
			selTriFaceIds.push_back(pi);
			selTriFaceIds.push_back(boxTriFacePair[pi]);  // push the tri with in the same quad
		}
	}

	return (pi != -1);
}

bool COBB::PickByRay(const MathLib::Vector3 &sp, const MathLib::Vector3 &dir, double &dPD)
{
	MathLib::Vector3	e1, e2, p, s, q;
	double		t(0), u(0), v(0), w(0), tmp(0);
	int		pi = -1;
	const std::vector<MathLib::Vector3> &VI = vp;
	for (int i = 0; i < boxNumFace; i++) {
		const MathLib::Vector3 &v1 = vp[boxTriFace[i][0]];
		const MathLib::Vector3 &v2 = vp[boxTriFace[i][1]];
		const MathLib::Vector3 &v3 = vp[boxTriFace[i][2]];
		e1.set(v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2]);
		e2.set(v3[0] - v1[0], v3[1] - v1[1], v3[2] - v1[2]);
		p = dir.cross(e2);
		tmp = p.dot(e1);
		if (MathLib::IsZero(tmp)) {
			continue;
		}
		tmp = 1.0f / tmp;
		s.set(sp.x - v1[0], sp.y - v1[1], sp.z - v1[2]);
		u = tmp * p.dot(s);
		if (u < 0.0f || u > 1.0f) {
			continue;
		}
		q = s.cross(e1);
		v = tmp * q.dot(dir);
		if (v < 0.0f || v > 1.0f) {
			continue;
		}
		w = u + v;
		if (w > 1.0f) {
			continue;
		}
		t = tmp * q.dot(e2);
		if (t > 0 && t < dPD) {
			pi = i;
			dPD = t;
		}
	}

	// save selected faces
	if (pi != -1)
	{
		std::vector<int>::iterator id_pos;
		id_pos = std::find(selTriFaceIds.begin(), selTriFaceIds.end(), pi);
		if (id_pos != selTriFaceIds.end())
		{
			selTriFaceIds.erase(id_pos);
		}

		id_pos = std::find(selTriFaceIds.begin(), selTriFaceIds.end(), boxTriFacePair[pi]);
		if (id_pos != selTriFaceIds.end())
		{
			selTriFaceIds.erase(id_pos);
		}

		else
		{
			selTriFaceIds.push_back(pi);
			selTriFaceIds.push_back(boxTriFacePair[pi]);  // push the tri with in the same quad
		}
	}

	return (pi != -1);
}

std::vector<MathLib::Vector3> COBB::GetTransformedVertices(const MathLib::Matrix4d &transMat)
{
	std::vector<MathLib::Vector3> trans_vp = vp;

	for (int i = 0; i < trans_vp.size(); i++)
	{
		trans_vp[i] = transMat.transform(vp[i]);
	}

	return trans_vp;
}

std::vector<int> COBB::getSelQuadFaceIds()
{
	std::set<int> quadIdSet;

	for (int i = 0; i < selTriFaceIds.size(); i++)
	{
		quadIdSet.insert(boxTriToQuadFaceMap[selTriFaceIds[i]]);
	}

	std::vector<int> quadIds(quadIdSet.begin(), quadIdSet.end());

	return quadIds;
}

MathLib::Vector3 COBB::getFaceNormal(int faceId)
{
	switch (faceId)
	{
	case 0:
		return axis[0];
	case 1:
		return -axis[0];
	case 2:
		return axis[2];
	case 3:
		return -axis[1];
	case 4:
		return -axis[2];
	case 5:
		return axis[1];
	}
}

void COBB::setSelQuadFace(const std::vector<int> &quadIds)
{
	for (int i = 0; i < quadIds.size(); i++)
	{
		int f_id = quadIds[i];

		selTriFaceIds.push_back(boxQuadToTriFaceMap[f_id][0]);
		selTriFaceIds.push_back(boxQuadToTriFaceMap[f_id][1]);
	}
}

MathLib::Vector3 COBB::GetFaceHorizonAxis(int f) const
{
	MathLib::Vector3 edgeDir1 = vp[boxQuadFace[f][0]] - vp[boxQuadFace[f][1]];
	MathLib::Vector3 edgeDir2 = vp[boxQuadFace[f][1]] - vp[boxQuadFace[f][2]];

	edgeDir1.normalize();
	edgeDir2.normalize();

	if (std::abs(edgeDir1.dot(axis[2])) < 1e-3)
	{
		if (std::abs(edgeDir1.dot(axis[0])) > 0.9)
		{
			return axis[0];
		}

		if (std::abs(edgeDir1.dot(axis[1])) > 0.9)
		{
			return axis[1];
		}
	}

	if (std::abs(edgeDir2.dot(axis[2])) < 1e-3)
	{
		if (std::abs(edgeDir2.dot(axis[0])) > 0.9)
		{
			return axis[0];
		}

		if (std::abs(edgeDir2.dot(axis[1])) > 0.9)
		{
			return axis[1];
		}
	}
}

double COBB::GetDiagLength()
{
	double d = 0;
	d = size[0] * size[0] + size[1] * size[1] + size[2] * size[2];
	d = std::sqrt(d);

	return d;
}

double COBB::GetHeight()
{
	for (int i = 0; i < 3; i++)
	{
		if (std::abs(axis[i].dot(MathLib::Vector3(0,0,1)) > 0.9))
		{
			return size[i];
		}
	}
}


