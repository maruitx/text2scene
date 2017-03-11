#pragma once

#include "bvh_common.h"

using namespace Nvidia;

#include <iostream>
#include <queue>
#include <stack>
using namespace std;


struct BoxHitRecord {
    float t;
    bool hit;
    int index;

    BoxHitRecord() : hit(false), t(RAY_TMAX), index(-1) {}
};

struct SimpleBoxNodeData {
	BoundingBox box;
	std::vector<int> triangles;
};

template <class NodeData>
struct GeometryPredicate {
    virtual bool operator()(const NodeData &a, const NodeData &b) const = 0;
};

template <class NodeData>
class BoxBvh {
public:
    BoxBvh(const std::vector<NodeData> &data);
    ~BoxBvh();

    bool hit(const Ray &r, BoxHitRecord &record, float tmin = RAY_TMIN, float tmax = RAY_TMAX) const;
    bool hit(const Ray &r, float tmin = RAY_TMIN, float tmax = RAY_TMAX) const;
    bool hit(float3 p, float3 q, float tmin = RAY_TMIN) const;
    bool hit(const BoxBvh<NodeData> &another, const GeometryPredicate<NodeData> &predicate) const;

protected:    
    void build();
    void cleanup();

protected:        
    BvhNode *root;    
    BvhNode *nodes;
    int num_nodes;
    int num_levels;
        
    std::vector<NodeData> data;         // we save a data version locally
   
    unsigned int        *sorted_morton;    
    BoundingBox         *sorted_boxes;    
    int                 *org_index;
}; 

template <class NodeData>
BoxBvh<NodeData>::BoxBvh(const std::vector<NodeData> &data) : data(data),
sorted_morton(NULL), sorted_boxes(NULL),
root(NULL), nodes(NULL), num_nodes(0)
{
    build();
}

template <class NodeData>
BoxBvh<NodeData>::~BoxBvh() {
    cleanup();
}

template <class NodeData>
void BoxBvh<NodeData>::cleanup() {
    if (sorted_morton)      delete[] sorted_morton;
    if (sorted_boxes)       delete[] sorted_boxes;
    if (nodes)              delete[] nodes;
}

static void get_depth_first_order(BvhNode *nodes, int cur, int *order, int &num_nodes_so_far) {
    order[cur] = num_nodes_so_far;
    num_nodes_so_far++;

    BvhNode &node = nodes[cur];
    if (node.index_left >= 0)
        get_depth_first_order(nodes, node.index_left, order, num_nodes_so_far);
    if (node.index_right >= 0)
        get_depth_first_order(nodes, node.index_right, order, num_nodes_so_far);
}

template <class NodeData>
void BoxBvh<NodeData>::build() {
    if (data.size() <= 0) return;
    int num_boxes = data.size();

    BoundingBox box;    // global bounding box
    BoundingBox *boxes = new BoundingBox[num_boxes];  // leaf
    for (int i = 0; i < data.size(); ++i) {
        boxes[i] = data[i].box;
        box.merge(boxes[i]);
    }

    //cout << "BVH statistics:" << endl;
    //Stats stats;

    //cout << "Building Morton code..." << endl;
    //stats.tic();
    unsigned int *morton = new unsigned int[num_boxes];
    for (int i = 0; i < num_boxes; ++i) {
        morton[i] = morton_box(&boxes[i], &box);
    }
    //stats.toc();
    //stats.print_elapsed_milliseconds();

    //cout << "Sorting Morton array..." << endl;
    //stats.tic();
    MortonEntry *entries;
    sort_morton_array(morton, num_boxes, entries);

    // permute triangles
    sorted_morton = new unsigned int[num_boxes];
    sorted_boxes = new BoundingBox[num_boxes];
    org_index = new int[num_boxes];
    for (int i = 0; i < num_boxes; ++i) {
        sorted_morton[i] = entries[i].code;
        int original_idx = entries[i].index;
        sorted_boxes[i] = boxes[original_idx];
        org_index[i] = original_idx;
    }
    delete[] morton;
    delete[] boxes;
    delete[] entries;
    //stats.toc();
    //stats.print_elapsed_milliseconds();

    //cout << "Building levels..." << endl;
    //stats.tic();
    // build all levels    
    int num_estimated_levels = (int)ceil(log(num_boxes) / log(2.0f));    // is it a good bound?
    int capacity = (int)pow(2, num_estimated_levels + 1);
    nodes = new BvhNode[capacity];

    // the queue
    int first = 0,                      // current to-pop node
        last = 1,                       // next empty node
        next_last = 1;                  // next level

    num_nodes = 1;
    root = nodes;
    root->start = 0;
    root->end = num_boxes - 1;      // inclusive

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

        }
        else {

            int split = Nvidia::findSplit(sorted_morton, start, end);

            node->index_left = next_last;
            node->index_right = next_last + 1;
            next_last += 2;
            num_nodes += 2;

            BvhNode *left = &nodes[node->index_left];
            BvhNode *right = &nodes[node->index_right];
            left->start = start;
            left->end = split;
            right->start = split + 1;
            right->end = end;
        }

        first++;
        if (first == last) {                // level end
            if (next_last == last) {
                break;                      // no nodes queued, done
            }
            else {
                last = next_last;           // go to next level
                levels.push_back(Level(first, last));
                level++;
            }
        }
    }
    //cout << "BVH nodes/capacity : " << num_nodes << "/" << capacity << endl;
    //cout << "BVH levels         : " << levels.size() << endl;
    num_levels = levels.size();
    //stats.toc();
    //stats.print_elapsed_milliseconds();

    //cout << "Linking levels..." << endl;
    //stats.tic();
    for (int i = levels.size() - 1; i >= 0; --i) {
        int first = levels[i].first;
        int last = levels[i].second;

        for (int j = first; j < last; ++j) {
            BvhNode *node = &nodes[j];

            if (node->index_left < 0) {
                if (node->index_right < 0) {
                    node->box = sorted_boxes[node->start];
                }
                else {
                    node->box = nodes[node->index_right].box;
                }
            }
            else {
                if (node->index_right < 0) {
                    node->box = nodes[node->index_left].box;
                }
                else {
                    node->box = nodes[node->index_left].box;
                    node->box.merge(nodes[node->index_right].box);
                }
            }
        }
    }
    //stats.toc();
    //stats.print_elapsed_milliseconds();
}


