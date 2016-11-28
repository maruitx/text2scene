
// ------------------------------------------------------------
// Math Library 
// ------------------------------------------------------------
// Chris Stephenson
// ------------------------------------------------------------
// This library is mostly pieced together from other inseficiant 
// free sources I have found (mostly www.flipcode.com code of the day section)
// All angles in degrees.(radians suck!)
// Everything else is basic.
// 
// For the Matrix classes, transfomations can be writen like this:
// {
//    Matrix4 m;
//    m.settranslate(1, -2, 3).rotate(40, 1, 0.5, 4).scale(0.5, 2, 1.3);
// }
// Instead of this:
// {
//    Matrix4 m;
//    m.settranslate(1, -2, 3)
//    m.rotate(40, 1, 0.5, 4)
//    m.scale(0.5, 2, 1.3);
// }
// ------------------------------------------------------------

#pragma once

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits>
#include <vector>
//#include "../../GlobalFunc.h"
// ------------------------------------------------------------
// constants
// ------------------------------------------------------------

// PI & conversions
namespace MathLib{

const double ML_2_PI			= 6.283185307179586476;
const double ML_PI				= 3.141592653589793238;
const double ML_DEG_TO_RAD		= 0.017453292519943295;
const double ML_RAD_TO_DEG		= 57.29577951308232286;
const double ML_PI_2			= 1.570796326794896558;
const double ML_PI_4			= 0.785398163397448279;

// tolerance values

const float ML_L_TOL		= 0.000001f;
const float ML_TOL			= 0.000100f;
const float ML_H_TOL		= 0.010000f;

const double ML_D_L_TOL		= 0.0000000001;
const double ML_D_TOL		= 0.0000000010;
const double ML_D_H_TOL		= 0.0000000100;

const double RAD_TO_DEG = 57.295779513;
const double DEG_TO_RAD = 0.0174532925;

// ------------------------------------------------------------
// basic functions
// ------------------------------------------------------------

inline double Sign(double f){ return f > 0 ? 1 : -1;}
inline void Clampf(double &f, double min, double max){ f = min > f ? min : (max > f ? f : max); }
inline void Clamp(int &i, int min, int max){ i = min > i ? min : (max > i ? i : max); }
inline double Lerp(double f, double from, double to){ return from + (to - from) * f;}
inline double Min(double f1, double f2){ return f1 < f2 ? f1 : f2;}
inline double Max(double f1, double f2){ return f1 > f2 ? f1 : f2;}
inline double Abs(double f){return f > 0 ? f : -f;}
inline double Sin(double f){ return sin(f*ML_DEG_TO_RAD); }
inline double Cos(double f){ return cos(f*ML_DEG_TO_RAD); }
inline double Tan(double f){ return tan(f*ML_DEG_TO_RAD); }
inline double Asin(double f){ Clampf(f,-1.0,1.0); return asin(f) * ML_RAD_TO_DEG;}
inline double AsinR(double f){ Clampf(f,-1.0,1.0); return asin(f);}
inline double Acos(double f){ Clampf(f,-1.0,1.0); return acos(f) * ML_RAD_TO_DEG;}
inline double AcosR(double f){ Clampf(f,-1.0,1.0); return acos(f);}
inline double Atan(double f){ return atan(f) * ML_DEG_TO_RAD;}
inline double AtanR(double f){ return atan(f);}
inline double Degree(double f){ return f * ML_RAD_TO_DEG;}
inline double Radians(double f){ return f * ML_DEG_TO_RAD;}
inline double Random(){ return (double)(rand()) / (double)(RAND_MAX);}
inline double Random(double f){ return Random() * f;}
inline double Random(double f1, double f2){ return Random() * (f2 - f1) + f1;}
inline void SeedRand(unsigned int seed){srand(seed);};
inline bool Coin(void) { return ((rand()%2)==0); }

inline bool IsZero(double f){ return fabs(f)<=std::numeric_limits<double>::epsilon();}
inline bool IsZero(double f, double tol){ return fabs(f)<tol;}
inline bool IsEqual(double f1, double f2){ return fabs(f1-f2)<=std::numeric_limits<double>::epsilon();}
inline bool IsEqual(double f1, double f2, double tol){ return fabs(f1-f2)<tol;}

void Solve3x3LinSysDoolittle(double a[][3], double b[], double x[]);
void Solve3x3LinSysGaussElim(double a[][3], double b[], double x[]);

// ------------------------------------------------------------
// classes
// ------------------------------------------------------------

class Vector2;
class Matrix3;
class Vector3;
class Matrix4d;
class Plane;

// ------------------------------------------------------------
extern const Vector3 ML_A[3];
extern const Vector3 ML_O;
extern const Vector3 ML_AX;
extern const Vector3 ML_AY;
extern const Vector3 ML_AZ;
extern const Matrix3 ML_M3I;


double TriArea(const Vector3 &a, const Vector3 &b, const Vector3 &c);

// ------------------------------------------------------------
// Vector2
// ------------------------------------------------------------

class Vector2
{
public:

	//members

	union 
	{
		struct { double x, y; };
		struct { double v[2]; };
	};

	// constructors

	Vector2();
	Vector2(const double* args);
	Vector2(double _v);
	Vector2(double _x, double _y);
	Vector2(const Vector2& v);
	Vector2(const Vector3& v);

	// set
	void set(double _x, double _y);
	void set(const Vector2& v);
	void set(double* args);
	void setzero(void) { v[0]=v[1]=0.0; }

	// operators
	Vector2 operator *(double f) const;
	Vector2 operator /(double f) const;
	Vector2 operator +(const Vector2& v) const;
	Vector2 operator -(const Vector2& v) const;
	Vector2 operator -() const;
	Vector2& operator *=(double f);
	Vector2& operator /=(double f);
	Vector2& operator +=(const Vector2& v);
	Vector2& operator -=(const Vector2& v);
	int operator==(const Vector2& v);
	int operator!=(const Vector2& v);
	Vector2& operator=(const Vector2& v);
	Vector2& operator=(const Vector3& v);
	Vector2& operator&(const Vector2& v); // reverse copy

	// Data access using indices
	double&       operator[](int i)       { return (v[i]); }
	const double& operator[](int i) const { return (v[i]); }

	// linear algebra
	double squaremagnitude() const;
	double magnitude() const;
	double squaredistance(const Vector2& v) const;
	double distance(const Vector2& v) const;
	void normalize(); 
	void normalize(const Vector2& v);
	double dot(const Vector2& v) const;
};

// ------------------------------------------------------------
// Matrix2
// 0 1
// 2 3
// ------------------------------------------------------------

class Matrix2
{
public:

	// members

	union
	{
		double M[4];
		double m[2][2];
	};

	static double Identity_Matrix[];

	Matrix2();
	Matrix2(const double* other);
	Matrix2(const Matrix2& other);

	Vector2 mul(const Vector2& v) const;
};

// ------------------------------------------------------------
// Matrix3
// 0 1 2
// 3 4 5
// 6 7 8
// ------------------------------------------------------------

class Matrix3
{
public:

