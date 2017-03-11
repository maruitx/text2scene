#pragma once

#include "bvh_common.h"

/** 
 * CPU implementation of linear BVH.
 */


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