template <class NodeData>
bool BoxBvh<NodeData>::hit(const Ray &r, BoxHitRecord &record, float tmin, float tmax) const {
    record.hit = false;
    if (!root) return false;
    if (!root->box.hit(r, tmin, tmax)) return false;

    Stack q;
    q.push(0);

    while (!q.is_empty()) {
        int node_index = q.pop();
        BvhNode &node = nodes[node_index];
        if (node.index_left < 0 && node.index_right < 0) {
            // leaf node found
            if (record.hit == false || record.t > tmax) {
                record.t = tmax;
                record.index = org_index[node.start];
            }
            record.hit = true;
        }
        else {

            if (node.index_left >= 0 && nodes[node.index_left].box.hit(r, tmin, tmax))
                q.push(node.index_left);
            if (node.index_right >= 0 && nodes[node.index_right].box.hit(r, tmin, tmax))
                q.push(node.index_right);
        }
    }
    return record.hit;
}

template <class NodeData>
bool BoxBvh<NodeData>::hit(const Ray &r, float tmin, float tmax) const {
    if (!root) return false;
    if (!root->box.hit(r, tmin, tmax)) return false;

    Stack q;
    q.push(0);

    while (!q.is_empty()) {
        BvhNode &node = nodes[q.pop()];

        if (node.index_left < 0 && node.index_right < 0) {

            return true;

        }
        else {

            if (node.index_left >= 0 && nodes[node.index_left].box.hit(r, tmin, tmax))
                q.push(node.index_left);
            if (node.index_right >= 0 && nodes[node.index_right].box.hit(r, tmin, tmax))
                q.push(node.index_right);

        }
    }
    return false;
}

template <class NodeData>
bool BoxBvh<NodeData>::hit(float3 p, float3 q, float tmin) const {
    Ray shadow(p, q - p);
    float shadow_tmax = norm(q - p) - tmin;  // deduct an epsilon amount to avoid
    // the case where q is 'behind' the surface of 
    // itself.
    return this->hit(shadow, tmin, shadow_tmax);
}

template <class NodeData>
bool BoxBvh<NodeData>::hit(const BoxBvh<NodeData> &another, 
                           const GeometryPredicate<NodeData> &predicate) const 
{
    if (!root) return false;
    if (!another.root) return false;
    
    typedef std::pair<int, int> Pair;
    std::stack<Pair> q;
    q.push(Pair(0, 0));

    while (!q.empty()) {
        Pair pair = q.top();
        q.pop();
        
        BvhNode *first = &nodes[pair.first];
        BvhNode *second = &another.nodes[pair.second];
        BoundingBox b1 = nodes[pair.first].box;
        BoundingBox b2 = another.nodes[pair.second].box;
        if (b1.overlap(b2) || b1.contains(b2)) {
            if (first->index_left < 0 && first->index_right < 0 &&
                second->index_left < 0 && second->index_right < 0) 
            {
                const NodeData &d1 = data[org_index[first->start]];
                const NodeData &d2 = another.data[another.org_index[second->start]];
                if (predicate(d1, d2)) return true;
            }
            
            // when each node has at least a child            
            if (first->index_left >= 0 && second->index_left >= 0)
                q.push(Pair(first->index_left, second->index_left));

            if (first->index_left >= 0 && second->index_right >= 0)
                q.push(Pair(first->index_left, second->index_right));

            if (first->index_right >= 0 && second->index_left >= 0)
                q.push(Pair(first->index_right, second->index_left));

            if (first->index_right >= 0 && second->index_right >= 0)
                q.push(Pair(first->index_right, second->index_right));
            
            // when one of the node is leaf but the other is not
            if (first->index_left < 0 && first->index_right < 0 && second->index_left >= 0)
                q.push(Pair(pair.first, second->index_left));

            if (first->index_left < 0 && first->index_right < 0 && second->index_right >= 0)
                q.push(Pair(pair.first, second->index_right));

            if (first->index_left >= 0 && second->index_left < 0 && second->index_right < 0)
                q.push(Pair(first->index_left, pair.second));

            if (first->index_right >= 0 && second->index_left < 0 && second->index_right < 0)
                q.push(Pair(first->index_right, pair.second));
        }
    }
    return false;
}