	// members

	union
	{
		double M[9];
		double m[3][3];
	};

	static double Identity_Matrix[];

	// constructors

	Matrix3();
	Matrix3(const double* other);
	Matrix3(const Matrix3& other);
	Matrix3(const Matrix4d& other);

	// set

	Matrix3& operator=(const Matrix3& other);
	void set(const double* other);
	void setcols(const double* other);
	void setorthocol(int c);
	void setorthocol(int c1, int c2);
	void setdiag(const double* diag);
	void setdiag(double d1, double d2, double d3);
	void set(const Matrix3& other);
	void set(const Matrix4d& other);

	// simple methods

	Matrix3& setidentity();
	Matrix3& setzero();
	bool iszero();

	// this *= other
	Matrix3& mul(const Matrix3& other);
	// this = m1 * m2
	Matrix3& mul(const Matrix3& m1, const Matrix3& m2);
	// column c *= f
	Matrix3& mulcol(int c, double f);
	// this = other * f
	Matrix3& mul(const Matrix3& other, double f);
	// vo = this * vi
	void mul(const Vector3& vi, Vector3& vo);
	Vector3 mul(const Vector3& v) const;

	// operators
	Matrix3 operator*(const Matrix3& other) const;
	Matrix3& operator*=(const Matrix3& other);
	Matrix3 operator*(double f) const;
	Matrix3& operator*=(double f);
	Matrix3 operator/(double f) const;
	Matrix3& operator/=(double f);
	Matrix3 operator +(const Matrix3& v) const;
	Matrix3& operator+=(const Matrix3& other);
	Matrix3 operator -(const Matrix3& v) const;
	Matrix3& operator-=(const Matrix3& other);
	int operator==(const Matrix3& m);
	int operator!=(const Matrix3& m);
	friend int operator==(const Matrix3& m1, const Matrix3& m2);
	friend int operator!=(const Matrix3& m1, const Matrix3& m2) { return !(m1 == m2); }
	
	// transformation methods

	Matrix3& translate(double tx, double ty);
	Matrix3& settranslate(double tx, double ty);
	Matrix3& rotate(double r);
	Matrix3& setrotate(double r);
	Matrix3& scale(double sx, double sy);
	Matrix3& setscale(double sx, double sy);
	bool isrotate();
	//////////////////////////////////////////////////////////////////////////
	Matrix3& setrotate3d(double angle, const Vector3& axis);
	Matrix3& setreflect3d(const Vector3& p, const Vector3& n);
	Matrix3& setreflect3d(const Vector3& n);
	void getaxisangle3d(double& angle, Vector3& axis);

	// Actions with Vector2

	Vector2 transform(const Vector2& v);

	static double det2x2(double a, double b, double c, double d);
	static double det3x3(double a1, double a2, double a3,
						double b1, double b2, double b3,
						double c1, double c2, double c3);
	static double det3x3(const double *m);

	Matrix3& adjoint();
	Matrix3& adjoint(const Matrix3& other);
	Matrix3& transpose();
	Matrix3& transpose(const Matrix3& other);
	Matrix3& invert_exp();
	Matrix3& invert_GE();
	Matrix3& invert_D();
	Matrix3& invert(const Matrix3& other);
	void squarecolsymm(double *symm);
	void squareupper(Matrix3& out);

	double det() const;
	double trace() const;
};

// ------------------------------------------------------------
// Vector3
// ------------------------------------------------------------

class Vector3
{
public:

	// members

	union 
	{
		struct { double x, y, z; };
		struct { double v[3]; };
	};

	// constructors

	Vector3();
	Vector3(const double* args);
	Vector3(double _v);
	Vector3(double _x, double _y, double _z);
	Vector3(const Vector3& v);
	Vector3(const Vector2& v);

	// set

	void set(const double* args);
	void set(double _x, double _y, double _z);
	void set(const Vector3& v);
	void set(const Vector3* v);
	// Per coordinate (explicit inline functions)
	void setx(double newX) { v[0] = newX; }
	void sety(double newY) { v[1] = newY; }
	void setz(double newZ) { v[2] = newZ; }
	void setzero(void) { v[0]=v[1]=v[2]=0.0; }

	// Data access using indices
	double&       operator[](int i)       { return (v[i]); }
	const double& operator[](int i) const { return (v[i]); }

	// operators

	Vector3 operator *(double f) const;
	Vector3 operator /(double f) const;
	Vector3 operator +(const Vector3& v) const;
	Vector3 operator -(const Vector3& v) const;
	Vector3 operator -() const;
	Matrix3 operator *(const Vector3& v) const;
	Vector3& operator *=(double f);
	Vector3& operator /=(double f);
	Vector3& operator +=(const Vector3& v);
	Vector3& operator -=(const Vector3& v);
	int operator==(const Vector3& v);
	int operator!=(const Vector3& v);
	friend int operator==(const Vector3& v1, const Vector3& v2);
	friend int operator!=(const Vector3& v1, const Vector3& v2) { return !(v1 == v2); }
	Vector3& operator=(const Vector3& v);
	Vector3& operator&(const Vector3& v); // reverse copy

	// linear algebra
	double squaremagnitude() const;
	double magnitude() const;
	double squaredistance(const Vector3& v) const;
	double distance(const Vector3& v) const;
	void normalize(); 
	void normalize(const Vector3& v);
	double dot(const Vector3& v) const;
	Vector3 cross(const Vector3& v) const;
	Matrix3 square() const;
	void getorthovecs(Vector3 &v1, Vector3 &v2);
};
// ------------------------------------------------------------
// Matrix4
//  0  1  2  3
//  4  5  6  7
//  8  9 10 11
// 12 13 14 15
// ------------------------------------------------------------

class Matrix4d
{
public:

	// members

	union
	{
		double M[16];
		double m[4][4];
	};

	static double Identity_Matrix[];
	static double Orientation_Switch_Matrix[];
	static double Perspective_Matrix[];

public:

	// constructors

	Matrix4d();
	Matrix4d(const float* other);
	Matrix4d(const double* other);
	Matrix4d(const Matrix4d& other);
	Matrix4d(const std::vector<float> &other);
	Matrix4d(const std::vector<double> &other);

	// set

	Matrix4d& operator=(const Matrix4d& other);
	void set(const float* other);
	void set(const double* other);
	void set(const Matrix4d& other);

	// Simple methods

	Matrix4d& setidentity();
	Matrix4d& setzero();
	Matrix4d& setperspective();
	Matrix4d& setswitchorientation();

	// this *= other
	Matrix4d& mul_r(const Matrix4d& other);
	// this *= other
	Matrix4d& mul_l(const Matrix4d& other);
	// this = m1 * m2
	Matrix4d& mul(const Matrix4d& m1, const Matrix4d& m2);

	// Operators

	Matrix4d& operator*=(const Matrix4d& other);
	Matrix4d operator*(const Matrix4d& other) const;
	Matrix4d operator*(double f) const;
	Matrix4d& operator*=(double f);
	Matrix4d operator/(double f) const;
	Matrix4d& operator/=(double f);

