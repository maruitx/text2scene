#pragma once

#include "Geometry.h"
#include "box_bvh.h"
#include "Utility.h"

/** 
 * CPU implementation of linear BVH.
 */

typedef uint3 TriangleIdx;

struct TriangleHitRecord {
	float t;
	bool hit;
	TriangleIdx triangle;
	float beta, gamma;
	int mesh_triangle_index;

	TriangleHitRecord() : hit(false), t(RAY_TMAX), beta(0.0f), gamma(0.0f), mesh_triangle_index(-1) {
	}
};

struct TriangleMesh {
	TriangleMesh() {}
	TriangleMesh(const vector<float3>&      vertex_vec,
		const vector<TriangleIdx>&    triangle_vec)

		: vertex_vec(vertex_vec)
		, triangle_vec(triangle_vec)
	{

	}

	BoundingBox get_mesh_bounding_box() {
		BoundingBox box;
		for (int i = 0; i < triangle_vec.size(); ++i) {
			BoundingBox b = get_triangle_bounding_box(triangle_vec[i]);
			box.merge(b);
		}
		return box;
	}

	BoundingBox get_triangle_bounding_box(const TriangleIdx &tri) {
		float3 p0 = make_float3(vertex_vec[tri.x]);
		float3 p1 = make_float3(vertex_vec[tri.y]);
		float3 p2 = make_float3(vertex_vec[tri.z]);
		return BoundingBox(p0, p1, p2);
	}

	bool hit(const TriangleIdx &tri, const Ray &r, float tmin, float tmax, TriangleHitRecord &record) const {
		record.hit = false;

		float3 p0 = make_float3(vertex_vec[tri.x]);
		float3 p1 = make_float3(vertex_vec[tri.y]);
		float3 p2 = make_float3(vertex_vec[tri.z]);

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

		float G = r.dir().x;
		float H = r.dir().y;
		float I = r.dir().z;

		float3 p0org = p0 - r.org();
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

		float beta = (J * EIHF + K * GFDI + L * DHEG) / denom;
		if (beta < 0.0f || beta > 1.0f) return false;

		float AKJB = A * K - J * B;
		float JCAL = J * C - A * L;
		float BLKC = B * L - K * C;

		float gamma = (I * AKJB + H * JCAL + G * BLKC) / denom;
		if (gamma < 0.0f || beta + gamma > 1.0f) return false;

		float t = -(F * AKJB + E * JCAL + D * BLKC) / denom;
		if (t >= tmin && t <= tmax) {
			record.hit = true;
			record.t = t;
			record.beta = beta;
			record.gamma = gamma;
			record.triangle = tri;
			return true;
		}
		return false;
	}

	bool hit(const TriangleIdx &tri, const Ray &r, float tmin, float tmax) const {
		float3 p0 = make_float3(vertex_vec[tri.x]);
		float3 p1 = make_float3(vertex_vec[tri.y]);
		float3 p2 = make_float3(vertex_vec[tri.z]);

		float3 p01 = p0 - p1;
		float A = p01.x;
		float B = p01.y;
		float C = p01.z;

		float3 p02 = p0 - p2;
		float D = p02.x;
		float E = p02.y;
		float F = p02.z;

		float G = r.dir().x;
		float H = r.dir().y;
		float I = r.dir().z;

		float3 p0org = p0 - r.org();
		float J = p0org.x;
		float K = p0org.y;
		float L = p0org.z;

		float EIHF = E * I - H * F;
		float GFDI = G * F - D * I;
		float DHEG = D * H - E * G;

		float denom = (A * EIHF + B * GFDI + C * DHEG);
		if (-ZERO_EPSILON < denom && denom < ZERO_EPSILON) {
			return false;
		}

		float beta = (J * EIHF + K * GFDI + L * DHEG) / denom;
		if (beta < 0.0f || beta > 1.0f) return false;

		float AKJB = A * K - J * B;
		float JCAL = J * C - A * L;
		float BLKC = B * L - K * C;

		float gamma = (I * AKJB + H * JCAL + G * BLKC) / denom;
		if (gamma < 0.0f || beta + gamma > 1.0f) return false;

		float t = -(F * AKJB + E * JCAL + D * BLKC) / denom;
		return (t >= tmin && t <= tmax);
	}

	vector<float3>      vertex_vec;
	vector<TriangleIdx>    triangle_vec;
};

class MeshBvh {
public:
    MeshBvh(TriangleMesh *mesh);
    ~MeshBvh();

    bool hit(const Ray &r, TriangleHitRecord &record, float tmin = RAY_TMIN, float tmax = RAY_TMAX) const;
    bool hit(const Ray &r, float tmin = RAY_TMIN, float tmax = RAY_TMAX) const;
    bool hit(float3 p, float3 q, float tmin = RAY_TMIN) const;

    void getLeafNodes(std::vector<BvhLeafNode> &nodes);
 
    void build();
    void cleanup();

protected:        
    BvhNode *root;    
    BvhNode *nodes;
    int num_nodes;
    int num_levels;
        
    TriangleMesh *mesh;

    TriangleIdx            *sorted_triangles;        
    unsigned int        *sorted_morton;    
    BoundingBox         *sorted_boxes;    
    int                 *org_index;
}; 

