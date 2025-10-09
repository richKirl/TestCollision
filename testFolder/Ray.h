#ifndef RAY_HPP
#define RAY_HPP

#include <algorithm>
#include <glm/glm.hpp>
#include "AABB.h"
// Создание луча
struct Ray
{
    glm::vec3 origin;
    glm::vec3 direction;
    glm::vec3 invDirection;
};

Ray CreateRay(const glm::vec3 &p1, const glm::vec3 &p2);

// Проверка пересечения луча с AABB
bool RayCast(const AABB &box, const Ray &ray);

#endif // RAY_HPP