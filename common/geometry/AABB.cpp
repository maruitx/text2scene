
#include "AABB.h"
#include <fstream>

GLuint CAABB::s_edl = 0;

static bool CAgreater(const std::pair<double,std::pair<int,int>> &ca1, const std::pair<double,std::pair<int,int>> &ca2) { return ca1.first>ca2.first; }

CAABB::CAABB(void)
{
	vp.resize(8);
	ca = 0;
	dl = vol = cas = 0.0;
	er = 0.02;
	as = ap = al = 0.0;

}

CAABB::CAABB(const CAABB& aabb)
{
	*this = aabb;
}

CAABB::CAABB(const MathLib::Vector3 &c, const MathLib::Vector3 &s)
{
	SetData(c, s);
}

CAABB::~CAABB()
{
}

void CAABB::Unitize(const MathLib::Vector3 &c, double s)
{
	for (unsigned int i=0; i<8; i++) {
		vp[i] -= c;
		vp[i] *= s;
	}
	updateDataP();
}

void CAABB::SetData(const MathLib::Vector3 &c, const MathLib::Vector3 &s)
{	// Axis-aligned data
	cent = c;
	size = s;
	updateDataAS();
}

void CAABB::SetDataM(const MathLib::Vector3 &min, const MathLib::Vector3 &max)
{	// Axis-aligned data
	cent = (max+min)*0.5;
	size[0] = max[0]-min[0];
	size[1] = max[1]-min[1];
	size[2] = max[2]-min[2];
	updateDataAS();
}

// Update with positions as known
void CAABB::updateDataP(void)
{
	cent.setzero();
	for (int i=0; i<8; i++) {
		cent += vp[i];
	}
	cent /= 8.0;
	hsize[0] = 0.5*size[0];
	hsize[1] = 0.5*size[1];
	hsize[2] = 0.5*size[2];
	vol = size[0]*size[1]*size[2];
	dl = std::sqrt(size[0]*size[0]+size[1]*size[1]+size[2]*size[2]);
}

// Update with axes and center as known
// ??? : better not perform re-ordering... problems would come
void CAABB::updateDataAS(void)
{
	// Update the rest stuff
	hsize[0] = 0.5*size[0];
	hsize[1] = 0.5*size[1];
	hsize[2] = 0.5*size[2];
	vp[0] = cent + MathLib::Vector3(hsize[0],hsize[1],hsize[2]);
	vp[1] = cent + MathLib::Vector3(hsize[0],-hsize[1],hsize[2]);
	vp[2] = cent + MathLib::Vector3(hsize[0],-hsize[1],-hsize[2]);
	vp[3] = cent + MathLib::Vector3(hsize[0],hsize[1],-hsize[2]);
	vp[4] = cent + MathLib::Vector3(-hsize[0],hsize[1],hsize[2]);
	vp[5] = cent + MathLib::Vector3(-hsize[0],-hsize[1],hsize[2]);
	vp[6] = cent + MathLib::Vector3(-hsize[0],-hsize[1],-hsize[2]);
	vp[7] = cent + MathLib::Vector3(-hsize[0],hsize[1],-hsize[2]);
	vol = size[0]*size[1]*size[2];
	dl = std::sqrt(size[0]*size[0]+size[1]*size[1]+size[2]*size[2]);
}

void CAABB::TransScl(double dx, double dy, double dz, double s)
{
	for (unsigned int j=0; j<8; j++) {
		vp[j][0] += dx;
		vp[j][1] += dy;
		vp[j][2] += dz;
		vp[j][0] *= s;
		vp[j][1] *= s;
		vp[j][2] *= s;
	}
	// inversely computing the cent
	cent.set(0.0, 0.0, 0.0);
	for (unsigned int j=0; j<8; j++) {
		cent += vp[j];
	}
	cent /= 8.0f;
	size *= s;
	updateDataAS();
}

void CAABB::Merge(const CAABB &newBox) {
	MathLib::Vector3 newMin = newBox.GetMinV();
	MathLib::Vector3 newMax = newBox.GetMaxV();

	MathLib::Vector3 currMin = this->GetMinV();
	MathLib::Vector3 currMax = this->GetMaxV();

	for (int i = 0; i < 3; i++)
	{
		if (currMin[i] < newMin[i])
		{
			newMin[i] = currMin[i];
		}

		if (currMax[i] > newMax[i])
		{
			newMax[i] = currMax[i];
		}
	}

	SetDataM(newMin, newMax);
}