	// Transformation methods

	Matrix4d& settranslate(double tx, double ty, double tz);
	Matrix4d& settranslate(const Vector3 &t);
	Matrix4d& translate(double tx, double ty, double tz);
	Matrix4d& translate(const Vector3 &t);
	Matrix4d& setscale(double sx, double sy, double sz);
	Matrix4d& scale(double sx, double sy, double sz);

	Matrix4d& setrotate(const Matrix3& matrix);
	void getrotate(Matrix3 &matrix) const;
	// rotation around three euler-angles
	Matrix4d& setrotate(const Vector3& r);
	Matrix4d& setrotate(double rx, double ry, double rz);
	Matrix4d& rotate(const Vector3& r);
	Matrix4d& rotate(double rx, double ry, double rz);
	Matrix4d& setreflect(const Vector3& p, const Vector3& n);
	Matrix4d& setreflect(const Vector3& n);

	// rotation euler-angle around axis
	Matrix4d& setrotate(double angle, const Vector3& r);
	Matrix4d& setrotate(double angle, double x, double y, double z);
	Matrix4d& rotate(double angle, const Vector3& r);
	Matrix4d& rotate(double angle, double x, double y, double z);

	// Invert/Transpose

	Matrix4d& adjoint();
	Matrix4d& adjoint(const Matrix4d& other);

	Matrix4d& transpose();
	Matrix4d& transpose(const Matrix4d& other);

	Matrix4d& invert();
	Matrix4d& invert(const Matrix4d& other);

	double det() const;

	// Perpsective

	Matrix4d& setprojection(double fov, double aspect, double znear, double zfar);
	Matrix4d& setothogonal(double znear, double zfar);

	// static
	static double det2x2(double a, double b, double c, double d);
	static double det3x3(double a1, double a2, double a3,
						double b1, double b2, double b3,
						double c1, double c2, double c3);

	// Actions with Vector3
	Vector3 transform(const Vector3& v) const;
};

// ------------------------------------------------------------
// Plane
// ------------------------------------------------------------

class Plane
{
public:

	// members
	union {
		struct { double a, b, c, d; };
		struct { Vector3 n; double d; };
	};

	// constructors
	Plane();
	Plane(const Vector3& _n, double _d);
	Plane(const Vector3& normal, const Vector3& point);
	Plane(double _a, double _b, double _c, double _d);
	Plane(double* args);
	Plane(const Plane& p);

	// set
	void set(const Vector3& _n, double _d);
	void set(double _a, double _b, double _c, double _d);
	void set(double* args);
	void set(const Plane& p);
	void set(const Vector3& normal, const Vector3& point);

	// operators
	Plane& operator=(const Plane& p);

