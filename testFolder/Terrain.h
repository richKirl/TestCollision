#ifndef TERRAIN_HPP
#define TERRAIN_HPP
#include <glm/glm.hpp>
#include <vector>
#include <GL/glew.h>
#include <algorithm>
#include <iostream>


#define terrainSize 512
using HeightMap1 = std::vector<float>;
// OpenGL buffers
struct TerrainVAO{
    GLuint terrainVAO, terrainVBO, terrainEBO;
};

// Вершина
struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
};

struct TerrainVertices{
    // Создать меш и нормали
    std::vector<Vertex> terrainVertices;
    std::vector<unsigned int> terrainIndices;
};

struct Heightmap{
    std::vector<float> heightmap=std::vector<float>(terrainSize * terrainSize,0.0);
};


float getHeightAt(Heightmap *heightmap,float x, float z);


void setupTerrainBuffers(TerrainVAO *terrainVAO,TerrainVertices *terrainVertices);
// Обновленная функция createTerrainMesh, использующая heightMap
void createTerrainMesh(Heightmap *heightmap,TerrainVertices *terrainVertices);
void createFromImage(Heightmap *heightmap,TerrainVAO *terrainVAO,TerrainVertices *terrainVertices);

void SaveToImage(Heightmap *heightmap);


#endif // TERRAIN_HPP