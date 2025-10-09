#ifndef COLLISION_HPP
#define COLLISION_HPP
#include <glm/glm.hpp>
#include "AABB.h"
#include "BVH.h"
#include "Object3D.h"
#include "Ray.h"

bool checkCollision(const Object3D &a, const Object3D &b);

void resolveCollision(Object3D &a, Object3D &b);

void reflectVelocity(Object3D &obj, const glm::vec3 &normal)
{
    obj.velocity = obj.velocity - 2.0f * glm::dot(obj.velocity, normal) * normal;
}

void traverseBVH(BVHNode *node, Object3D *obj);

void checkCollisionsBVH(BVHNode *node, Object3D *obj, float deltaTime);

int counterRR = 0;
void traverseBVHR(BVHNode *node, Object3D *obj);

#endif // COLLSION_HPP