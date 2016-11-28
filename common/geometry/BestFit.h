#ifndef BEST_FIT_H

#define BEST_FIT_H

// A code snippet to compute the best fit AAB, OBB, plane, capsule and sphere
// Quaternions are assumed a float X,Y,Z,W
// Matrices are assumed 4x4 D3DX style format passed as a float pointer
// The orientation of a capsule is assumed that height is along the Y axis, the same format as the PhysX SDK uses
// The best fit plane routine is derived from code previously published by David Eberly on his Magic Software site.
// The best fit OBB is computed by first approximating the best fit plane, and then brute force rotating the points
// around a single axis to derive the closest fit.  If you set 'bruteforce' to false, it will just use the orientation
// derived from the best fit plane, which is close enough in most cases, but not all.
// Each routine allows you to pass the point stride between position elements in your input vertex stream.
// These routines should all be thread safe as they make no use of any global variables.


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

typedef double REAL;

namespace BEST_FIT
{

bool  computeBestFitPlane(size_t vcount,     // number of input data points
                     const REAL *points,     // starting address of points array.
                     size_t vstride,    // stride between input points.
                     const REAL *weights,    // *optional point weighting values.
                     size_t wstride,    // weight stride for each vertex.
                     REAL plane[4]);

enum FitStrategy
{
  FS_FAST_FIT, // just computes the diagonals only, can be off substantially at times.
  FS_MEDIUM_FIT, // rotates on one axis to converge to a solution.
  FS_SLOW_FIT,   // rotates on all three axes to find the best fit.
};

REAL  computeBestFitAABB(size_t vcount,const REAL *points,size_t pstride,REAL bmin[3],REAL bmax[3]); // returns the diagonal distance
REAL  computeBestFitSphere(size_t vcount,const REAL *points,size_t pstride,REAL center[3]);
void   computeBestFitOBB(size_t vcount,const REAL *points,size_t pstride,REAL *sides,REAL matrix[16],FitStrategy strategy=FS_MEDIUM_FIT);
void   computeBestFitOBB(size_t vcount,const REAL *points,size_t pstride,REAL *sides,REAL pos[3],REAL quat[4],FitStrategy strategy=FS_MEDIUM_FIT);
void   computeBestFitOBB_FixZ(size_t vcount,const REAL *points,size_t pstride,REAL *sides,REAL matrix[16],FitStrategy strategy=FS_MEDIUM_FIT);
void   computeBestFitCapsule(size_t vcount,const REAL *points,size_t pstride,REAL &radius,REAL &height,REAL matrix[16],FitStrategy strategy=FS_MEDIUM_FIT);

};

#endif
