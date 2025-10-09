#ifndef ROUTE_HPP
#define ROUTE_HPP
#include <vector>
#include <cmath>
#include <queue>
#include <glm/glm.hpp>
#include <algorithm>
#include "Terrain.h"
// Структура узла
struct NodeP
{
  int x, z;                   // Координаты в сетке
  float height;               // Высота
  std::vector<int> neighbors; // Индексы соседних узлов

  float costToNeighbor(int neighborIndex, const std::vector<NodeP> &nodes) const;
  
};

// Объект движения
struct MovingObject
{
  std::vector<int> p;
  glm::vec3 position; // текущая позиция
  int targetIndex;    // индекс следующей вершины маршрута
  float progress;     // прогресс между текущей и следующей точкой (0..1)
  float speed;        // скорость движения (единиц в секунду)
};

struct NodesS{
  std::vector<NodeP> nodes;
};
// const int terrainSize = 5; // Размер сетки
//  float heightMap[25] = {
//      0, 1, 2, 1, 0,
//      1, 2, 3, 2, 1,
//      2, 3, 4, 3, 2,
//      1, 2, 3, 2, 1,
//      0, 1, 2, 1, 0
//  };
void createPathsMap(NodesS *nodesS,Heightmap *heightmap);
// Функция для получения стоимости между двумя узлами
float getCost(int aIndex, int bIndex, const std::vector<NodeP> &nodes);

// Алгоритм Дейкстры для поиска кратчайшего пути
std::vector<int> dijkstra(const std::vector<NodeP> &nodes, int startIdx, int goalIdx, float &totalCost);


// Инициализация
void initObject(NodesS *nodesS,MovingObject &obj, std::vector<int> &route, float moveSpeed);

// Обновление позиции объекта в каждом кадре
void updateObject(NodesS *nodesS,MovingObject &obj, float deltaTime);

#endif // ROUTE_HPP