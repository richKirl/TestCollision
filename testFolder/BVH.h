#ifndef BVH_HPP
#define BVH_HPP
#include <vector>
#include <algorithm>

#include "AABB.h"
struct BVHNode
{
    AABB box;
    BVHNode *left = nullptr;
    BVHNode *right = nullptr;
    std::vector<Object3D *> objects; // листовой узел
    bool isLeaf() const { return !objects.empty(); }
    ~BVHNode()
    {
        delete left;
        delete right;
    }
};

BVHNode *buildBVH(std::vector<Object3D *> &objs, int depth = 0);

#endif // BVH_HPP