	// interactor with vector3
	double signeddistance(const Vector3& v) const;
	double distance(const Vector3& v) const;
	Vector3 reflect(const Vector3& v);
	Vector3 opoint(void);
	int zproject(const Vector2 &v, double &z);
	int intersect(const Vector3& sp, const Vector3& dir, Vector3 &inters);
};

// ------------------------------------------------------------
// Vector2 functions
// ------------------------------------------------------------

// constructors

inline Vector2::Vector2()
{
	x = y = 0.0f;
}

inline Vector2::Vector2(const double* args)
{
	x = args[0];
	y = args[1];
}

inline Vector2::Vector2(double _v) 
{
	x = _v; 
	y = _v; 
}

inline Vector2::Vector2(double _x, double _y) 
{
	x = _x; 
	y = _y; 
}

inline Vector2::Vector2(const Vector2& v)
{
	x = v.x;
	y = v.y;
}

inline Vector2::Vector2(const Vector3& v)
{
	x = v.x;
	y = v.y;
}

// set

inline void Vector2::set(double _x, double _y)
{
	x = _x; 
	y = _y; 
}

inline void Vector2::set(const Vector2& v)
{
	x = v.x;
	y = v.y;
}

inline void Vector2::set(double* args)
{
	x = args[0];
	y = args[1];
}

// operators

inline Vector2 Vector2::operator *(double f) const
{ 
	Vector2 tmp(*this);
	tmp *= f;
	return tmp;
}

inline Vector2 Vector2::operator /(double f) const
{ 
	Vector2 tmp(*this);
	tmp /= f;
	return tmp;
}

inline Vector2 Vector2::operator +(const Vector2& v) const
{
	Vector2 tmp(*this);
	tmp += v;
	return tmp;
}

inline Vector2 Vector2::operator -(const Vector2& v) const
{
	Vector2 tmp(*this);
	tmp -= v;
	return tmp;
}

inline Vector2 Vector2::operator -() const
{
	Vector2 tmp(*this);
	tmp.x = -tmp.x;
	tmp.y = -tmp.y;
	return tmp;
}

inline Vector2& Vector2::operator *=(double f) 
{ 
	x *= f; 
	y *= f; 
	return *this;
}

inline Vector2& Vector2::operator /=(double f) 
{ 
	double d = 1.0f / f;
	x *= d; 
	y *= d; 
	return *this;
}

inline Vector2& Vector2::operator +=(const Vector2& v) 
{
	x += v.x; 
	y += v.y; 
	return *this;
}

inline Vector2& Vector2::operator -=(const Vector2& v) 
{
	x -= v.x;
	y -= v.y;
	return *this;
}

inline int Vector2::operator==(const Vector2& v) 
{
	return (IsEqual(x,v.x) && IsEqual(y,v.y));
}

inline int Vector2::operator!=(const Vector2& v) 
{ 
	return (!IsEqual(x,v.x) || !IsEqual(y,v.y));
}

inline Vector2& Vector2::operator=(const Vector2& v)
{
	x = v.x;
	y = v.y;
	return *this;
}

inline Vector2& Vector2::operator=(const Vector3& v)
{
	x = v.x;
	y = v.y;
	return *this;
}

inline Vector2& Vector2::operator&(const Vector2& v)
{
	x = -v.x;
	y = -v.y;
	return *this;
}

// linear algebra

inline double Vector2::squaremagnitude() const
{
	return x * x + y * y;
}

inline double Vector2::magnitude() const
{
	return sqrt(squaremagnitude());
}

inline double Vector2::squaredistance(const Vector2& v) const
{
	Vector2 tmp = Vector2(v.x - x, v.y - y);
	return tmp.squaremagnitude();
}

inline double Vector2::distance(const Vector2& v) const
{
	Vector2 tmp = Vector2(v.x - x, v.y - y);
	return tmp.magnitude();
}

inline void Vector2::normalize()
{
	double m = magnitude();
	if(IsZero(m)) return;
	*this /= m;
}

inline void Vector2::normalize(const Vector2& v)
{
	*this = v;
	normalize();
}

inline double Vector2::dot(const Vector2& v) const
{ 
	return x * v.x + y * v.y; 
}

// ------------------------------------------------------------
// Matrix2
// ------------------------------------------------------------

// constructors

inline Matrix2::Matrix2()
{
	for(int i = 0; i < 4; i++) M[i] = 0.0;
}

inline Matrix2::Matrix2(const double* other)
{
	for(int i = 0; i < 4; i++) M[i] = other[i];
}

inline Matrix2::Matrix2(const Matrix2& other)
{
	for(int i = 0; i < 4; i++) M[i] = other.M[i];
}

inline Vector2 Matrix2::mul(const Vector2& v) const
{
	Vector2 tmp;
	tmp[0] = M[0]*v[0] + M[1]*v[1];
	tmp[1] = M[2]*v[0] + M[3]*v[1];
	return tmp;
}

// ------------------------------------------------------------
// Matrix3
// ------------------------------------------------------------

// constructors

inline Matrix3::Matrix3()
{
	for(int i = 0; i < 9; i++) M[i] = 0.0;
}

inline Matrix3::Matrix3(const double* other)
{
	for(int i = 0; i < 9; i++) M[i] = other[i];
}

inline Matrix3::Matrix3(const Matrix3& other)
{
	for(int i = 0; i < 9; i++) M[i] = other.M[i];
}

inline Matrix3::Matrix3(const Matrix4d& other)
{
	M[0] = other.M[0]; M[3] = other.M[1]; M[6] = other.M[2];
	M[1] = other.M[4]; M[4] = other.M[5]; M[7] = other.M[6];
	M[2] = other.M[8]; M[5] = other.M[9]; M[8] = other.M[10];
}

inline Matrix3& Matrix3::operator=(const Matrix3& other)
{
	for(int i = 0; i < 9; i++) M[i] = other.M[i];
	return *this;
}

// set

inline void Matrix3::set(const Matrix4d& other)
{
	M[0] = other.M[0]; M[3] = other.M[1]; M[6] = other.M[2];
	M[1] = other.M[4]; M[4] = other.M[5]; M[7] = other.M[6];
	M[2] = other.M[8]; M[5] = other.M[9]; M[8] = other.M[10];
}

inline void Matrix3::set(const Matrix3& other)
{
	for(int i = 0; i < 9; i++) M[i] = other.M[i];
}

inline void Matrix3::set(const double* other)
{
	for(int i = 0; i < 9; i++) M[i] = other[i];
}

inline void Matrix3::setcols(const double* other)
{
	M[0] = other[0];
	M[3] = other[1];
	M[6] = other[2];
	M[1] = other[3];
	M[4] = other[4];
	M[7] = other[5];
	M[2] = other[6];
	M[5] = other[7];
	M[8] = other[8];
}

inline bool Matrix3::isrotate()
{
	Vector3 c1(m[0][0], m[1][0], m[2][0]);
	Vector3 c2(m[0][1], m[1][1], m[2][1]);
	Vector3 c3(m[0][2], m[1][2], m[2][2]);

	if (!IsZero(c1.dot(c2))) {
		return false;
	}
	if (!IsZero(c1.dot(c3))) {
		return false;
	}
	if (!IsZero(c2.dot(c3))) {
		return false;
	}
	if (!IsEqual(c1.dot(c1), 1.0)) {
		return false;
	}
	if (!IsEqual(c2.dot(c2), 1.0)) {
		return false;
	}
	if (!IsEqual(c3.dot(c3), 1.0)) {
		return false;
	}
	return true;
}

inline void Matrix3::setorthocol(int c1)
{
	int c2, c3;
	if (c1 == 0) {
		c2 = 1;
		c3 = 2;
	} else if (c1 == 1) {
		c2 = 2;
		c3 = 0;
	} else if (c1 == 2) {
		c2 = 0;
		c3 = 1;
	} else {
		return;
	}
	Vector3 v1(m[0][c1], m[1][c1], m[2][c1]);
//	v1.normalize();
	// get a vector v2 orthogonal to v1
	Vector3 v2;
	if (!IsZero(v1[0])) {
		v2.set(-v1[1]/v1[0], 1.0, 0.0);
	} else if (!IsZero(v1[1])) {
		v2.set(1.0, -v1[0]/v1[1], 0.0);
	} else if (!IsZero(v1[2])) {
		v2.set(1.0, 0.0, -v1[0]/v1[2]);
	} else {
		setzero();
		return;
	}
//	v2.normalize();
	// get the 3rd vector orthogonal to v1 and v2
	Vector3 v3 = v1.cross(v2);
	m[0][c2] = v2.x;
	m[1][c2] = v2.y;
	m[2][c2] = v2.z;
	m[0][c3] = v3.x;
	m[1][c3] = v3.y;
	m[2][c3] = v3.z;
}

inline void Matrix3::setorthocol(int c1, int c2)
{
	if (c1<0 || c1>2 || c2<0 || c2>2 || c1==c2) {
		return;
	}
	if (c1 > c2) {
		int t = c1;
		c1 = c2;
		c2 = t;
	}
	Vector3 v1(m[0][c1], m[1][c1], m[2][c1]);
	Vector3 v2(m[0][c2], m[1][c2], m[2][c2]);
	Vector3 v3;
	int c3;
	if (c1==0) {
		if (c2==1) {
			c3 = 2;
			v3 = v1.cross(v2);
		} else { // c2==2
			c3 = 1;
			v3 = v2.cross(v1);
		}
	} else if (c1==1) { // c2==2
		c3 = 0;
		v3 = v1.cross(v2);
	} else {
		return;
	}
	m[0][c3] = v3.x;
	m[1][c3] = v3.y;
	m[2][c3] = v3.z;
}

inline void Matrix3::setdiag(const double* diag)
{
	setzero();
	M[0] = diag[0];
	M[4] = diag[1];
	M[8] = diag[2];
}

inline void Matrix3::setdiag(double d1, double d2, double d3)
{
	setzero();
	M[0] = d1;
	M[4] = d2;
	M[8] = d3;
}

// simple methods

inline Matrix3& Matrix3::setidentity()
{
	for(int i = 0; i < 9; i++) M[i] = Matrix3::Identity_Matrix[i];
	return *this;
}

inline Matrix3& Matrix3::setzero()
{
	for(int i = 0; i < 9; i++) M[i] = 0.0f;
	return *this;
}

inline bool Matrix3::iszero()
{
	for (int i=0; i<9; i++) {
		if (!IsZero(M[i])) {
			return false;
		}
	}
	return true;
}

inline Matrix3& Matrix3::mul(const Matrix3& other)
{
	Matrix3 tmp(*this);
	mul(tmp, other);
	return *this;
}

inline Matrix3& Matrix3::mul(const Matrix3& m1, const Matrix3& m2)
{
	M[0] = m1.M[0]*m2.M[0] + m1.M[1]*m2.M[3] + m1.M[2]*m2.M[6];
	M[1] = m1.M[0]*m2.M[1] + m1.M[1]*m2.M[4] + m1.M[2]*m2.M[7];
	M[2] = m1.M[0]*m2.M[2] + m1.M[1]*m2.M[5] + m1.M[2]*m2.M[8];
	M[3] = m1.M[3]*m2.M[0] + m1.M[4]*m2.M[3] + m1.M[5]*m2.M[6];
	M[4] = m1.M[3]*m2.M[1] + m1.M[4]*m2.M[4] + m1.M[5]*m2.M[7];
	M[5] = m1.M[3]*m2.M[2] + m1.M[4]*m2.M[5] + m1.M[5]*m2.M[8];
	M[6] = m1.M[6]*m2.M[0] + m1.M[7]*m2.M[3] + m1.M[8]*m2.M[6];
	M[7] = m1.M[6]*m2.M[1] + m1.M[7]*m2.M[4] + m1.M[8]*m2.M[7];
	M[8] = m1.M[6]*m2.M[2] + m1.M[7]*m2.M[5] + m1.M[8]*m2.M[8];
	return *this;
}

inline Matrix3& Matrix3::mulcol(int c, double f)
{
	m[0][c] *= f;
	m[1][c] *= f;
	m[2][c] *= f;
	return *this;
}

inline Matrix3& Matrix3::mul(const Matrix3& other, double f)
{
	const double* m = other.M;
	for(int i = 0; i < 9; i++) M[i] = m[i] * f;
	return *this;
}

inline void Matrix3::mul(const Vector3& vi, Vector3& vo)
{
	vo[0] = M[0]*vi[0] + M[1]*vi[1] + M[2]*vi[2];
	vo[1] = M[3]*vi[0] + M[4]*vi[1] + M[5]*vi[2];
	vo[2] = M[6]*vi[0] + M[7]*vi[1] + M[8]*vi[2];
}

inline Vector3 Matrix3::mul(const Vector3& v) const
{
	Vector3 tmp;
	tmp[0] = M[0]*v[0] + M[1]*v[1] + M[2]*v[2];
	tmp[1] = M[3]*v[0] + M[4]*v[1] + M[5]*v[2];
	tmp[2] = M[6]*v[0] + M[7]*v[1] + M[8]*v[2];
	return tmp;
}
// operators

inline Matrix3 Matrix3::operator *(const Matrix3& other) const
{
	Matrix3 tmp(*this);
	tmp *= other;
	return tmp;
}

inline Matrix3 Matrix3::operator +(const Matrix3& other) const
{
	Matrix3 tmp(*this);
	tmp += other;
	return tmp;
}

inline Matrix3 Matrix3::operator -(const Matrix3& other) const
{
	Matrix3 tmp(*this);
	tmp -= other;
	return tmp;
}

inline Matrix3& Matrix3::operator *=(const Matrix3& other)
{
	Matrix3 tmp(*this);
	mul(tmp, other);
	return *this;
}

inline Matrix3& Matrix3::operator +=(const Matrix3& v) 
{
	M[0] += v.M[0];
	M[1] += v.M[1];
	M[2] += v.M[2];
	M[3] += v.M[3];
	M[4] += v.M[4];
	M[5] += v.M[5];
	M[6] += v.M[6];
	M[7] += v.M[7];
	M[8] += v.M[8];
	return *this;
}

inline Matrix3& Matrix3::operator -=(const Matrix3& v) 
{
	M[0] -= v.M[0];
	M[1] -= v.M[1];
	M[2] -= v.M[2];
	M[3] -= v.M[3];
	M[4] -= v.M[4];
	M[5] -= v.M[5];
	M[6] -= v.M[6];
	M[7] -= v.M[7];
	M[8] -= v.M[8];
	return *this;
}

inline Matrix3 Matrix3::operator *(double f) const
{ 
	Matrix3 tmp(*this);
	tmp *= f;
	return tmp;
}

inline Matrix3& Matrix3::operator *=(double f) 
{
	M[0] *= f; M[1] *= f; M[2] *= f;
	M[3] *= f; M[4] *= f; M[5] *= f;
	M[6] *= f; M[7] *= f; M[8] *= f;
	return *this;
}

inline Matrix3 Matrix3::operator /(double f) const
{ 
	Matrix3 tmp(*this);
	tmp /= f;
	return tmp;
}

inline Matrix3& Matrix3::operator /=(double f) 
{
	double rf = 1 / f;
	M[0] *= rf;
	M[1] *= rf;
	M[2] *= rf;
	M[3] *= rf;
	M[4] *= rf;
	M[5] *= rf;
	M[6] *= rf;
	M[7] *= rf;
	M[8] *= rf;
	return *this;
}

inline int Matrix3::operator==(const Matrix3& m) 
{ 
	for (int i=0; i<9; i++) {
		if (!IsEqual(M[i], m.M[i]))
			return false;
	}
	return true;
}

inline int Matrix3::operator!=(const Matrix3& m) 
{ 
	for (int i=0; i<9; i++) {
		if (!IsEqual(M[i], m.M[i]))
			return true;
	}
	return false;
}

inline int operator==(const Matrix3& m1, const Matrix3& m2)
{
	for (int i=0; i<9; i++) {
		if (!IsEqual(m1.M[i], m2.M[i]))
			return false;
	}
	return true;
}

inline void Matrix3::squarecolsymm(double *symm)
{
	// symm: Compute square M^t * M. The upper triangle of the result symmetric matrix 
	//		 stored in column symmetric storage: [m11, m12, m22, m13, m23, m33]
	//		 It must be allocated by caller.
	symm[0] = M[0]*M[0] + M[3]*M[3] + M[6]*M[6];
	symm[1] = M[0]*M[1] + M[3]*M[4] + M[6]*M[7];
	symm[2] = M[1]*M[1] + M[4]*M[4] + M[7]*M[7];
	symm[3] = M[0]*M[2] + M[3]*M[5] + M[6]*M[8];
	symm[4] = M[1]*M[2] + M[4]*M[5] + M[7]*M[8];
	symm[5] = M[2]*M[2] + M[5]*M[5] + M[8]*M[8];
}

inline void Matrix3::squareupper(Matrix3& out)
{
	out.M[0] = M[0]*M[0] + M[3]*M[3] + M[6]*M[6];
	out.M[1] = M[0]*M[1] + M[3]*M[4] + M[6]*M[7];
	out.M[2] = M[0]*M[2] + M[3]*M[5] + M[6]*M[8];
	out.M[4] = M[1]*M[1] + M[4]*M[4] + M[7]*M[7];
	out.M[5] = M[1]*M[2] + M[4]*M[5] + M[7]*M[8];
	out.M[8] = M[2]*M[2] + M[5]*M[5] + M[8]*M[8];
}

// transformation methods

inline Matrix3& Matrix3::translate(double tx, double ty)
{
	M[6] = M[0] * tx + M[3] * ty + M[6];
	M[7] = M[1] * tx + M[4] * ty + M[7];
	M[8] = M[2] * tx + M[5] * ty + M[8];
	return *this;
}

inline Matrix3& Matrix3::settranslate(double tx, double ty)
{
	setidentity();
	M[6] = tx;
	M[7] = ty;
	return *this;
}

inline Matrix3& Matrix3::rotate(double r)
{
	double c = Cos(r), s = Sin(r);
	Matrix3 tmp;
	tmp.setidentity();
	tmp.M[0] = c;
	tmp.M[1] = -s;
	tmp.M[3] = s;
	tmp.M[4] = c;
	*this *= tmp;
	return *this;
}

inline Matrix3& Matrix3::setrotate(double r)
{
	double c = Cos(r), s = Sin(r);
	Matrix3 tmp;
	setidentity();
	M[0] = c;
	M[1] = -s;
	M[3] = s;
	M[4] = c;
	return *this;
}

inline Matrix3& Matrix3::scale(double sx, double sy)
{
	M[0] *= sx; M[3] *= sy;
	M[1] *= sx; M[4] *= sy;
	M[2] *= sx; M[5] *= sy;
	return *this;
}

inline Matrix3& Matrix3::setscale(double sx, double sy)
{
	M[0] = sx; M[4] = sy; M[8] = 1.0f;
	M[1] = M[2] = M[3] = M[5] = M[6] = M[7] = 0.0f;
	return *this;
}

// Actions with Vector2

inline Vector2 Matrix3::transform(const Vector2& v)
{
	return Vector2(v.x * M[0] + v.y * M[3] + M[6], v.x * M[1] + v.y * M[4] + M[7]);
}

inline Matrix3& Matrix3::adjoint()
{
	Matrix3 tmp(*this);
	adjoint(tmp);
	return *this;
}

inline Matrix3& Matrix3::invert_exp()
{
	Matrix3 tmp(*this);
	invert(tmp);
	return *this;
}

inline Matrix3& Matrix3::invert_GE()
{	// NOTE: this function has some problem when the matrix has negative entries
	double b[3];
	double a[3][3];
	Matrix3 x;

	for (int i=0; i<3; i++) {
		for (int j=0; j<3; j++) {
			a[i][j] = m[i][j];
		}
	}
	for (int i=0; i<3; i++) {
		for (int j=0; j<3; j++) {
			b[j] = 0.0;
		}
		b[i] = 1.0;
		Solve3x3LinSysGaussElim(a, b, m[i]);
	}
	return *this;
}

inline Matrix3& Matrix3::invert_D()
{	// NOTE: this function has some problem when the matrix has negative entries
	double b[3];
	double a[3][3];
	Matrix3 x;

	for (int i=0; i<3; i++) {
		for (int j=0; j<3; j++) {
			a[i][j] = m[i][j];
		}
	}
	for (int i=0; i<3; i++) {
		for (int j=0; j<3; j++) {
			b[j] = 0.0;
		}
		b[i] = 1.0;
		Solve3x3LinSysDoolittle(a, b, m[i]);
	}
	return *this;
}

inline Matrix3& Matrix3::transpose()
{
	Matrix3 tmp(*this);
	transpose(tmp);
	return *this;
}

// ------------------------------------------------------------
// Vector3 functions
// ------------------------------------------------------------

// constructors

inline Vector3::Vector3()
{
	x = y = z = 0.0f;
}

inline Vector3::Vector3(const double* args)
{
	x = args[0];
	y = args[1];
	z = args[2];
}

inline Vector3::Vector3(double _x, double _y, double _z) 
{
	x = _x; 
	y = _y; 
	z = _z;
}

inline Vector3::Vector3(double _v)
{
	x = _v; 
	y = _v; 
	z = _v;
}

inline Vector3::Vector3(const Vector3& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
}

inline Vector3::Vector3(const Vector2& v)
{
	x = v.x;
	y = v.y;
	z = 0.0;
}

// set

inline void Vector3::set(const double* args)
{
	x = args[0];
	y = args[1];
	z = args[2];
}

inline void Vector3::set(double _x, double _y, double _z) 
{
	x = _x; 
	y = _y; 
	z = _z;
}

inline void Vector3::set(const Vector3& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
}

inline void Vector3::set(const Vector3* v)
{
	x = v->x;
	y = v->y;
	z = v->z;
}

// operators

inline Vector3 Vector3::operator *(double f) const
{ 
	Vector3 tmp(*this);
	tmp *= f;
	return tmp;
}

inline Vector3 Vector3::operator /(double f) const
{ 
	Vector3 tmp(*this);
	tmp /= f;
	return tmp;
}

inline Vector3 Vector3::operator +(const Vector3& v) const
{
	Vector3 tmp(*this);
	tmp += v;
	return tmp;
}

inline Vector3 Vector3::operator -(const Vector3& v) const
{
	Vector3 tmp(*this);
	tmp -= v;
	return tmp;
}

inline Vector3 Vector3::operator -() const
{
	Vector3 tmp(*this);
	tmp.x = -tmp.x;
	tmp.y = -tmp.y;
	tmp.z = -tmp.z;
	return tmp;
}

inline Vector3& Vector3::operator *=(double f) 
{ 
	x *= f; 
	y *= f; 
	z *= f; 
	return *this;
}

inline Vector3& Vector3::operator /=(double f) 
{ 
	double d = 1.0f / f;
	x *= d; 
	y *= d; 
	z *= d; 
	return *this;
}

inline Vector3& Vector3::operator +=(const Vector3& v) 
{
	x += v.x; 
	y += v.y; 
	z += v.z; 
	return *this;
}

inline Vector3& Vector3::operator -=(const Vector3& v) 
{
	x -= v.x; 
	y -= v.y; 
	z -= v.z; 
	return *this;
}

inline int Vector3::operator==(const Vector3& v) 
{ 
	return (IsEqual(x,v.x) && IsEqual(y,v.y) && IsEqual(z,v.z));
}

inline int Vector3::operator!=(const Vector3& v) 
{ 
	return (!IsEqual(x,v.x) || !IsEqual(y,v.y) || !IsEqual(z,v.z));
}

inline int operator==(const Vector3& v1, const Vector3& v2)
{
	return (IsEqual(v1.x,v2.x) && IsEqual(v1.y,v2.y) && IsEqual(v1.z,v2.z));
}

inline Vector3& Vector3::operator=(const Vector3& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
	return *this;
}

inline Vector3& Vector3::operator&(const Vector3& v)
{
	x = -v.x;
	y = -v.y;
	z = -v.z;
	return *this;
}

// linear algebra

inline double Vector3::squaremagnitude() const
{
	return x * x + y * y + z * z;
}

inline double Vector3::magnitude() const
{
	return sqrt(squaremagnitude());
}

inline double Vector3::squaredistance(const Vector3& v) const
{
	Vector3 tmp = Vector3(v.x - x, v.y - y, v.z - z);
	return tmp.squaremagnitude();
}

inline double Vector3::distance(const Vector3& v) const
{
	Vector3 tmp = Vector3(v.x - x, v.y - y, v.z - z);
	return tmp.magnitude();
}

inline void Vector3::normalize()
{
	double m = magnitude();
	if(IsZero(m)) return;
	*this /= m;
}

inline void Vector3::normalize(const Vector3& v)
{
	*this = v;
	normalize();
}
inline double Vector3::dot(const Vector3& v) const 
{ 
	return x * v.x + y * v.y + z * v.z; 
}

inline Vector3 Vector3::cross(const Vector3& v) const
{ 
	return Vector3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); 
}

