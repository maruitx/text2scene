/* Triangle/triangle intersection test routine,
 * by Tomas Moller, 1997.
 * See article "A Fast Triangle-Triangle Intersection Test",
 * Journal of Graphics Tools, 2(2), 1997
 * updated: 2001-06-20 (added line of intersection)
 *
 * int tri_tri_intersect(float V0[3],float V1[3],float V2[3],
 *                       float U0[3],float U1[3],float U2[3])
 *
 * parameters: vertices of triangle 1: V0,V1,V2
 *             vertices of triangle 2: U0,U1,U2
 * result    : returns 1 if the triangles intersect, otherwise 0
 *
 * Here is a version withouts divisions (a little faster)
 * int NoDivTriTriIsect(float V0[3],float V1[3],float V2[3],
 *                      float U0[3],float U1[3],float U2[3]);
 * 
 * This version computes the line of intersection as well (if they are not coplanar):
 * int tri_tri_intersect_with_isectline(float V0[3],float V1[3],float V2[3], 
 *				        float U0[3],float U1[3],float U2[3],int *coplanar,
 *				        float isectpt1[3],float isectpt2[3]);
 * coplanar returns whether the tris are coplanar
 * isectpt1, isectpt2 are the endpoints of the line of intersection
 */

#include "../utilities/mathlib.h"


bool IntersectTriTri(const MathLib::Vector3 &v0, const MathLib::Vector3 &v1, const MathLib::Vector3 &v2, const MathLib::Vector3 &u0, const MathLib::Vector3 &u1, const MathLib::Vector3 &u2);
bool IntersectTriTri2D(const MathLib::Vector2 t0[3], const MathLib::Vector2 t1[3]);
bool ContactTriTri(const MathLib::Vector3 &v0, const MathLib::Vector3 &v1, const MathLib::Vector3 &v2, const MathLib::Vector3 &n,
				   const MathLib::Vector3 &u0, const MathLib::Vector3 &u1, const MathLib::Vector3 &u2, const MathLib::Vector3 &m,
				   float ae, float de, bool opp);
bool ContactTriTri(const MathLib::Vector3 &v0, const MathLib::Vector3 &v1, const MathLib::Vector3 &v2, const MathLib::Vector3 &n,
				   const MathLib::Vector3 &u0, const MathLib::Vector3 &u1, const MathLib::Vector3 &u2, const MathLib::Vector3 &m,
				   float de);

//int tri_tri_intersect(float V0[3],float V1[3],float V2[3], float U0[3],float U1[3],float U2[3]);
//int NoDivTriTriIsect(float V0[3],float V1[3],float V2[3], float U0[3],float U1[3],float U2[3]);
//int tri_tri_intersect_with_isectline(float V0[3],float V1[3],float V2[3], float U0[3],float U1[3],float U2[3],int *coplanar, float isectpt1[3],float isectpt2[3]);