void CAABB::analyzeAxis(void) {
	// Analyze characteristic axis
	// ??? TODO: deal with un-re-ordered axes
	al = (size[0]-size[1])/(size[0]+size[1]+size[2]);		// linear
	ap = 2.0f*(size[1]-size[2])/(size[0]+size[1]+size[2]);	// planar
	as = 3.0f*size[2]/(size[0]+size[1]+size[2]);			// spherical

	ca = (al>=ap) ? 0 : 2;
	cas = (al>=ap) ? al : ap;
}

MathLib::Vector3 CAABB::GetFaceCent(int f) const
{
	if (f<0 || f>=boxNumQuadFace) {
		Simple_Message_Box(QString("CAABB::GetFaceCent: Invaid face id!"));
		return MathLib::Vector3(0,0,0);
	}
	MathLib::Vector3 cent;
	for (int i=0; i<4; i++) {
		cent += vp[boxQuadFace[f][i]];
	}
	return cent/4.0;
}

void CAABB::CalcAnisotropy(MathLib::Vector3 &c) const
{
	std::vector<double> s(3);
	s[0] = size[0];
	s[1] = size[1];
	s[2] = size[2];
	std::sort(s.begin(), s.end(), std::greater<double>());
	double sum = s[0]+s[1]+s[2];
	c[0] = (s[0]-s[1])/sum;
	c[1] = 2.0*(s[1]-s[2])/sum;
	c[2] = 3.0*s[2]/sum;
}

//int CAABB::CalcDissimilarity(const CAABB &bb, double &d, double dDMax/*=0.0*/, char flags/*=BB_SIMI_ALL*/) const
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

double CAABB::CalcHausdorffDist(const CAABB &bb) const
{
	double d1(0);
	for (unsigned int i=0; i<vp.size(); i++) {
		double dd(std::numeric_limits<double>::max());
		for (unsigned int j=0; j<bb.vp.size(); j++) {
			dd = std::min(dd, vp[i].distance(bb.vp[j]));
		}
		d1 = std::max(d1, dd);
	}
	double d2(0);
	for (unsigned int i=0; i<bb.vp.size(); i++) {
		double dd(std::numeric_limits<double>::max());
		for (unsigned int j=0; j<vp.size(); j++) {
			dd = std::min(dd, bb.vp[i].distance(vp[j]));
		}
		d2 = std::max(d2, dd);
	}
	return std::max(d1, d2);
}

int CAABB::CalcDissimilarity(const CAABB &bb, double dDMax, double &d) const
{
	d = CalcHausdorffDist(bb)/dDMax;
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
	double dsl = Acos(MathLib::Abs(la1.dot(la2)))/90.0 * (1.0-exp(-(dcl*0.5+dl1*0.25+dl2*0.25)*s));
	double dsp = Acos(MathLib::Abs(sa1.dot(sa2)))/90.0 * (1.0-exp(-(dcp*0.5+dp1*0.25+dp2*0.25)*s));
	double ds = std::min(dsl,dsp);*/
	
//	d = dd*wd + ds*ws;
	return 0;
}

void CAABB::WriteData(FILE *fp)
{
	fprintf(fp, "%f %f %f %f %f %f\n",
		cent[0], cent[1], cent[2],
		size[0], size[1], size[2]);
}

void CAABB::WriteData(std::ofstream &ofs)
{
	ofs << cent[0] << " " << cent[1] << " " << cent[2] << " "
		<< size[0] << " " << size[1] << " " << size[2] << std::endl;
}

void CAABB::ReadData(std::ifstream &ifs)
{
	ifs >> cent[0] >> cent[1] >> cent[2]
		>> size[0] >> size[1] >> size[2];
	updateDataAS();
}

void CAABB::ReadData(std::ifstream &ifs, const MathLib::Vector3 &uc, double us)
{
	ifs >> cent[0] >> cent[1] >> cent[2]
		>> size[0] >> size[1] >> size[2];
	updateDataAS();
	Unitize(uc,us);
}

double CAABB::Vol(void) const
{
	return vol;
}

const MathLib::Vector3& CAABB::C(void) const
{
	return cent;
}

const double& CAABB::S(int i) const
{
	return size[i];
}
const MathLib::Vector3& CAABB::S(void) const
{
	return size;
}
const double& CAABB::HS(int i) const
{
	return hsize[i];
}
const MathLib::Vector3& CAABB::HS(void) const
{
	return hsize;
}

const MathLib::Vector3& CAABB::V(int i) const
{
	return vp[i];
}

const std::vector<MathLib::Vector3>& CAABB::V(void) const
{
	return vp;
}

