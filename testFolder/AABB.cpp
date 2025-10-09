#include "AABB.h"


AABB getExpandedAABB(const Object3D &obj, float dt)
{
    glm::vec3 startPos = obj.prevpos;
    glm::vec3 endPos = obj.position;
    glm::vec3 minPos = glm::min(startPos, endPos);
    glm::vec3 maxPos = glm::max(startPos, endPos);
    glm::vec3 size = (maxPos - minPos);
    // Добавьте радиусы или размеры
    if (obj.isSphere)
    {
        minPos -= glm::vec3(obj.radius);
        maxPos += glm::vec3(obj.radius);
    }
    else
    {
        minPos -= glm::vec3(obj.size * 0.5f);
        maxPos += glm::vec3(obj.size * 0.5f);
    }
    return AABB(minPos, maxPos);
}



AABB getSweptAABB(const Object3D &obj, const glm::vec3 &velocity, float deltaTime)
{
    glm::vec3 startMin, startMax;
    if (obj.isSphere)
    {
        startMin = obj.position - glm::vec3(obj.radius);
        startMax = obj.position + glm::vec3(obj.radius);
    }
    else
    {
        startMin = obj.position - glm::vec3(obj.size * 0.5f);
        startMax = obj.position + glm::vec3(obj.size * 0.5f);
    }

    glm::vec3 endPos = obj.position + velocity * deltaTime;

    glm::vec3 endMin, endMax;
    if (obj.isSphere)
    {
        endMin = endPos - glm::vec3(obj.radius);
        endMax = endPos + glm::vec3(obj.radius);
    }
    else
    {
        endMin = endPos - glm::vec3(obj.size * 0.5f);
        endMax = endPos + glm::vec3(obj.size * 0.5f);
    }

    glm::vec3 minCoords = glm::min(startMin, endMin);
    glm::vec3 maxCoords = glm::max(startMax, endMax);

    return AABB(minCoords, maxCoords);
}