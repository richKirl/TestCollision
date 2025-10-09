#include "Collision.h"


bool checkCollision(const Object3D &a, const Object3D &b)
{
    if (a.isSphere && b.isSphere)
    {
        // Проверка сфер
        float distSq = glm::dot(a.position - b.position, a.position - b.position);
        float radiusSum = a.radius.x + b.radius.x; // предполагаю радиусы одинаковые по x,y,z
        return distSq <= radiusSum * radiusSum;
    }
    else
    {
        // Проверка кубов (или смешанных)
        // Создаем AABB для каждого и проверяем пересечение
        AABB aabbA;
        if (a.isSphere)
        {
            aabbA.min = a.position - glm::vec3(a.radius);
            aabbA.max = a.position + glm::vec3(a.radius);
        }
        else
        {
            aabbA.min = a.position - glm::vec3(a.size.x * 0.5f, a.size.y * 0.5f, a.size.z * 0.5f);
            aabbA.max = a.position + glm::vec3(a.size.x * 0.5f, a.size.y * 0.5f, a.size.z * 0.5f);
        }

        AABB aabbB;
        if (b.isSphere)
        {
            aabbB.min = b.position - glm::vec3(b.radius);
            aabbB.max = b.position + glm::vec3(b.radius);
        }
        else
        {
            aabbB.min = b.position - glm::vec3(b.size.x * 0.5f, b.size.y * 0.5f, b.size.z * 0.5f);
            aabbB.max = b.position + glm::vec3(b.size.x * 0.5f, b.size.y * 0.5f, b.size.z * 0.5f);
        }

        return aabbA.intersects(aabbB);
    }
}


void resolveCollision(Object3D &a, Object3D &b)
{
    // Предположим, что масса обратно пропорциональна объему

    float massA = a.stayObj ? 1e9f : glm::length(a.isSphere ? a.radius : a.size);
    float massB = b.stayObj ? 1e9f : glm::length(b.isSphere ? b.radius : b.size);

    // Вектор столкновения
    glm::vec3 collisionNormal = glm::normalize(b.position - a.position);
    float relativeVelocity = glm::dot(b.velocity - a.velocity, collisionNormal);

    if (relativeVelocity > 0)
        return; // объекты удаляются друг от друга

    // Расчет импульса
    float restitution = 0.8f; // или любой другой параметр
    float impulseMag = -(1.0f + restitution) * relativeVelocity / (1 / massA + 1 / massB);

    glm::vec3 impulse = impulseMag * collisionNormal;

    if (!a.stayObj)
        a.velocity -= (1 / massA) * impulse;
    if (!b.stayObj)
        b.velocity += (1 / massB) * impulse;
}


void traverseBVH(BVHNode *node, Object3D *obj)
{
    //   AABB aabbExpanded = getExpandedAABB(*obj, 0.16f);
    // if (!node || !node->box.intersects(aabbExpanded)) return;
    if (!node || !node->box.intersects(AABB(obj->position - (obj->isSphere ? obj->radius : obj->size * 0.5f),
                                            obj->position + (obj->isSphere ? obj->radius : obj->size * 0.5f))))
        return;
    if (node->isLeaf())
    {
        for (auto *other : node->objects)
        {
            if (other != obj && checkCollision(*obj, *other))
            {
                if (other->stayObj)
                {
                    obj->velocity *= -1;
                    reflectVelocity(*obj, other->NormalWall);
                    obj->collided = true;
                }
                else
                {
                    resolveCollision(*obj, *other);
                    obj->collided = true;
                    other->collided = true;
                }
            }
        }
    }
    else
    {
        traverseBVH(node->left, obj);
        traverseBVH(node->right, obj);
    }
}

void checkCollisionsBVH(BVHNode *node, Object3D *obj, float deltaTime)
{
    AABB sweptAABB = getSweptAABB(*obj, obj->velocity, deltaTime);
    if (!node || !node->box.intersects(sweptAABB))
        return;

    if (node->isLeaf())
    {
        for (auto *other : node->objects)
        {
            if (other != obj)
            {
                // Проверка пересечения AABB
                AABB otherAABB;
                if (other->isSphere)
                {
                    otherAABB.min = other->position - glm::vec3(other->radius);
                    otherAABB.max = other->position + glm::vec3(other->radius);
                }
                else
                {
                    otherAABB.min = other->position - glm::vec3(other->size * 0.5f);
                    otherAABB.max = other->position + glm::vec3(other->size * 0.5f);
                }

                if (sweptAABB.intersects(otherAABB))
                {
                    // Обработка столкновения (например, resolveCollision)
                    if (other->stayObj)
                    {
                        obj->velocity *= -1;
                        reflectVelocity(*obj, other->NormalWall);
                        obj->collided = true;
                    }
                    else
                    {
                        resolveCollision(*obj, *other);
                        obj->collided = true;
                        other->collided = true;
                    }
                }
            }
        }
    }
    else
    {
        checkCollisionsBVH(node->left, obj, deltaTime);
        checkCollisionsBVH(node->right, obj, deltaTime);
    }
}

// int counterRR = 0;
void traverseBVHR(BVHNode *node, Object3D *obj)
{
    // if(counterRR >= 100){counterRR=0;}
    // if(counterRR<100){
    AABB aabbExpanded = getExpandedAABB(*obj, 0.16f);
    if (!node || !node->box.intersects(aabbExpanded))
        return;
    //}
    // else {
    //   if (!node ||
    //       !node->box.intersects(
    // 			      AABB(obj->position - (obj->isSphere ? obj->radius : obj->size*0.5f),
    // 				   obj->position + (obj->isSphere ? obj->radius : obj->size*0.5f))))
    //     return;
    // }
    if (node->isLeaf())
    {
        for (auto *other : node->objects)
        {
            // Создаем AABB объекта
            AABB aabb;
            if (other->isSphere)
            {
                aabb.min = other->position - glm::vec3(other->radius * 2.5f);
                aabb.max = other->position + glm::vec3(other->radius * 2.5f);
            }
            else
            {
                aabb.min = other->position - glm::vec3(other->size * 0.5f);
                aabb.max = other->position + glm::vec3(other->size * 0.5f);
            }
            glm::vec3 startPos = obj->prevpos;
            glm::vec3 endPos = obj->position; // текущая позиция
            // Ray ray = CreateRay(startPos, endPos);
            Ray ray = CreateRay(obj->position, obj->position + (obj->velocity * 2000000.0f));
            if (other != obj && RayCast(aabb, ray))
            {
                if (other->stayObj)
                {
                    obj->velocity *= -1;
                    reflectVelocity(*obj, other->NormalWall);
                    obj->collided = true;
                    // break;
                }
                else
                {
                    resolveCollision(*obj, *other);
                    obj->collided = true;
                    other->collided = true;
                    // break;
                }
            }
        }
    }
    else
    {
        // counterRR++;
        // if (counterRR < 100) {
        //   traverseBVH(node->left, obj);
        //   traverseBVH(node->right, obj);
        // }
        // else {
        traverseBVHR(node->left, obj);
        traverseBVHR(node->right, obj);
        //}
    }
}