std::vector<MathLib::Vector3>& CAABB::V(void)
{
	return vp;
}

MathLib::Vector3& CAABB::V(int i)
{
	return vp[i];
}

void CAABB::Face(int ai, int d, std::vector<int> &fv)
{
	fv.resize(4, 0);
	switch (ai) {
			case 0:
				if (d >= 0) {
					fv[0] = 0;
					fv[1] = 1;
					fv[2] = 2;
					fv[3] = 3;
				} else {
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
				} else {
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
				} else {
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

CAABB& CAABB::operator=(const CAABB& other) {
	size = other.size;
	hsize = other.hsize;
	cent = other.cent;
	vp = other.vp;
	vol = other.vol;
	dl = other.dl;
	ca = other.ca;
	cas = other.cas;
	as = other.as;
	ap = other.ap;
	al = other.al;
	return *this;
}


void CAABB::DrawBox(bool b3DEdge, bool bFace, bool bGraph, bool bShowStat, bool bHighL, GLfloat *fColor) const
{
	GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GLfloat green[] = { 0.5f, 1.0f, 0.5f, 1.0f };
	GLfloat axis_col[3][4] = { { 1.0f, 0.3f, 0.3f, 1.0f }, { 0.2f, 0.8f, 0.2f, 1.0f }, { 0.1f, 0.1f, 1.0f, 1.0f } };
	GLfloat yellow[] = { 0.9f, 0.9f, 0.0f, 1.0f };

	glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_HINT_BIT | GL_LINE_BIT | GL_CURRENT_BIT);
	{
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
			if (fColor == NULL) {
				glColor3f(green[0], green[1], green[2]);
			}
			else {
				glColor3f(fColor[0], fColor[1], fColor[2]);
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
	}

	glPopAttrib();
}

inline bool CAABB::IsInside(const MathLib::Vector3 &p) const
{
	MathLib::Vector3 dv = p-vp[6];
	if (dv[0]<0.0 || dv[0]>size[0]) {
		return false;
	}
	if (dv[1]<0.0 || dv[1]>size[1]) {
		return false;
	}
	if (dv[2]<0.0 || dv[2]>size[2]) {
		return false;
	}
	return true;
}

bool CAABB::IsIntersect(const CAABB &aabb) const
{
	if ( MathLib::Abs(cent[0]-aabb.cent[0]) > (hsize[0]+aabb.hsize[0]) ) return false;
	if ( MathLib::Abs(cent[1]-aabb.cent[1]) > (hsize[1]+aabb.hsize[1]) ) return false;
	if ( MathLib::Abs(cent[2]-aabb.cent[2]) > (hsize[2]+aabb.hsize[2]) ) return false;
	return true;
}

bool CAABB::IsIntersect(const CAABB &aabb, double epsilon) const
{
	if ( (MathLib::Abs(cent[0]-aabb.cent[0]) - (hsize[0]+aabb.hsize[0])) > epsilon ) return false;
	if ( (MathLib::Abs(cent[1]-aabb.cent[1]) - (hsize[1]+aabb.hsize[1])) > epsilon ) return false;
	if ( (MathLib::Abs(cent[2]-aabb.cent[2]) - (hsize[2]+aabb.hsize[2])) > epsilon ) return false;
	return true;
}

double CAABB::CalcDistance( const CAABB &bb ) const
{
	double dist = 0;

	for (int i = 0; i < 3; i++)
	{
		// if curr max < tar min, then add dist to sum
		if (vp[0][i] < bb.vp[6][i])
		{
			dist +=  (bb.vp[6][i] - vp[0][i])*(bb.vp[6][i] - vp[0][i]);
		}

		// if curr min > tar max, then add dist to sum
		else if (vp[6][i] > bb.vp[0][i])
		{
			dist += (vp[6][i] - bb.vp[0][i])*(vp[6][i] - bb.vp[0][i]);
		}
	}

	dist = sqrt(dist);
	return dist;
}

bool CAABB::isInXYRange(const std::vector<double> &rangeVals, double boundTh)
{
	double xmin, ymin, xmax, ymax;

	xmin = vp[6][0];
	ymin = vp[6][1];

	xmax = vp[0][0];
	ymax = vp[0][1];

	if (xmin < rangeVals[0] - boundTh || ymin<rangeVals[2] -boundTh)
	{
		return false;
	}

	if (xmax>rangeVals[1] + boundTh || ymax>rangeVals[3] + boundTh)
	{
		return false;
	}

	return true;
}
