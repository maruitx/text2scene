#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include "../utilities/mathlib.h"

/*!
**
** Copyright (c) 2009 by John W. Ratcliff mailto:jratcliffscarab@gmail.com
**
** The MIT license:
**
** Permission is hereby granted, FREE of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is furnished
** to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.

** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
** WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/


#include "BestFit.h"

namespace BEST_FIT
{

const REAL FM_PI = 3.1415926535897932384626433832795028841971693993751;
const REAL FM_DEG_TO_RAD = ((2.0 * FM_PI) / 360.0);
const REAL FM_RAD_TO_DEG = (360.0 / (2.0 * FM_PI));

void fm_identity(REAL matrix[16]) // set 4x4 matrix to identity.
{
  matrix[0*4+0] = 1;
  matrix[1*4+1] = 1;
  matrix[2*4+2] = 1;
  matrix[3*4+3] = 1;

  matrix[1*4+0] = 0;
  matrix[2*4+0] = 0;
  matrix[3*4+0] = 0;

  matrix[0*4+1] = 0;
  matrix[2*4+1] = 0;
  matrix[3*4+1] = 0;

  matrix[0*4+2] = 0;
  matrix[1*4+2] = 0;
  matrix[3*4+2] = 0;

  matrix[0*4+3] = 0;
  matrix[1*4+3] = 0;
  matrix[2*4+3] = 0;

}



void  fm_matrixMultiply(const REAL *pA,const REAL *pB,REAL *pM)
{
  REAL a = pA[0*4+0] * pB[0*4+0] + pA[0*4+1] * pB[1*4+0] + pA[0*4+2] * pB[2*4+0] + pA[0*4+3] * pB[3*4+0];
  REAL b = pA[0*4+0] * pB[0*4+1] + pA[0*4+1] * pB[1*4+1] + pA[0*4+2] * pB[2*4+1] + pA[0*4+3] * pB[3*4+1];
  REAL c = pA[0*4+0] * pB[0*4+2] + pA[0*4+1] * pB[1*4+2] + pA[0*4+2] * pB[2*4+2] + pA[0*4+3] * pB[3*4+2];
  REAL d = pA[0*4+0] * pB[0*4+3] + pA[0*4+1] * pB[1*4+3] + pA[0*4+2] * pB[2*4+3] + pA[0*4+3] * pB[3*4+3];

  REAL e = pA[1*4+0] * pB[0*4+0] + pA[1*4+1] * pB[1*4+0] + pA[1*4+2] * pB[2*4+0] + pA[1*4+3] * pB[3*4+0];
  REAL f = pA[1*4+0] * pB[0*4+1] + pA[1*4+1] * pB[1*4+1] + pA[1*4+2] * pB[2*4+1] + pA[1*4+3] * pB[3*4+1];
  REAL g = pA[1*4+0] * pB[0*4+2] + pA[1*4+1] * pB[1*4+2] + pA[1*4+2] * pB[2*4+2] + pA[1*4+3] * pB[3*4+2];
  REAL h = pA[1*4+0] * pB[0*4+3] + pA[1*4+1] * pB[1*4+3] + pA[1*4+2] * pB[2*4+3] + pA[1*4+3] * pB[3*4+3];

  REAL i = pA[2*4+0] * pB[0*4+0] + pA[2*4+1] * pB[1*4+0] + pA[2*4+2] * pB[2*4+0] + pA[2*4+3] * pB[3*4+0];
  REAL j = pA[2*4+0] * pB[0*4+1] + pA[2*4+1] * pB[1*4+1] + pA[2*4+2] * pB[2*4+1] + pA[2*4+3] * pB[3*4+1];
  REAL k = pA[2*4+0] * pB[0*4+2] + pA[2*4+1] * pB[1*4+2] + pA[2*4+2] * pB[2*4+2] + pA[2*4+3] * pB[3*4+2];
  REAL l = pA[2*4+0] * pB[0*4+3] + pA[2*4+1] * pB[1*4+3] + pA[2*4+2] * pB[2*4+3] + pA[2*4+3] * pB[3*4+3];

  REAL m = pA[3*4+0] * pB[0*4+0] + pA[3*4+1] * pB[1*4+0] + pA[3*4+2] * pB[2*4+0] + pA[3*4+3] * pB[3*4+0];
  REAL n = pA[3*4+0] * pB[0*4+1] + pA[3*4+1] * pB[1*4+1] + pA[3*4+2] * pB[2*4+1] + pA[3*4+3] * pB[3*4+1];
  REAL o = pA[3*4+0] * pB[0*4+2] + pA[3*4+1] * pB[1*4+2] + pA[3*4+2] * pB[2*4+2] + pA[3*4+3] * pB[3*4+2];
  REAL p = pA[3*4+0] * pB[0*4+3] + pA[3*4+1] * pB[1*4+3] + pA[3*4+2] * pB[2*4+3] + pA[3*4+3] * pB[3*4+3];

  pM[0] = a;  pM[1] = b;  pM[2] = c;  pM[3] = d;

  pM[4] = e;  pM[5] = f;  pM[6] = g;  pM[7] = h;

  pM[8] = i;  pM[9] = j;  pM[10] = k;  pM[11] = l;

  pM[12] = m;  pM[13] = n;  pM[14] = o;  pM[15] = p;

}

void fm_matrixToQuat(const REAL *matrix,REAL *quat) // convert the 3x3 portion of a 4x4 matrix into a quaterion as x,y,z,w
{

	REAL tr = matrix[0*4+0] + matrix[1*4+1] + matrix[2*4+2];

	// check the diagonal

	if (tr > 0.0f )
	{
		REAL s = (REAL) sqrt ( (double) (tr + 1.0f) );
		quat[3] = s * 0.5f;
		s = 0.5f / s;
		quat[0] = (matrix[1*4+2] - matrix[2*4+1]) * s;
		quat[1] = (matrix[2*4+0] - matrix[0*4+2]) * s;
		quat[2] = (matrix[0*4+1] - matrix[1*4+0]) * s;

	}
	else
	{
		// diagonal is negative
		int nxt[3] = {1, 2, 0};
		REAL  qa[4];

		int i = 0;

		if (matrix[1*4+1] > matrix[0*4+0]) i = 1;
		if (matrix[2*4+2] > matrix[i*4+i]) i = 2;

		int j = nxt[i];
		int k = nxt[j];

		REAL s = sqrt ( ((matrix[i*4+i] - (matrix[j*4+j] + matrix[k*4+k])) + 1.0f) );

		qa[i] = s * 0.5f;

		if (s != 0.0f ) s = 0.5f / s;

		qa[3] = (matrix[j*4+k] - matrix[k*4+j]) * s;
		qa[j] = (matrix[i*4+j] + matrix[j*4+i]) * s;
		qa[k] = (matrix[i*4+k] + matrix[k*4+i]) * s;

		quat[0] = qa[0];
		quat[1] = qa[1];
		quat[2] = qa[2];
		quat[3] = qa[3];
	}


}


void fm_getTranslation(const REAL *matrix,REAL *t)
{
	t[0] = matrix[3*4+0];
	t[1] = matrix[3*4+1];
	t[2] = matrix[3*4+2];
}


void  fm_rotate(const REAL matrix[16],const REAL v[3],REAL t[3]) // rotate and translate this point
{
  if ( matrix )
  {
    REAL tx = (matrix[0*4+0] * v[0]) +  (matrix[1*4+0] * v[1]) + (matrix[2*4+0] * v[2]);
    REAL ty = (matrix[0*4+1] * v[0]) +  (matrix[1*4+1] * v[1]) + (matrix[2*4+1] * v[2]);
    REAL tz = (matrix[0*4+2] * v[0]) +  (matrix[1*4+2] * v[1]) + (matrix[2*4+2] * v[2]);
    t[0] = tx;
    t[1] = ty;
    t[2] = tz;
  }
  else
  {
    t[0] = v[0];
    t[1] = v[1];
    t[2] = v[2];
  }
}

void fm_inverseRT(const REAL matrix[16],const REAL pos[3],REAL t[3]) // inverse rotate translate the point.
{

	REAL _x = pos[0] - matrix[3*4+0];
	REAL _y = pos[1] - matrix[3*4+1];
	REAL _z = pos[2] - matrix[3*4+2];

	// Multiply inverse-translated source vector by inverted rotation transform

	t[0] = (matrix[0*4+0] * _x) + (matrix[0*4+1] * _y) + (matrix[0*4+2] * _z);
	t[1] = (matrix[1*4+0] * _x) + (matrix[1*4+1] * _y) + (matrix[1*4+2] * _z);
	t[2] = (matrix[2*4+0] * _x) + (matrix[2*4+1] * _y) + (matrix[2*4+2] * _z);

}

void  fm_setTranslation(const REAL *translation,REAL *matrix)
{
  matrix[12] = translation[0];
  matrix[13] = translation[1];
  matrix[14] = translation[2];
}

void  fm_transform(const REAL matrix[16],const REAL v[3],REAL t[3]) // rotate and translate this point
{
  if ( matrix )
  {
    REAL tx = (matrix[0*4+0] * v[0]) +  (matrix[1*4+0] * v[1]) + (matrix[2*4+0] * v[2]) + matrix[3*4+0];
    REAL ty = (matrix[0*4+1] * v[0]) +  (matrix[1*4+1] * v[1]) + (matrix[2*4+1] * v[2]) + matrix[3*4+1];
    REAL tz = (matrix[0*4+2] * v[0]) +  (matrix[1*4+2] * v[1]) + (matrix[2*4+2] * v[2]) + matrix[3*4+2];
    t[0] = tx;
    t[1] = ty;
    t[2] = tz;
  }
  else
  {
    t[0] = v[0];
    t[1] = v[1];
    t[2] = v[2];
  }
}

void fm_quatToMatrix(const REAL *quat,REAL *matrix) // convert quaterinion rotation to matrix, zeros out the translation component.
{

	REAL xx = quat[0]*quat[0];
	REAL yy = quat[1]*quat[1];
	REAL zz = quat[2]*quat[2];
	REAL xy = quat[0]*quat[1];
	REAL xz = quat[0]*quat[2];
	REAL yz = quat[1]*quat[2];
	REAL wx = quat[3]*quat[0];
	REAL wy = quat[3]*quat[1];
	REAL wz = quat[3]*quat[2];

	matrix[0*4+0] = 1 - 2 * ( yy + zz );
	matrix[1*4+0] =     2 * ( xy - wz );
	matrix[2*4+0] =     2 * ( xz + wy );

	matrix[0*4+1] =     2 * ( xy + wz );
	matrix[1*4+1] = 1 - 2 * ( xx + zz );
	matrix[2*4+1] =     2 * ( yz - wx );

	matrix[0*4+2] =     2 * ( xz - wy );
	matrix[1*4+2] =     2 * ( yz + wx );
	matrix[2*4+2] = 1 - 2 * ( xx + yy );

	matrix[3*4+0] = matrix[3*4+1] = matrix[3*4+2] = (REAL) 0.0f;
	matrix[0*4+3] = matrix[1*4+3] = matrix[2*4+3] = (REAL) 0.0f;
	matrix[3*4+3] =(REAL) 1.0f;

}

void fm_cross(REAL *cross,const REAL *a,const REAL *b)
{
	cross[0] = a[1]*b[2] - a[2]*b[1];
	cross[1] = a[2]*b[0] - a[0]*b[2];
	cross[2] = a[0]*b[1] - a[1]*b[0];
}


REAL fm_dot(const REAL *p1,const REAL *p2)
{
  return p1[0]*p2[0]+p1[1]*p2[1]+p1[2]*p2[2];
}

// Reference, from Stan Melax in Game Gems I
//  Quaternion q;
//  vector3 c = CrossProduct(v0,v1);
//  REAL   d = DotProduct(v0,v1);
//  REAL   s = (REAL)sqrt((1+d)*2);
//  q.x = c.x / s;
//  q.y = c.y / s;
//  q.z = c.z / s;
//  q.w = s /2.0f;
//  return q;
int fm_rotationArc(const REAL *v0,const REAL *v1,REAL *quat)
{
  REAL cross[3];

  fm_cross(cross,v0,v1);
  REAL d = fm_dot(v0,v1);
  REAL s = sqrt((1+d)*2);
  if (MathLib::IsZero(s)) {
	  return -1;
  }
  REAL recip = 1.0f / s;

  quat[0] = cross[0] * recip;
  quat[1] = cross[1] * recip;
  quat[2] = cross[2] * recip;
  quat[3] = s * 0.5f;

  return 0;
}

void fm_planeToMatrix(const REAL *plane,REAL *matrix) // convert a plane equation to a 4x4 rotation matrix
{
  REAL ref[3][3] = { {0,1,0}, {0,0,1}, {1,0,0} };
  REAL quat[4];
  for (int i=0; i<3; i++) {
	  if (fm_rotationArc(ref[i],plane,quat) == 0) {
		  break;
	  }
  }
  fm_quatToMatrix(quat,matrix);
  REAL origin[3] = { 0, -plane[3], 0 };
  REAL center[3];
  fm_transform(matrix,origin,center);
  fm_setTranslation(center,matrix);
}

void fm_planeToQuat(const REAL *plane,REAL *quat,REAL *pos) // convert a plane equation to a quaternion and translation
{
  REAL ref[3][3] = { {0,1,0}, {0,0,1}, {1,0,0} };
  REAL matrix[16];
  for (int i=0; i<3; i++) {
	  if (fm_rotationArc(ref[i],plane,quat) == 0) {
		  break;
	  }
  }
  fm_quatToMatrix(quat,matrix);
  REAL origin[3] = { 0, plane[3], 0 };
  fm_transform(matrix,origin,pos);
}

void fm_eulerToQuat(REAL roll,REAL pitch,REAL yaw,REAL *quat) // convert euler angles to quaternion.
{
	roll  *= 0.5f;
	pitch *= 0.5f;
	yaw   *= 0.5f;

	REAL cr = cos(roll);
	REAL cp = cos(pitch);
	REAL cy = cos(yaw);

	REAL sr = sin(roll);
	REAL sp = sin(pitch);
	REAL sy = sin(yaw);

	REAL cpcy = cp * cy;
	REAL spsy = sp * sy;
	REAL spcy = sp * cy;
	REAL cpsy = cp * sy;

	quat[0]   = ( sr * cpcy - cr * spsy);
	quat[1]   = ( cr * spcy + sr * cpsy);
	quat[2]   = ( cr * cpsy - sr * spcy);
	quat[3]   = cr * cpcy + sr * spsy;
}

void  fm_eulerToQuat(const REAL *euler,REAL *quat) // convert euler angles to quaternion.
{
  fm_eulerToQuat(euler[0],euler[1],euler[2],quat);
}


void fm_eulerMatrix(REAL ax,REAL ay,REAL az,REAL *matrix) // convert euler (in radians) to a dest 4x4 matrix (translation set to zero)
{
  REAL quat[4];
  fm_eulerToQuat(ax,ay,az,quat);
  fm_quatToMatrix(quat,matrix);
}

template <class Type> class Eigen
{
public:


  void DecrSortEigenStuff(void)
  {
    Tridiagonal(); //diagonalize the matrix.
    QLAlgorithm(); //
    DecreasingSort();
    GuaranteeRotation();
  }

  void Tridiagonal(void)
  {
    Type fM00 = mElement[0][0];
    Type fM01 = mElement[0][1];
    Type fM02 = mElement[0][2];
    Type fM11 = mElement[1][1];
    Type fM12 = mElement[1][2];
    Type fM22 = mElement[2][2];

    m_afDiag[0] = fM00;
    m_afSubd[2] = 0;
    if (fM02 != (Type)0.0)
    {
      Type fLength = sqrt(fM01*fM01+fM02*fM02);
      Type fInvLength = ((Type)1.0)/fLength;
      fM01 *= fInvLength;
      fM02 *= fInvLength;
      Type fQ = ((Type)2.0)*fM01*fM12+fM02*(fM22-fM11);
      m_afDiag[1] = fM11+fM02*fQ;
      m_afDiag[2] = fM22-fM02*fQ;
      m_afSubd[0] = fLength;
      m_afSubd[1] = fM12-fM01*fQ;
      mElement[0][0] = (Type)1.0;
      mElement[0][1] = (Type)0.0;
      mElement[0][2] = (Type)0.0;
      mElement[1][0] = (Type)0.0;
      mElement[1][1] = fM01;
      mElement[1][2] = fM02;
      mElement[2][0] = (Type)0.0;
      mElement[2][1] = fM02;
      mElement[2][2] = -fM01;
      m_bIsRotation = false;
    }
    else
    {
      m_afDiag[1] = fM11;
      m_afDiag[2] = fM22;
      m_afSubd[0] = fM01;
      m_afSubd[1] = fM12;
      mElement[0][0] = (Type)1.0;
      mElement[0][1] = (Type)0.0;
      mElement[0][2] = (Type)0.0;
      mElement[1][0] = (Type)0.0;
      mElement[1][1] = (Type)1.0;
      mElement[1][2] = (Type)0.0;
      mElement[2][0] = (Type)0.0;
      mElement[2][1] = (Type)0.0;
      mElement[2][2] = (Type)1.0;
      m_bIsRotation = true;
    }
  }

  bool QLAlgorithm(void)
  {
    const int iMaxIter = 32;

    for (int i0 = 0; i0 <3; i0++)
    {
      int i1;
      for (i1 = 0; i1 < iMaxIter; i1++)
      {
        int i2;
        for (i2 = i0; i2 <= (3-2); i2++)
        {
          Type fTmp = fabs(m_afDiag[i2]) + fabs(m_afDiag[i2+1]);
          if ( fabs(m_afSubd[i2]) + fTmp == fTmp )
            break;
        }
        if (i2 == i0)
        {
          break;
        }

        Type fG = (m_afDiag[i0+1] - m_afDiag[i0])/(((Type)2.0) * m_afSubd[i0]);
        Type fR = sqrt(fG*fG+(Type)1.0);
        if (fG < (Type)0.0)
        {
          fG = m_afDiag[i2]-m_afDiag[i0]+m_afSubd[i0]/(fG-fR);
        }
        else
        {
          fG = m_afDiag[i2]-m_afDiag[i0]+m_afSubd[i0]/(fG+fR);
        }
        Type fSin = (Type)1.0, fCos = (Type)1.0, fP = (Type)0.0;
        for (int i3 = i2-1; i3 >= i0; i3--)
        {
          Type fF = fSin*m_afSubd[i3];
          Type fB = fCos*m_afSubd[i3];
          if (fabs(fF) >= fabs(fG))
          {
            fCos = fG/fF;
            fR = sqrt(fCos*fCos+(Type)1.0);
            m_afSubd[i3+1] = fF*fR;
            fSin = ((Type)1.0)/fR;
            fCos *= fSin;
          }
          else
          {
            fSin = fF/fG;
            fR = sqrt(fSin*fSin+(Type)1.0);
            m_afSubd[i3+1] = fG*fR;
            fCos = ((Type)1.0)/fR;
            fSin *= fCos;
          }
          fG = m_afDiag[i3+1]-fP;
          fR = (m_afDiag[i3]-fG)*fSin+((Type)2.0)*fB*fCos;
          fP = fSin*fR;
          m_afDiag[i3+1] = fG+fP;
          fG = fCos*fR-fB;
          for (int i4 = 0; i4 < 3; i4++)
          {
            fF = mElement[i4][i3+1];
            mElement[i4][i3+1] = fSin*mElement[i4][i3]+fCos*fF;
            mElement[i4][i3] = fCos*mElement[i4][i3]-fSin*fF;
          }
        }
        m_afDiag[i0] -= fP;
        m_afSubd[i0] = fG;
        m_afSubd[i2] = (Type)0.0;
      }
      if (i1 == iMaxIter)
      {
        return false;
      }
    }
    return true;
  }

  void DecreasingSort(void)
  {
    //sort eigenvalues in decreasing order, e[0] >= ... >= e[iSize-1]
    for (int i0 = 0, i1; i0 <= 3-2; i0++)
    {
      // locate maximum eigenvalue
      i1 = i0;
      Type fMax = m_afDiag[i1];
      int i2;
      for (i2 = i0+1; i2 < 3; i2++)
      {
        if (m_afDiag[i2] > fMax)
        {
          i1 = i2;
          fMax = m_afDiag[i1];
        }
      }

      if (i1 != i0)
      {
        // swap eigenvalues
        m_afDiag[i1] = m_afDiag[i0];
        m_afDiag[i0] = fMax;
        // swap eigenvectors
        for (i2 = 0; i2 < 3; i2++)
        {
          Type fTmp = mElement[i2][i0];
          mElement[i2][i0] = mElement[i2][i1];
          mElement[i2][i1] = fTmp;
          m_bIsRotation = !m_bIsRotation;
        }
      }
    }
  }


  void GuaranteeRotation(void)
  {
    if (!m_bIsRotation)
    {
      // change sign on the first column
      for (int iRow = 0; iRow <3; iRow++)
      {
        mElement[iRow][0] = -mElement[iRow][0];
      }
    }
  }

  Type mElement[3][3];
  Type m_afDiag[3];
  Type m_afSubd[3];
  bool m_bIsRotation;
};

bool computeBestFitPlane(size_t vcount,
                     const REAL *points,
                     size_t vstride,
                     const REAL *weights,
                     size_t wstride,
                     REAL *plane)
{
  bool ret = false;

  REAL kOrigin[3] = { 0, 0, 0 };

  REAL wtotal = 0;

  {
    const char *source  = (const char *) points;
    const char *wsource = (const char *) weights;

    for (size_t i=0; i<vcount; i++)
    {

      const REAL *p = (const REAL *) source;

      REAL w = 1;

      if ( wsource )
      {
        const REAL *ws = (const REAL *) wsource;
        w = *ws; //
        wsource+=wstride;
      }

      kOrigin[0]+=p[0]*w;
      kOrigin[1]+=p[1]*w;
      kOrigin[2]+=p[2]*w;

      wtotal+=w;

      source+=vstride;
    }
  }

  REAL recip = 1.0f / wtotal; // reciprocol of total weighting

  kOrigin[0]*=recip;
  kOrigin[1]*=recip;
  kOrigin[2]*=recip;


  REAL fSumXX=0;
  REAL fSumXY=0;
  REAL fSumXZ=0;

  REAL fSumYY=0;
  REAL fSumYZ=0;
  REAL fSumZZ=0;


  {
    const char *source  = (const char *) points;
    const char *wsource = (const char *) weights;

    for (size_t i=0; i<vcount; i++)
    {

      const REAL *p = (const REAL *) source;

      REAL w = 1;

      if ( wsource )
      {
        const REAL *ws = (const REAL *) wsource;
        w = *ws; //
        wsource+=wstride;
      }

      REAL kDiff[3];

      kDiff[0] = w*(p[0] - kOrigin[0]); // apply vertex weighting!
      kDiff[1] = w*(p[1] - kOrigin[1]);
      kDiff[2] = w*(p[2] - kOrigin[2]);

      fSumXX+= kDiff[0] * kDiff[0]; // sume of the squares of the differences.
      fSumXY+= kDiff[0] * kDiff[1]; // sume of the squares of the differences.
      fSumXZ+= kDiff[0] * kDiff[2]; // sume of the squares of the differences.

      fSumYY+= kDiff[1] * kDiff[1];
      fSumYZ+= kDiff[1] * kDiff[2];
      fSumZZ+= kDiff[2] * kDiff[2];


      source+=vstride;
    }
  }

  fSumXX *= recip;
  fSumXY *= recip;
  fSumXZ *= recip;
  fSumYY *= recip;
  fSumYZ *= recip;
  fSumZZ *= recip;

  // setup the eigensolver
  Eigen<REAL> kES;

  kES.mElement[0][0] = fSumXX;
  kES.mElement[0][1] = fSumXY;
  kES.mElement[0][2] = fSumXZ;

  kES.mElement[1][0] = fSumXY;
  kES.mElement[1][1] = fSumYY;
  kES.mElement[1][2] = fSumYZ;

  kES.mElement[2][0] = fSumXZ;
  kES.mElement[2][1] = fSumYZ;
  kES.mElement[2][2] = fSumZZ;

  // compute eigenstuff, smallest eigenvalue is in last position
  kES.DecrSortEigenStuff();

  REAL kNormal[3];

  kNormal[0] = kES.mElement[0][2];
  kNormal[1] = kES.mElement[1][2];
  kNormal[2] = kES.mElement[2][2];

  // the minimum energy
  plane[0] = kNormal[0];
  plane[1] = kNormal[1];
  plane[2] = kNormal[2];

  plane[3] = 0 - fm_dot(kNormal,kOrigin);

  ret = true;

  return ret;
}

// computes the OBB for this set of points relative to this transform matrix.
void computeOBB(size_t vcount,const REAL *points,size_t pstride,REAL *sides,REAL *matrix)
{
  const char *src = (const char *) points;

  REAL bmin[3] = { 1e9, 1e9, 1e9 };
  REAL bmax[3] = { -1e9, -1e9, -1e9 };

  for (size_t i=0; i<vcount; i++)
  {
    const REAL *p = (const REAL *) src;
    REAL t[3];

    fm_inverseRT(matrix, p, t ); // inverse rotate translate

    if ( t[0] < bmin[0] ) bmin[0] = t[0];
    if ( t[1] < bmin[1] ) bmin[1] = t[1];
    if ( t[2] < bmin[2] ) bmin[2] = t[2];

    if ( t[0] > bmax[0] ) bmax[0] = t[0];
    if ( t[1] > bmax[1] ) bmax[1] = t[1];
    if ( t[2] > bmax[2] ) bmax[2] = t[2];

    src+=pstride;
  }

  REAL center[3];

  sides[0] = bmax[0]-bmin[0];
  sides[1] = bmax[1]-bmin[1];
  sides[2] = bmax[2]-bmin[2];

  center[0] = sides[0]*0.5f+bmin[0];
  center[1] = sides[1]*0.5f+bmin[1];
  center[2] = sides[2]*0.5f+bmin[2];

  REAL ocenter[3];

  fm_rotate(matrix,center,ocenter);

  matrix[12]+=ocenter[0];
  matrix[13]+=ocenter[1];
  matrix[14]+=ocenter[2];

}

void computeBestFitOBB_FixZ(size_t vcount,const REAL *points,size_t pstride,REAL *sides,REAL *matrix,FitStrategy strategy)
{
	fm_identity(matrix);
	REAL bmin[3];
	REAL bmax[3];
	computeBestFitAABB(vcount,points,pstride,bmin,bmax);

	matrix[12] = (bmin[0]+bmax[0])*0.5f;
	matrix[13] = (bmin[1]+bmax[1])*0.5f;
	matrix[14] = (bmin[2]+bmax[2])*0.5f;
	sides[0] = bmax[0]-bmin[0];
	sides[1] = bmax[1]-bmin[1];
	sides[2] = bmax[2]-bmin[2];
	REAL volume = sides[0]*sides[1]*sides[2];

	REAL refmatrix[16];
	memcpy(refmatrix,matrix,16*sizeof(REAL));

	float stepSize=5;
	switch ( strategy )
	{
	case FS_FAST_FIT:
		stepSize = 13; // 15 degree increments
		break;
	case FS_MEDIUM_FIT:
		stepSize = 7; // 10 degree increments
		break;
	case FS_SLOW_FIT:
		stepSize = 3; // 5 degree increments
		break;
	}

	for (REAL a=0; a<180; a+=stepSize)
	{
		REAL quat[4];
		fm_eulerToQuat(0,0,a*FM_DEG_TO_RAD,quat);
		REAL temp[16];
		REAL pmatrix[16];
		fm_quatToMatrix(quat,temp);
		fm_matrixMultiply(temp,refmatrix,pmatrix);
		REAL psides[3];
		computeOBB( vcount, points, pstride, psides, pmatrix );
		REAL v = psides[0]*psides[1]*psides[2];
		if ( v < volume )
		{
			volume = v;
			memcpy(matrix,pmatrix,sizeof(REAL)*16);
			sides[0] = psides[0];
			sides[1] = psides[1];
			sides[2] = psides[2];
		}
	}
}

void computeBestFitOBB(size_t vcount,const REAL *points,size_t pstride,REAL *sides,REAL *matrix,FitStrategy strategy)
{
  fm_identity(matrix);
  REAL bmin[3];
  REAL bmax[3];
  computeBestFitAABB(vcount,points,pstride,bmin,bmax);

  REAL avolume = (bmax[0]-bmin[0])*(bmax[1]-bmin[1])*(bmax[2]-bmin[2]);

  REAL plane[4];
  computeBestFitPlane(vcount,points,pstride,0,0,plane);
  fm_planeToMatrix(plane,matrix);
  computeOBB( vcount, points, pstride, sides, matrix );

  REAL refmatrix[16];
  memcpy(refmatrix,matrix,16*sizeof(REAL));

  REAL volume = sides[0]*sides[1]*sides[2];

  float stepSize=5;
  switch ( strategy )
  {
    case FS_FAST_FIT:
      stepSize = 13; // 15 degree increments
      break;
    case FS_MEDIUM_FIT:
      stepSize = 7; // 10 degree increments
      break;
    case FS_SLOW_FIT:
      stepSize = 3; // 5 degree increments
      break;
  }

  for (REAL a=0; a<180; a+=stepSize)
  {
    REAL quat[4];
    fm_eulerToQuat(0,a*FM_DEG_TO_RAD,0,quat);
    REAL temp[16];
    REAL pmatrix[16];
    fm_quatToMatrix(quat,temp);
    fm_matrixMultiply(temp,refmatrix,pmatrix);
    REAL psides[3];
    computeOBB( vcount, points, pstride, psides, pmatrix );
    REAL v = psides[0]*psides[1]*psides[2];
    if ( v < volume )
    {
      volume = v;
      memcpy(matrix,pmatrix,sizeof(REAL)*16);
      sides[0] = psides[0];
      sides[1] = psides[1];
      sides[2] = psides[2];
    }
  }

  if ( avolume < volume )
  {
    fm_identity(matrix);
    matrix[12] = (bmin[0]+bmax[0])*0.5f;
    matrix[13] = (bmin[1]+bmax[1])*0.5f;
    matrix[14] = (bmin[2]+bmax[2])*0.5f;
    sides[0] = bmax[0]-bmin[0];
    sides[1] = bmax[1]-bmin[1];
    sides[2] = bmax[2]-bmin[2];
  }
}

void computeBestFitOBB(size_t vcount,const REAL *points,size_t pstride,REAL *sides,REAL *pos,REAL *quat,FitStrategy strategy)
{
  REAL matrix[16];
  computeBestFitOBB(vcount,points,pstride,sides,matrix,strategy);
  fm_getTranslation(matrix,pos);
  fm_matrixToQuat(matrix,quat);
}

void computeBestFitABB(size_t vcount,const REAL *points,size_t pstride,REAL *sides,REAL *pos)
{
	REAL bmin[3];
	REAL bmax[3];

  bmin[0] = points[0];
  bmin[1] = points[1];
  bmin[2] = points[2];

  bmax[0] = points[0];
  bmax[1] = points[1];
  bmax[2] = points[2];

	const char *cp = (const char *) points;
	for (size_t i=0; i<vcount; i++)
	{
		const REAL *p = (const REAL *) cp;

		if ( p[0] < bmin[0] ) bmin[0] = p[0];
		if ( p[1] < bmin[1] ) bmin[1] = p[1];
		if ( p[2] < bmin[2] ) bmin[2] = p[2];

    if ( p[0] > bmax[0] ) bmax[0] = p[0];
    if ( p[1] > bmax[1] ) bmax[1] = p[1];
    if ( p[2] > bmax[2] ) bmax[2] = p[2];

    cp+=pstride;
	}


	sides[0] = bmax[0] - bmin[0];
	sides[1] = bmax[1] - bmin[1];
	sides[2] = bmax[2] - bmin[2];

	pos[0] = bmin[0]+sides[0]*0.5f;
	pos[1] = bmin[1]+sides[1]*0.5f;
	pos[2] = bmin[2]+sides[2]*0.5f;

}


void computeBestFitCapsule(size_t vcount,const REAL *points,size_t pstride,REAL &radius,REAL &height,REAL matrix[16],FitStrategy strategy)
{
  REAL sides[3];
  REAL omatrix[16];
  computeBestFitOBB(vcount,points,pstride,sides,omatrix,strategy);

  int axis = 0;
  if ( sides[0] > sides[1] && sides[0] > sides[2] )
    axis = 0;
  else if ( sides[1] > sides[0] && sides[1] > sides[2] )
    axis = 1;
  else
    axis = 2;

  REAL localTransform[16];

  REAL maxDist = 0;
  REAL maxLen = 0;

  switch ( axis )
  {
    case 0:
      {
        fm_eulerMatrix(0,0,FM_PI/2,localTransform);
        fm_matrixMultiply(localTransform,omatrix,matrix);

        const unsigned char *scan = (const unsigned char *)points;
        for (size_t i=0; i<vcount; i++)
        {
          const REAL *p = (const REAL *)scan;
          REAL t[3];
          fm_inverseRT(omatrix,p,t);
          REAL dist = t[1]*t[1]+t[2]*t[2];
          if ( dist > maxDist )
          {
            maxDist = dist;
          }
          REAL l = (REAL) fabs(t[0]);
          if ( l > maxLen )
          {
            maxLen = l;
          }
          scan+=pstride;
        }
      }
      height = sides[0];
      break;
    case 1:
      {
        fm_eulerMatrix(0,FM_PI/2,0,localTransform);
        fm_matrixMultiply(localTransform,omatrix,matrix);

        const unsigned char *scan = (const unsigned char *)points;
        for (size_t i=0; i<vcount; i++)
        {
          const REAL *p = (const REAL *)scan;
          REAL t[3];
          fm_inverseRT(omatrix,p,t);
          REAL dist = t[0]*t[0]+t[2]*t[2];
          if ( dist > maxDist )
          {
            maxDist = dist;
          }
          REAL l = (REAL) fabs(t[1]);
          if ( l > maxLen )
          {
            maxLen = l;
          }
          scan+=pstride;
        }
      }
      height = sides[1];
      break;
    case 2:
      {
        fm_eulerMatrix(FM_PI/2,0,0,localTransform);
        fm_matrixMultiply(localTransform,omatrix,matrix);

        const unsigned char *scan = (const unsigned char *)points;
        for (size_t i=0; i<vcount; i++)
        {
          const REAL *p = (const REAL *)scan;
          REAL t[3];
          fm_inverseRT(omatrix,p,t);
          REAL dist = t[0]*t[0]+t[1]*t[1];
          if ( dist > maxDist )
          {
            maxDist = dist;
          }
          REAL l = (REAL) fabs(t[2]);
          if ( l > maxLen )
          {
            maxLen = l;
          }
          scan+=pstride;
        }
      }
      height = sides[2];
      break;
  }
  radius = (REAL)sqrt(maxDist);
  height = (maxLen*2)-(radius*2);
}

REAL computeBestFitAABB(size_t vcount,const REAL *points,size_t pstride,REAL *bmin,REAL *bmax) // returns the diagonal distance
{

  const unsigned char *source = (const unsigned char *) points;

	bmin[0] = points[0];
	bmin[1] = points[1];
	bmin[2] = points[2];

	bmax[0] = points[0];
	bmax[1] = points[1];
	bmax[2] = points[2];


  for (size_t i=1; i<vcount; i++)
  {
  	source+=pstride;
  	const REAL *p = (const REAL *) source;

  	if ( p[0] < bmin[0] ) bmin[0] = p[0];
  	if ( p[1] < bmin[1] ) bmin[1] = p[1];
  	if ( p[2] < bmin[2] ) bmin[2] = p[2];

		if ( p[0] > bmax[0] ) bmax[0] = p[0];
		if ( p[1] > bmax[1] ) bmax[1] = p[1];
		if ( p[2] > bmax[2] ) bmax[2] = p[2];

  }

  REAL dx = bmax[0] - bmin[0];
  REAL dy = bmax[1] - bmin[1];
  REAL dz = bmax[2] - bmin[2];

	return (REAL) sqrt( dx*dx + dy*dy + dz*dz );

}

inline float mb_sqr(float r) {return r*r;}
inline double mb_sqr(double r) {return r*r;}

// Vec3 (inline)
// -------------
template <class Type> class Vec3
{
public:
  // default
  Vec3(void)  {}

  // copy from Vec3
  Vec3 (const Vec3& p)
  {
    coord[0] = p.coord[0];
    coord[1] = p.coord[1];
    coord[2] = p.coord[2];
  }

  // copy from Type*
  Vec3 (const Type* p)
  {
    coord[0] = p[0];
    coord[1] = p[1];
    coord[2] = p[2];
  }

  void set(const Type* p)
  {
    coord[0] = p[0];
    coord[1] = p[1];
    coord[2] = p[2];
  }

  // assignment
  Vec3& operator = (const Vec3& p)
  {
    if (this != &p)
    {
    	coord[0] = p.coord[0];
    	coord[1] = p.coord[1];
    	coord[2] = p.coord[2];
    }
    return *this;
  }

  // coordinate access
  Type& operator [] (int i)
  {
    return coord[i];
  }

  const Type& operator [] (int i) const
  {
    return coord[i];
  }

  const Type* begin(void) const
  {
    return coord;
  }

  const Type* end(void) const
  {
    return coord+3;
  }
private:
  Type coord [3];

};




// SphereFit
// ----------

template <class Type> class SphereFit
{
public:
  SphereFit() {reset();}

  // access
  const Type*       center() const { return mCurrentCenter;  }
  Type              squared_radius() const  { return mCurrentSquaredRadius;  }
  int                size() const  {    return mSize;  }
  int                support_size() const  {  return mSupport;  }
  Type              excess (const Vec3<Type>& p) const
  {
    Type e = -mCurrentSquaredRadius;
    for (int k=0; k<3; ++k)
      e += mb_sqr(p[k]-mCurrentCenter[k]);
    return e;
  }

  // modification
  void                reset(void) // generates empty sphere with m=s=0
  {
    mSize = mSupport = 0;
    // we misuse mC[0] for the center of the empty sphere
    for (int j=0; j<3; ++j)
      mC[0][j]=0;
    mCurrentCenter = mC[0];
    mCurrentSquaredRadius = -1;
  }

  bool  push(const Vec3<Type>& p)
  {
    int i, j;
    Type eps = 1e-15f;
    if (mSize==0)
    {
      for (i=0; i<3; ++i)
        mQ[i] = p[i];

      for (i=0; i<3; ++i)
        mC[0][i] = mQ[i];

      mSquareRadius[0] = 0;

    }
    else
    {
      // set v_m to Q_m
      for (i=0; i<3; ++i)
        mV[mSize][i] = p[i]-mQ[i];

      // compute the a_{m,i}, i< m
      for (i=1; i<mSize; ++i)
      {
        mA[mSize][i] = 0;
        for (j=0; j<3; ++j)
     	    mA[mSize][i] += mV[i][j] * mV[mSize][j];
        mA[mSize][i]*=(2/mZ[i]);
      }

      // update v_m to Q_m-\bar{Q}_m
      for (i=1; i<mSize; ++i)
      {
        for (j=0; j<3; ++j)
        {
  	      mV[mSize][j] -= mA[mSize][i]*mV[i][j];
        }
      }

      // compute z_m
      mZ[mSize]=0;

      for (j=0; j<3; ++j)
      {
        mZ[mSize] += mb_sqr(mV[mSize][j]);
      }

      mZ[mSize]*=2;

      // reject push if z_m too small
      if (mZ[mSize]<eps*mCurrentSquaredRadius)
      {
        return false;
      }

      // update c, mSquareRadius
      Type e = -mSquareRadius[mSize-1];

      for (i=0; i<3; ++i)
      {
        e += mb_sqr(p[i]-mC[mSize-1][i]);
      }
      mF[mSize]=e/mZ[mSize];

      for (i=0; i<3; ++i)
      {
        mC[mSize][i] = mC[mSize-1][i]+mF[mSize]*mV[mSize][i];
      }
      mSquareRadius[mSize] = mSquareRadius[mSize-1] + e*mF[mSize]/2;
    }
    mCurrentCenter = mC[mSize];
    mCurrentSquaredRadius = mSquareRadius[mSize];
    mSupport = ++mSize;
    return true;
  }

  void                pop (void) { --mSize; };

  // checking
  Type              slack(void) const
  {
    Type l[4], min_l=0;

    l[0] = 1;

    for (int i=mSupport-1; i>0; --i)
    {
      l[i] = mF[i];
      for (int k=mSupport-1; k>i; --k)
      {
        l[i]-=mA[k][i]*l[k];
      }
      if (l[i] < min_l) min_l = l[i];
      {
        l[0] -= l[i];
      }
    }

    if (l[0] < min_l)
      min_l = l[0];

    return ( (min_l < 0) ? -min_l : 0);

  }

private:
  // data members
  int                mSize;
  int                mSupport;

  Type              mQ[3];
  Type              mZ[4];
  Type              mF[4];
  Type              mV[4][3];
  Type              mA[4][3];

  Type              mC[4][3];
  Type              mSquareRadius[4];
  Type*             mCurrentCenter;      // refers to some mC[j]
  Type              mCurrentSquaredRadius;
};

// --------
template <class Type> class BestFitSphere
{
public:
  // builds the smallest enclosing sphere of the internal point set
  Type computeBestFitSphere(unsigned int pcount,const Type *points,unsigned int pstride,Type *center)
  {
    mPcount = pcount;
    mPoints = new Vec3<Type>[pcount];
    const unsigned char *scan = (const unsigned char *)points;

    for (unsigned int i=0; i<pcount; i++)
    {
      Type *p = (Type *)scan;
      mPoints[i].set(p);
      scan+=pstride;
    }
    mBestFitSphere.reset();
    mSupportEnd = 0;
    pivot_mb(mPcount);
    const Type *c = mBestFitSphere.center();
    center[0] = c[0];
    center[1] = c[1];
    center[2] = c[2];
    delete []mPoints;
    mPoints = 0;
    return sqrtf(mBestFitSphere.squared_radius());
  }


private:

  int         nr_support_points (void) const
  {
    return mBestFitSphere.support_size();
  }

  Type      accuracy (Type& slack) const
  {
    Type e, max_e = 0;
    int n_supp=0;
    unsigned int i;
    for (i=0; i!=mSupportEnd; ++i,++n_supp)
      if ((e = std::abs (mBestFitSphere.excess (mPoints[i]))) > max_e)
        max_e = e;

    // you've found a non-numerical problem if the following ever fails
    assert (n_supp == nr_support_points());

    for (i=mSupportEnd; i!=mPcount; ++i)
    {
      if ((e = mBestFitSphere.excess(mPoints[i])) > max_e)
      {
        max_e = e;
      }
    }

    slack = mBestFitSphere.slack();

    return (max_e/mBestFitSphere.squared_radius());
  }

  // returns true if the accuracy is below the given tolerance and the
  // slack is 0
  bool        is_valid(Type tolerance = 1e-15f) const
  {
    Type slack;
    return ( (accuracy (slack) < tolerance) && (slack == 0) );
  }


  void  mtf_mb(unsigned int i)
  {
    mSupportEnd = 0;
    if ((mBestFitSphere.size())==4) return;
    for (unsigned int k=0; k!=i;)
    {
      unsigned int j= k++;
      if (mBestFitSphere.excess(mPoints[j]) > 0)
      {
        if (mBestFitSphere.push(mPoints[j]))
        {
      	  mtf_mb(j);
       	  mBestFitSphere.pop();
  	      move_to_front(j);
        }
      }
    }
  }

  void pivot_mb(unsigned int i)
  {
    unsigned int t = 1;
    mtf_mb(t);
    Type max_e, old_mSquareRadius = -1;
    do
    {
      unsigned int pivot;
      max_e = max_excess (t,i,pivot);
      if (max_e > 0)
      {
        t = mSupportEnd;
        if (t==pivot) ++t;
        old_mSquareRadius = mBestFitSphere.squared_radius();
        mBestFitSphere.push (mPoints[pivot]);
        mtf_mb(mSupportEnd);
        mBestFitSphere.pop();
        move_to_front(pivot);
      }
    } while ((max_e > 0) && (mBestFitSphere.squared_radius() > old_mSquareRadius));
  }

  void move_to_front (unsigned int j)
  {
    if (mSupportEnd == j)
      mSupportEnd++;
    if ( j > 0 )
    {
      Vec3<Type> p = mPoints[j];
      for (int i=j; i>0; i--)
      {
        mPoints[i] = mPoints[i-1];
      }
      mPoints[0] = p;

      if ( mSupportEnd < j )
      {
        mSupportEnd++;
      }
    }
  }

  Type  max_excess(unsigned int t,unsigned int i,unsigned int& pivot) const
  {
    const Type *c = mBestFitSphere.center(), mSquareRadius = mBestFitSphere.squared_radius();
    Type e, max_e = 0;
    for (unsigned int k=t; k!=i; ++k)
    {
      const Type *p = (mPoints[k]).begin();
      e = -mSquareRadius;
      for (int j=0; j<3; ++j)
        e += mb_sqr(p[j]-c[j]);
      if (e > max_e)
      {
        max_e = e;
        pivot = k;
      }
    }
    return max_e;
  }

  // data members
  unsigned int         mPcount;
  Vec3<Type>          *mPoints;
  SphereFit<Type>      mBestFitSphere;            // the current best fit sphere
  unsigned int         mSupportEnd;  // past-the-end iterator of support set

};


REAL  computeBestFitSphere(size_t vcount,const REAL *points,size_t pstride,REAL *center)
{
  BestFitSphere<REAL> mb;
  return mb.computeBestFitSphere(vcount,points,pstride,center);
}


}; // end of namespace
