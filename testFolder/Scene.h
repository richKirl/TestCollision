#ifndef SCENE_HPP
#define SCENE_HPP
#include <vector>

enum OBJTYPES
{
    STATIC,
    DINAMYC,
    TERRAIN,
    SKELETALANIMATION,
    LIGHT,
};

//onelevel Scene
template<typename T>
struct Scene {
    Scene *left = nullptr;
    Scene *right = nullptr;
    std::vector<std::pair<OBJTYPES,T>> objects;
    ~Scene()
    {
        delete left;
        delete right;
    }
};
template<typename T>
void buildScene(std::vector<std::pair<OBJTYPES,T>>& objs, int depth = 0){
    Scene *node = new Scene();
    // Создаем изначальный AABB
    // node->box = AABB(glm::vec3(FLT_MAX), glm::vec3(-FLT_MAX));
    for (auto *o : objs)
        node->box.expand(*o);
    if (objs.size() <= 2)
    {
        node->objects = objs;
        return node;
    }
    int axis = depth % 3;
    auto compareAxis = [axis](Object3D *a, Object3D *b)
    {
        float aCoord = (axis == 0) ? a->position.x : (axis == 1) ? a->position.y
                                                                 : a->position.z;
        float bCoord = (axis == 0) ? b->position.x : (axis == 1) ? b->position.y
                                                                 : b->position.z;
        return aCoord < bCoord;
    };
    std::sort(objs.begin(), objs.end(), compareAxis);
    size_t mid = objs.size() / 2;
    std::vector<std::pair<OBJTYPES,T>> leftObjs(objs.begin(), objs.begin() + mid);
    std::vector<std::pair<OBJTYPES,T>> rightObjs(objs.begin() + mid, objs.end());
    // std::cout<<depth<<std::endl;
    node->left = buildScene(leftObjs, depth + 1);
    node->right = buildScene(rightObjs, depth + 1);
    return node;
}
void traverseScene();
void RenderScene();

#endif // SCENE_HPP