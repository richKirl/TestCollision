#include "Ray.h"

Ray CreateRay(const glm::vec3 &p1, const glm::vec3 &p2)
{
    glm::vec3 dir = p2 - p1;
    glm::vec3 invDir;
    invDir.x = (fabs(dir.x) > 1e-8) ? 1.0f / dir.x : FLT_MAX;
    invDir.y = (fabs(dir.y) > 1e-8) ? 1.0f / dir.y : FLT_MAX;
    invDir.z = (fabs(dir.z) > 1e-8) ? 1.0f / dir.z : FLT_MAX;
    return {p1, dir, invDir};
}


bool RayCast(const AABB &box, const Ray &ray)
{
    float tmin = -FLT_MAX;
    float tmax = FLT_MAX;

    for (int i = 0; i < 3; ++i)
    {
        float invD = (i == 0) ? ray.invDirection.x : (i == 1) ? ray.invDirection.y
                                                              : ray.invDirection.z;
        float originComponent = (i == 0) ? ray.origin.x : (i == 1) ? ray.origin.y
                                                                   : ray.origin.z;
        float minB = (i == 0) ? box.min.x : (i == 1) ? box.min.y
                                                     : box.min.z;
        float maxB = (i == 0) ? box.max.x : (i == 1) ? box.max.y
                                                     : box.max.z;

        if (invD == FLT_MAX) // параллельно оси
        {
            if (originComponent < minB || originComponent > maxB)
                return false;
        }
        else
        {
            float t0 = (minB - originComponent) * invD;
            float t1 = (maxB - originComponent) * invD;
            if (invD < 0.0f)
                std::swap(t0, t1);
            tmin = std::max(tmin, t0);
            tmax = std::min(tmax, t1);
            if (tmax < tmin)
                return false;
        }
    }
    return true;
}