inline Matrix3 Vector3::operator *(const Vector3& v) const
{
	Matrix3 tmp;
	tmp.M[0] = x * v.x;
	tmp.M[1] = x * v.y;
	tmp.M[2] = x * v.z;
	tmp.M[3] = y * v.x;
	tmp.M[4] = y * v.y;
	tmp.M[5] = y * v.z;
	tmp.M[6] = z * v.x;
	tmp.M[7] = z * v.y;
	tmp.M[8] = z * v.z;
	return tmp;
}

inline Matrix3 Vector3::square() const
{
	Matrix3 tmp;
	tmp.M[0] = v[0]*v[0];
	tmp.M[1] = tmp.M[3] = v[0]*v[1];
	tmp.M[2] = tmp.M[6] = v[0]*v[2];
	tmp.M[4] = v[1]*v[1];
	tmp.M[5] = tmp.M[7] = v[1]*v[2];
	tmp.M[8] = v[2]*v[2];
	return tmp;
}

inline void Vector3::getorthovecs(Vector3 &v1, Vector3 &v2)
{
	int ai(0);
	double dm(1.0);
	double norm = this->magnitude();
	for (int i=0; i<3; i++) {
		double d = Abs(this->dot(ML_A[i]))/norm;
		if (d < dm) {
			dm = d;
			ai = i;
		}
	}
	v1 = this->cross(ML_A[ai])/norm;
	v2 = this->cross(v1);
}

