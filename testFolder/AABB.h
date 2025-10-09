#ifndef AABB_HPP
#define AABB_HPP
#include "Object3D.h"
#include <glm/glm.hpp>

struct AABB
{
    glm::vec3 min, max;
    AABB() : min(glm::vec3(0)), max(glm::vec3(0)) {}
    AABB(const glm::vec3 &min_, const glm::vec3 &max_) : min(min_), max(max_) {}
    void expand(const Object3D &obj)
    {
        glm::vec3 minO, maxO;
        if (obj.isSphere)
        {
            minO = obj.position - glm::vec3(obj.radius);
            maxO = obj.position + glm::vec3(obj.radius);
        }
        else
        {
            minO = obj.position - glm::vec3(obj.size.x * 0.5f, obj.size.y * 0.5f, obj.size.z * 0.5f);
            maxO = obj.position + glm::vec3(obj.size.x * 0.5f, obj.size.y * 0.5f, obj.size.z * 0.5f);
        }
        min = glm::min(min, minO);
        max = glm::max(max, maxO);
    }
    bool intersects(const AABB &other) const
    {
        return (max.x >= other.min.x && min.x <= other.max.x) &&
               (max.y >= other.min.y && min.y <= other.max.y) &&
               (max.z >= other.min.z && min.z <= other.max.z);
    }
};

AABB getExpandedAABB(const Object3D &obj, float dt);

AABB getSweptAABB(const Object3D &obj, const glm::vec3 &velocity, float deltaTime);

#endif // AABB_HPP