#pragma once 

#include "Utility.h"

//static const float ZERO_EPSILON = 1e-6f;

static bool ray_tri_test(float3 org, float3 dir, float3 p0, float3 p1, float3 p2, float tmin, float tmax, float &t, float &beta, float &gamma) {
    // Input:
    // org: origin of the ray
    // dir: normalized direction of the ray
    // p0, p1, p2: triangle coordinates
    // tmin, tmax: min and max range of the ray, e.g., [1e-3f, 1e6], depending on which application
    // beta, gamma: if hit, output the barycentric coordinates of the hit point on the triangle

   /*
    * Solve 3 linear equation p0 + t * d = alpha * AB + beta * AC
    * to obtain t, alpha, beta.
    *
    * Special case:
    * When the ray is parallel to the triangle,
    * d is a linear combination of AB and AC, which causes the matrix [AB, AC, d]
    * to be ill-conditioned and cannot be inverted. The determinant of this matrix is zero.
    * Solution becomes none if the ray is not coplanar with the triangle, or a line segment
    * if the ray is coplanar.
    */
    float3 p01 = p0 - p1;
    float A = p01.x;
    float B = p01.y;
    float C = p01.z;

    float3 p02 = p0 - p2;
    float D = p02.x;
    float E = p02.y;
    float F = p02.z;

    float G = dir.x;
    float H = dir.y;
    float I = dir.z;

    float3 p0org = p0 - org;
    float J = p0org.x;
    float K = p0org.y;
    float L = p0org.z;

    float EIHF = E * I - H * F;
    float GFDI = G * F - D * I;
    float DHEG = D * H - E * G;

    float denom = (A * EIHF + B * GFDI + C * DHEG);
    if (-ZERO_EPSILON < denom && denom < ZERO_EPSILON) { // ray lies on the same surface with the triangle, matrix degenerated
        return false;
    }

    beta = (J * EIHF + K * GFDI + L * DHEG) / denom;
    if (beta < 0.0f || beta > 1.0f) return false;

    float AKJB = A * K - J * B;
    float JCAL = J * C - A * L;
    float BLKC = B * L - K * C;

    gamma = (I * AKJB + H * JCAL + G * BLKC) / denom;
    if (gamma < 0.0f || beta + gamma > 1.0f) return false;

    t = -(F * AKJB + E * JCAL + D * BLKC) / denom;    
    if (t >= tmin && t <= tmax) {
        return true;
    }
    return false;
}