// ------------------------------------------------------------
// Matrix4
// ------------------------------------------------------------

// constructors

inline Matrix4d::Matrix4d()
{
	for(int i = 0; i < 16; i++) M[i] = 0.0;
}

inline Matrix4d::Matrix4d(const Matrix4d& other)
{
	for(int i = 0; i < 16; i++) M[i] = other.M[i];
}

inline Matrix4d::Matrix4d(const float* other)
{
	for(int i = 0; i < 16; i++) M[i] = other[i];
}

inline Matrix4d::Matrix4d(const double* other)
{
	for(int i = 0; i < 16; i++) M[i] = other[i];
}

inline Matrix4d::Matrix4d(const std::vector<float> &other)
{
	for(int i=0; i<16; i++) M[i] = other[i];
}

inline Matrix4d::Matrix4d(const std::vector<double> &other)
{
	for(int i=0; i<16; i++) M[i] = other[i];
}

// set

inline Matrix4d& Matrix4d::operator=(const Matrix4d& other)
{
	for(int i = 0; i < 16; i++) M[i] = other.M[i];
	return *this;
}

inline void Matrix4d::set(const float* other)
{
	for(int i = 0; i < 16; i++) M[i] = other[i];
}

inline void Matrix4d::set(const double* other)
{
	for(int i = 0; i < 16; i++) M[i] = other[i];
}

