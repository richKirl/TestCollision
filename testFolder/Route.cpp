#include "Route.h"

float NodeP::costToNeighbor(int neighborIndex, const std::vector<NodeP> &nodes) const
{

    const NodeP &b = nodes[neighborIndex];

    float dx = static_cast<float>(b.x - x);
    float dz = static_cast<float>(b.z - z);
    float dy = b.height - height;

    float horizontalDistance = std::sqrt(dx * dx + dz * dz);

    // Расчет наклона в градусах
    float angleRadians = std::atan2(dy, horizontalDistance);
    float angleDegrees = angleRadians * (180.0f / M_PI);

    // Порог наклона (например, 30 градусов)
    const float maxSlopeDegrees = 16.0f;
    float penaltyFactor = 10.0f; // коэффициент штрафа, подберите по необходимости
    float penalty = 0.0f;

    if (abs(angleDegrees) > maxSlopeDegrees)
    {
        penalty = (abs(angleDegrees) - maxSlopeDegrees) * penaltyFactor;
    }
    // else penalty-=10.0f;
    //  Базовая стоимость перемещения
    float baseCost = std::sqrt(horizontalDistance * horizontalDistance + dy * dy);

    // Итоговая стоимость с штрафом
    return baseCost + penalty;
}

void createPathsMap(NodesS *nodesS,Heightmap *heightmap)
{
    nodesS->nodes.clear();
    // Создание узлов
    for (int z = 0; z < terrainSize; ++z)
    {
        for (int x = 0; x < terrainSize; ++x)
        {
            NodeP node;
            node.x = x;
            node.z = z;
            node.height = getHeightAt(heightmap, x - 256, z - 256) - 239.0f;
            nodesS->nodes.push_back(node);
        }
    }
    // Построение связей с 8 соседями
    for (int z = 0; z < terrainSize; ++z)
    {
        for (int x = 0; x < terrainSize; ++x)
        {
            int index = z * terrainSize + x;

            if (x > 0)
                nodesS->nodes[index].neighbors.push_back(index - 1);
            if (x < terrainSize - 1)
                nodesS->nodes[index].neighbors.push_back(index + 1);
            if (z > 0)
                nodesS->nodes[index].neighbors.push_back(index - terrainSize);
            if (z < terrainSize - 1)
                nodesS->nodes[index].neighbors.push_back(index + terrainSize);

            if (x > 0 && z > 0)
                nodesS->nodes[index].neighbors.push_back(index - terrainSize - 1);
            if (x > 0 && z < terrainSize - 1)
                nodesS->nodes[index].neighbors.push_back(index + terrainSize - 1);
            if (x < terrainSize - 1 && z > 0)
                nodesS->nodes[index].neighbors.push_back(index - terrainSize + 1);
            if (x < terrainSize - 1 && z < terrainSize - 1)
                nodesS->nodes[index].neighbors.push_back(index + terrainSize + 1);
        }
    }
}

float getCost(int aIndex, int bIndex, const std::vector<NodeP> &nodes)
{
    return nodes[aIndex].costToNeighbor(bIndex, nodes);
}

std::vector<int> dijkstra(const std::vector<NodeP> &nodes, int startIdx, int goalIdx, float &totalCost)
{
    size_t n = nodes.size();

    std::vector<float> dist(n, std::numeric_limits<float>::max());
    std::vector<int> prev(n, -1);
    dist[startIdx] = 0.0f;

    using P = std::pair<float, int>;
    std::priority_queue<P, std::vector<P>, std::greater<P>> queue;
    queue.emplace(0.0f, startIdx);

    totalCost = 0.0f;

    while (!queue.empty())
    {
        auto [currentDist, u] = queue.top();
        queue.pop();

        if (u == goalIdx)
            break;

        if (currentDist > dist[u])
            continue;

        for (int neighbor : nodes[u].neighbors)
        {
            float cost = getCost(u, neighbor, nodes);
            float newDist = dist[u] + cost;
            if (newDist < dist[neighbor])
            {
                dist[neighbor] = newDist;
                prev[neighbor] = u;
                queue.emplace(newDist, neighbor);
            }
        }
    }

    // Восстановление пути и подсчет стоимости
    std::vector<int> path;
    totalCost = dist[goalIdx];
    for (int at = goalIdx; at != -1; at = prev[at])
    {
        path.push_back(at);
    }
    std::reverse(path.begin(), path.end());
    return path;
}

void initObject(NodesS *nodesS,MovingObject &obj, std::vector<int> &route, float moveSpeed)
{
    obj.p = route;
    obj.position = glm::vec3(nodesS->nodes[route[0]].x, nodesS->nodes[route[0]].height, nodesS->nodes[route[0]].z);
    obj.targetIndex = 1; // следующая точка маршрута
    obj.progress = 0.0f;
    obj.speed = moveSpeed;
}

void updateObject(NodesS *nodesS,MovingObject &obj, float deltaTime)
{
    if (obj.targetIndex >= obj.p.size())
        return; // достиг цели

    // текущая точка
    glm::vec3 startPos = obj.position;
    // следующая точка маршрута
    int tt = obj.p[obj.targetIndex];
    NodeP targetNode = nodesS->nodes[tt];
    glm::vec3 endPos(targetNode.x, targetNode.height, targetNode.z);

    // Расстояние между точками
    float distance = glm::distance(startPos, endPos);
    // Время, за которое нужно пройти путь
    float travelTime = distance / obj.speed;

    // Обновляем прогресс
    float deltaProgress = deltaTime / travelTime; // доля пути за текущий кадр
    obj.progress += deltaProgress;

    if (obj.progress >= 1.0f)
    {
        // достигли следующей точки
        obj.position = endPos;
        obj.targetIndex++;
        obj.progress = 0.0f; // начинаем движение к следующей точке
    }
    else
    {
        // интерполируем позицию
        obj.position = glm::mix(startPos, endPos, obj.progress);
    }
}