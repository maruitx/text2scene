#include "mesh_bvh.h"

extern "C" {
    #include "morton.h"
}
using namespace Nvidia;

//extern "C" {
    //#include "../geometry/triangle_triangle_intersection.c"
//}

#include "triangle_triangle_intersection.h"
#include "Model.h"

#include <iostream>
#include <queue>
using namespace std;


TrianglePredicate::TrianglePredicate(Model *first, mat4 &firstTransform, Model *second, mat4 &secondTransform)
{
	model[0] = first, model[1] = second;
	transform[0] = firstTransform, transform[1] = secondTransform;
}

bool TrianglePredicate::operator()(const SimpleBoxNodeData &a, const SimpleBoxNodeData &b) const
{
	// retrieve triangle coordinates, transformed, and perform triangle test 
	for (int i = 0; i < a.triangles.size(); ++i) {
		vec3 p1, q1, r1;
		model[0]->getTriangle(a.triangles[i], p1, q1, r1);
		p1 = TransformPoint(transform[0], p1);
		q1 = TransformPoint(transform[0], q1);
		r1 = TransformPoint(transform[0], r1);
		for (int j = 0; j < b.triangles.size(); ++j) {
			vec3 p2, q2, r2;
			model[1]->getTriangle(b.triangles[j], p2, q2, r2);
			p2 = TransformPoint(transform[1], p2);
			q2 = TransformPoint(transform[1], q2);
			r2 = TransformPoint(transform[1], r2);

			if (Guigue::tri_tri_overlap_test_3d(&p1.x, &q1.x, &r1.x,
				&p2.x, &q2.x, &r2.x)) return true;
		}
	}
	return false;
}

MeshBvh::MeshBvh(TriangleMesh *mesh) : mesh(mesh),
    sorted_triangles(NULL), sorted_morton(NULL), sorted_boxes(NULL),
    root(NULL), nodes(NULL), num_nodes(0)
{    

}

MeshBvh::~MeshBvh() {
    cleanup();    
}

void MeshBvh::cleanup() {
    if (sorted_triangles)   delete [] sorted_triangles;
    if (sorted_morton)      delete [] sorted_morton;
    if (sorted_boxes)       delete [] sorted_boxes;
    if (nodes)              delete [] nodes;
}

void MeshBvh::build() {    
    if (mesh->triangle_vec.size() <= 0) return;
    int num_triangles = mesh->triangle_vec.size();
    TriangleIdx *triangles = new TriangleIdx[num_triangles];
    BoundingBox *boxes = new BoundingBox[num_triangles];
    int *surface_id = new int[num_triangles];
    BoundingBox box = mesh->get_mesh_bounding_box();
    for (int i = 0; i < mesh->triangle_vec.size(); ++i) {
        surface_id[i] = i;
        triangles[i] = mesh->triangle_vec[i];
        boxes[i] = mesh->get_triangle_bounding_box(triangles[i]);
    }
    
    //cout << "BVH statistics:" << endl;
    //Stats stats;

    //cout << "Building Morton code..." << endl;
    //stats.tic();
    unsigned int *morton = new unsigned int[num_triangles];
    for (int i = 0; i < num_triangles; ++i) {
        morton[i] = morton_box(&boxes[i], &box);
    }
    //stats.toc();
    //stats.print_elapsed_milliseconds();
        
    //cout << "Sorting Morton array..." << endl;
    //stats.tic();
    MortonEntry *entries;
    sort_morton_array(morton, num_triangles, entries);    

    // permute triangles
    sorted_triangles    = new TriangleIdx[num_triangles];
    sorted_morton       = new unsigned int[num_triangles];
    sorted_boxes        = new BoundingBox[num_triangles];
    org_index           = new int[num_triangles];    
    for (int i = 0; i < num_triangles; ++i) {                
        sorted_morton[i]        = entries[i].code;
        int original_idx        = entries[i].index;
        sorted_triangles[i]     = triangles[original_idx];        
        sorted_boxes[i]         = boxes[original_idx];
        org_index[i]            = original_idx;
    }
    delete [] triangles;
    delete [] morton;
    delete [] surface_id;
    delete [] boxes;
    delete [] entries;
    //stats.toc();
    //stats.print_elapsed_milliseconds();

    //cout << "Building levels..." << endl;
    //stats.tic();
    // build all levels    
    int num_estimated_levels = (int)ceil(log(num_triangles) / log(2.0f));    // is it a good bound?
    int capacity = (int)pow(2, num_estimated_levels + 1);
    nodes = new BvhNode[capacity];
    
    // the queue
    int first = 0,                      // current to-pop node
        last = 1,                       // next empty node
        next_last = 1;                  // next level

    num_nodes = 1;
    root = nodes;
    root->start = 0;
    root->end = num_triangles - 1;      // inclusive
    
    typedef pair<int, int> Level;       // record level info for bottom-up box building
    vector<Level> levels;
    levels.push_back(Level(0, 1));
    int level = 0;
    while (true) {        
        BvhNode* node = &nodes[first]; 
        node->level = level;

        int start = node->start;
        int end = node->end;
        if (start == end) {
            node->index_left = -1;
            node->index_right = -1;

        } else {

            int split = Nvidia::findSplit(sorted_morton, start, end);
            
            node->index_left    = next_last;
            node->index_right   = next_last + 1;
            next_last += 2;
            num_nodes += 2;            

            BvhNode *left  = &nodes[node->index_left];
            BvhNode *right = &nodes[node->index_right];
            left->start     = start;
            left->end       = split;
            right->start    = split + 1;
            right->end      = end; 
        }

        first++;
        if (first == last) {                // level end
            if (next_last == last) {
                break;                      // no nodes queued, done
            } else {                
                last = next_last;           // go to next level
                levels.push_back(Level(first, last));
                level++;
            }
        }
    }
   // cout << "BVH nodes/capacity : " << num_nodes << "/" << capacity << endl;
   // cout << "BVH levels         : " << levels.size() << endl;
    num_levels = levels.size();
    //stats.toc();
    //stats.print_elapsed_milliseconds();

   // cout << "Linking levels..." << endl;
    //stats.tic();
    for (int i = levels.size() - 1; i >= 0; --i) {
        int first = levels[i].first;
        int last = levels[i].second;

        for (int j = first; j < last; ++j) {
            BvhNode *node = &nodes[j];

            if (node->index_left < 0) {
                if (node->index_right < 0) {
                    node->box = sorted_boxes[node->start];                    
                } else {
                    node->box = nodes[node->index_right].box;                    
                }
            } else {
                if (node->index_right < 0) {
                    node->box = nodes[node->index_left].box;                    
                } else {
                    node->box = nodes[node->index_left].box;
                    node->box.merge(nodes[node->index_right].box);                    
                }
            }
        }
    }
    //stats.toc();
    //stats.print_elapsed_milliseconds();
}