inline void Matrix4d::set(const Matrix4d& other)
{
	for(int i = 0; i < 16; i++) M[i] = other.M[i];
}

// Simple methods

inline Matrix4d& Matrix4d::setidentity()
{
	for(int i = 0; i < 16; i++) M[i] = Matrix4d::Identity_Matrix[i];
	return *this;
}

inline Matrix4d& Matrix4d::setzero()
{
	for(int i = 0; i < 16; i++) M[i] = 0.0f;
	return *this;
}

inline Matrix4d& Matrix4d::setswitchorientation()
{
	for(int i = 0; i < 16; i++) M[i] = Matrix4d::Orientation_Switch_Matrix[i];
	return *this;
}

inline Matrix4d& Matrix4d::setperspective()
{
	for(int i = 0; i < 16; i++) M[i] = Matrix4d::Perspective_Matrix[i];
	return *this;
}

inline Matrix4d& Matrix4d::mul_r(const Matrix4d& other)
{
	Matrix4d tmp(*this);
	mul(tmp, other);
	return *this;
}

inline Matrix4d& Matrix4d::mul_l(const Matrix4d& other)
{
	Matrix4d tmp(*this);
	mul(other, tmp);
	return *this;
}

inline Matrix4d& Matrix4d::mul(const Matrix4d& m1, const Matrix4d& m2)
{
	const double *a = m1.M;
	const double *b = m2.M;
	double *prod = M;

	for(int i = 0; i < 4; i++)
	{
		double a0 = a[i];
		double a1 = a[i + 4];
		double a2 = a[i + 8];
		double a3 = a[i + 12];
		prod[i] = a0 * b[0] + a1 * b[1] + a2 * b[2] + a3 * b[3];
		prod[i + 4] = a0 * b[4] + a1 * b[5] + a2 * b[6] + a3 * b[7];
		prod[i + 8] = a0 * b[8] + a1 * b[9] + a2 * b[10] + a3 * b[11];
		prod[i + 12] = a0 * b[12] + a1 * b[13] + a2 * b[14] + a3 * b[15];
	}
	return *this;
}

inline Matrix4d Matrix4d::operator *(double f) const
{ 
	Matrix4d tmp(*this);
	tmp *= f;
	return tmp;
}

inline Matrix4d& Matrix4d::operator *=(double f) 
{
	for (int i=0; i<16; i++) {
		M[i] *= f;
	}
	return *this;
}

inline Matrix4d Matrix4d::operator /(double f) const
{ 
	Matrix4d tmp(*this);
	tmp /= f;
	return tmp;
}

inline Matrix4d& Matrix4d::operator /=(double f) 
{
	double rf = 1 / f;
	for (int i=0; i<16; i++) {
		M[i] *= rf;
	}
	return *this;
}

// Operators

inline Matrix4d& Matrix4d::operator*=(const Matrix4d& other) // right multiply
{
	Matrix4d tmp(*this);
	mul(tmp, other);
	return *this;
}

inline Matrix4d Matrix4d::operator*(const Matrix4d& other) const
{
	Matrix4d tmp;
	tmp.mul(*this, other);
	return tmp;
}

// Transformation methods

inline Matrix4d& Matrix4d::translate(double tx, double ty, double tz)
{
	M[12] = M[0] * tx + M[4] * ty + M[8]  * tz + M[12];
	M[13] = M[1] * tx + M[5] * ty + M[9]  * tz + M[13];
	M[14] = M[2] * tx + M[6] * ty + M[10] * tz + M[14];
	M[15] = M[3] * tx + M[7] * ty + M[11] * tz + M[15];
	return *this;
}

inline Matrix4d& Matrix4d::translate(const Vector3 &t)
{
	M[12] = M[0] * t.x + M[4] * t.y + M[8]  * t.z + M[12];
	M[13] = M[1] * t.x + M[5] * t.y + M[9]  * t.z + M[13];
	M[14] = M[2] * t.x + M[6] * t.y + M[10] * t.z + M[14];
	M[15] = M[3] * t.x + M[7] * t.y + M[11] * t.z + M[15];
	return *this;
}

inline Matrix4d& Matrix4d::settranslate(double tx, double ty, double tz)
{
	setidentity();
	return translate(tx, ty, tz);
}

inline Matrix4d& Matrix4d::settranslate(const Vector3 &t)
{
	setidentity();
	return translate(t.x, t.y, t.z);
}

inline Matrix4d& Matrix4d::scale(double sx, double sy, double sz)
{
	M[0] *= sx;
	M[5] *= sy;
	M[10] *= sz;
	return *this;
}

inline Matrix4d& Matrix4d::setscale(double sx, double sy, double sz)
{
	setidentity();
	return scale(sx, sy, sz);
}

inline Matrix4d& Matrix4d::setrotate(const Matrix3 &matrix)
{
	M[0] = matrix.M[0];
	M[1] = matrix.M[3];
	M[2] = matrix.M[6];
	M[3] = 0.0;
	M[4] = matrix.M[1];
	M[5] = matrix.M[4];
	M[6] = matrix.M[7];
	M[7] = 0.0;
	M[8] = matrix.M[2];
	M[9] = matrix.M[5];
	M[10] = matrix.M[8];
	M[11] = 0.0;
	M[12] = 0.0;
	M[13] = 0.0;
	M[14] = 0.0;
	M[15] = 1.0;
	return *this;
}

inline void Matrix4d::getrotate(Matrix3 &matrix) const
{
	matrix.M[0] = M[0];
	matrix.M[3] = M[1];
	matrix.M[6] = M[2];
	matrix.M[1] = M[4];
	matrix.M[4] = M[5];
	matrix.M[7] = M[6];
	matrix.M[2] = M[8];
	matrix.M[5] = M[9];
	matrix.M[8] = M[10];
}

// rotation around three euler-angles

inline Matrix4d& Matrix4d::setrotate(const Vector3& r)
{
	return setrotate(r.x, r.y, r.z);
}

inline Matrix4d& Matrix4d::rotate(const Vector3& r)
{
	return rotate(r.x, r.y, r.z);
}

inline Matrix4d& Matrix4d::rotate(double rx, double ry, double rz)
{
	Matrix4d tmp;
	tmp.setrotate(rx, ry, rz);
	mul_l(tmp);
	return *this;
}

// rotation euler-angle around axis

inline Matrix4d& Matrix4d::setrotate(double angle, const Vector3& r)
{
	return setrotate(angle, r.x, r.y, r.z);
}

inline Matrix4d& Matrix4d::rotate(double angle, const Vector3& r)
{
	return rotate(angle, r.x, r.y, r.z);
}

inline Matrix4d& Matrix4d::rotate(double angle, double x, double y, double z)
{
	Matrix4d tmp;
	tmp.setrotate(angle, x, y, z);
	mul_l(tmp);
	return *this;
}

// Invert/Transpose
inline Matrix4d& Matrix4d::adjoint()
{
	Matrix4d tmp(*this);
	adjoint(tmp);
	return *this;
}

inline Matrix4d& Matrix4d::transpose()
{
	Matrix4d tmp(*this);
	transpose(tmp);
	return *this;
}

inline Matrix4d& Matrix4d::invert()
{
	Matrix4d tmp(*this);
	invert(tmp);
	return *this;
}

// Actions with Vector3

inline Vector3 Matrix4d::transform(const Vector3& v) const
{
	return Vector3(v.x * M[0] + v.y * M[4] + v.z * M[8] + M[12],
				   v.x * M[1] + v.y * M[5] + v.z * M[9] + M[13],
				   v.x * M[2] + v.y * M[6] + v.z * M[10] + M[14]);
}


// ------------------------------------------------------------
// Plane functions
// ------------------------------------------------------------

// constructors

inline Plane::Plane()
{
	a = b = c = d = 0.0f;
}

inline Plane::Plane(const Vector3& _n, double _d)
{
	a = _n.x;
	b = _n.y;
	c = _n.z;
	double m = n.magnitude();
	n /= m;
	d = _d/m;
}

inline Plane::Plane(const Vector3& normal, const Vector3& point)
{
	Vector3 n(normal);
	n.normalize();
	d = -a*point.x-b*point.y-c*point.z;
}

inline Plane::Plane(double _a, double _b, double _c, double _d)
{
	a = _a;
	b = _b;
	c = _c;
	double m = n.magnitude();
	n /= m;
	d = _d/m;
}

inline Plane::Plane(double* args)
{
	a = args[0];
	b = args[1];
	c = args[2];
	double m = n.magnitude();
	n /= m;
	d = args[3]/m;
}

inline Plane::Plane(const Plane& p)
{
	a = p.a;
	b = p.b;
	c = p.c;
	d = p.d;
}

inline void Plane::set(const Vector3& p, const Vector3& _n)
{
	n.set(_n);
	n.normalize();
	d = -a*p.x-b*p.y-c*p.z;
}

inline void Plane::set(const Vector3& _n, double _d)
{
	a = _n.x;
	b = _n.y;
	c = _n.z;
	double m = n.magnitude();
	n /= m;
	d = _d/m;
}

// set

inline void Plane::set(double _a, double _b, double _c, double _d)
{
	a = _a; 
	b = _b; 
	c = _c; 
	double m = n.magnitude();
	n /= m;
	d = _d/m;
}

inline void Plane::set(double* args)
{
	a = args[0];
	b = args[1];
	c = args[2];
	double m = n.magnitude();
	n /= m;
	d = args[3]/m;
}

inline void Plane::set(const Plane& p)
{
	a = p.a; 
	b = p.b; 
	c = p.c; 
	d = p.d;
}

// operators

inline Plane& Plane::operator=(const Plane& p)
{
	a = p.a; b = p.b; c = p.c; d = p.d;
	return *this;
}

inline double Plane::signeddistance(const Vector3& v) const
{
	return n.dot(v)-d;
}

inline double Plane::distance(const Vector3& v) const
{
	return Abs(n.dot(v)-d);
}

inline Vector3 Plane::reflect(const Vector3& v)
{
	return v - n * 2 * signeddistance(v);
}

inline int Plane::zproject(const Vector2 &v, double &z)
{
	if (IsZero(c)) {
		return -1;
	}
	z = (-d-a*v.x-b*v.y)/c;
	return 0;
}

inline Vector3 Plane::opoint(void)
{
	return n*-d;
}

inline int Plane::intersect(const Vector3& sp, const Vector3& dir, Vector3 &inters)
{
	Vector3 dd(dir);
	dd.normalize();
	double det = dd.dot(n);
	if (IsZero(det)) {	// input line is parallel to the plane
		if (IsZero(sp.x*a+sp.y*b+sp.z*c+d)) {	// in the plane
			inters = sp;
			return 1;
		} else {
			return -1;
		}
	}
	double t = -(sp.x*a+sp.y*b+sp.z*c+d);
	t /= det;
	inters = sp + dd*t;
	return 0;
}

}