bool MeshBvh::hit(const Ray &r, TriangleHitRecord &record, float tmin, float tmax) const {
    if (! root) return false;
    if (! root->box.hit(r, tmin, tmax)) return false;

    Stack q;
    q.push(0);

    bool hit = false;
    int triangle_index;
    TriangleHitRecord best_gh;
    while (! q.is_empty()) {
        int node_index = q.pop();
        BvhNode &node = nodes[node_index];
        if (node.index_left < 0 && node.index_right < 0) {                        

            TriangleIdx &tri = sorted_triangles[node.start];            
            TriangleHitRecord gh;
            if (mesh->hit(tri, r, tmin, tmax, gh)) {
                tmax = gh.t;                // update the best tmax so far to prune quickly
                triangle_index = node.start;
                gh.mesh_triangle_index = org_index[triangle_index];
                best_gh = gh;
                hit = true;
            }

        }  else {

            if (node.index_left >= 0 && nodes[node.index_left].box.hit(r, tmin, tmax)) 
                q.push(node.index_left);
            if (node.index_right >= 0 && nodes[node.index_right].box.hit(r, tmin, tmax))
                q.push(node.index_right);
        }
    }
    if (hit) {
        record = best_gh;
    }
    return hit;
}

bool MeshBvh::hit(const Ray &r, float tmin, float tmax) const {
    if (! root) return false;
    if (! root->box.hit(r, tmin, tmax)) return false;

    Stack q;
    q.push(0);
        
    while (! q.is_empty()) {
        BvhNode &node = nodes[q.pop()];        

        if (node.index_left < 0 && node.index_right < 0) {                        

            TriangleIdx &tri = sorted_triangles[node.start];            
            if (mesh->hit(tri, r, tmin, tmax)) return true;

        }  else {
            
            if (node.index_left >= 0 && nodes[node.index_left].box.hit(r, tmin, tmax))
                q.push(node.index_left);
            if (node.index_right >= 0 && nodes[node.index_right].box.hit(r, tmin, tmax))
                q.push(node.index_right);

        }
    }
    return false;
}

bool MeshBvh::hit(float3 p, float3 q, float tmin) const {
    Ray shadow(p, q - p);
    float shadow_tmax = norm(q - p) - tmin;  // deduct an epsilon amount to avoid
                                             // the case where q is 'behind' the surface of 
                                             // itself.
    return this->hit(shadow, tmin, shadow_tmax);
}

void MeshBvh::getLeafNodes(std::vector<BvhLeafNode> &output) {
    output.clear();
    
    for (int i = 0; i < num_nodes; ++i) {
        if (nodes[i].index_left < 0 && nodes[i].index_right < 0) {
            int triangle_index = org_index[nodes[i].start];

            BvhLeafNode node;
            node.triangles.push_back(triangle_index);
            node.box = nodes[i].box;
            output.push_back(node);
        }
    }
}
