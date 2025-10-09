#include "Terrain.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
float getHeightAt(Heightmap *heightmap, float x, float z)
 {
  // Преобразуем мировые координаты в локальные индексы
  float gridSize = terrainSize - 1;
  float fx = (x + terrainSize / 2.0f);
  float fz = (z + terrainSize / 2.0f);

  float u = fx / terrainSize * gridSize;
  float v = fz / terrainSize * gridSize;

  int x0 = static_cast<int>(floor(u));
  int x1 = x0 + 1;
  int z0 = static_cast<int>(floor(v));
  int z1 = z0 + 1;

  // Ограничиваем индексы
  x0 = std::clamp(x0, 0, terrainSize - 1);
  x1 = std::clamp(x1, 0, terrainSize - 1);
  z0 = std::clamp(z0, 0, terrainSize - 1);
  z1 = std::clamp(z1, 0, terrainSize - 1);

  // Получаем высоты в углах квадрата

  float h00 = heightmap->heightmap[z0 * terrainSize + x0];
  float h10 = heightmap->heightmap[z0 * terrainSize + x1];
  float h01 = heightmap->heightmap[z1 * terrainSize + x0];
  float h11 = heightmap->heightmap[z1 * terrainSize + x1];

  // Билинейная интерполяция
  float tx = u - x0;
  float tz = v - z0;

  float h0 = h00 * (1 - tx) + h10 * tx;
  float h1 = h01 * (1 - tx) + h11 * tx;

  float height = h0 * (1 - tz) + h1 * tz;

  return height;
}

void setupTerrainBuffers(TerrainVAO *terrainVAO, TerrainVertices *terrainVertices)
 {
  glGenVertexArrays(1, &terrainVAO->terrainVAO);
  glGenBuffers(1, &terrainVAO->terrainVBO);
  glGenBuffers(1, &terrainVAO->terrainEBO);

  glBindVertexArray(terrainVAO->terrainVAO);

  glBindBuffer(GL_ARRAY_BUFFER, terrainVAO->terrainVBO);
  glBufferData(GL_ARRAY_BUFFER, terrainVertices->terrainVertices.size() * sizeof(Vertex), terrainVertices->terrainVertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainVAO->terrainEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, terrainVertices->terrainIndices.size() * sizeof(unsigned int), terrainVertices->terrainIndices.data(), GL_STATIC_DRAW);

  // Позиции
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
  glEnableVertexAttribArray(0);
  // Нормали
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normal)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
}

void createTerrainMesh(Heightmap *heightmap, TerrainVertices *terrainVertices)
 {
  terrainVertices->terrainVertices.clear();
  terrainVertices->terrainIndices.clear();

  size_t vertCount = terrainSize * terrainSize;
  terrainVertices->terrainVertices.reserve(vertCount);
  terrainVertices->terrainIndices.reserve((terrainSize - 1) * (terrainSize - 1) * 6);

  // Создаем вершины по карте высот
  for (int z = 0; z < terrainSize; ++z) {
    for (int x = 0; x < terrainSize; ++x) {
      float h = heightmap->heightmap[z * terrainSize + x]; // Используем вашу карту высот
      terrainVertices->terrainVertices.push_back({ glm::vec3(x - terrainSize / 2.0f, h, z - terrainSize / 2.0f), glm::vec3(0.0f) });
    }
  }

  // Создаем индексы треугольников (без изменений)
  for (int z = 0; z < terrainSize - 1; ++z) {
    for (int x = 0; x < terrainSize - 1; ++x) {
      int topLeft = z * terrainSize + x;
      int topRight = topLeft + 1;
      int bottomLeft = (z + 1) * terrainSize + x;
      int bottomRight = bottomLeft + 1;

      terrainVertices->terrainIndices.push_back(topLeft);
      terrainVertices->terrainIndices.push_back(bottomLeft);
      terrainVertices->terrainIndices.push_back(topRight);

      terrainVertices->terrainIndices.push_back(topRight);
      terrainVertices->terrainIndices.push_back(bottomLeft);
      terrainVertices->terrainIndices.push_back(bottomRight);
    }
  }

  // Вычисляем нормали (без изменений)
  for (auto& v : terrainVertices->terrainVertices) {
    v.normal = glm::vec3(0.0f);
  }
  for (size_t i = 0; i < terrainVertices->terrainIndices.size(); i += 3) {
    unsigned int ia = terrainVertices->terrainIndices[i];
    unsigned int ib = terrainVertices->terrainIndices[i + 1];
    unsigned int ic = terrainVertices->terrainIndices[i + 2];

    glm::vec3 v0 = terrainVertices->terrainVertices[ia].position;
    glm::vec3 v1 = terrainVertices->terrainVertices[ib].position;
    glm::vec3 v2 = terrainVertices->terrainVertices[ic].position;

    glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

    terrainVertices->terrainVertices[ia].normal += normal;
    terrainVertices->terrainVertices[ib].normal += normal;
    terrainVertices->terrainVertices[ic].normal += normal;
  }
  for (auto& v : terrainVertices->terrainVertices) {
    v.normal = glm::normalize(v.normal);
  }
}

void createFromImage(Heightmap *heightmap, TerrainVAO *terrainVAO, TerrainVertices *terrainVertices)
{
  int width = terrainSize; // ширина карты
  int height = terrainSize; // высота карты
  int channels; 
  unsigned char* data = stbi_load("asset/textures/heightmap3.png", &width, &height, &channels, 1); // 1 — для grayscale

  if (data == nullptr) {
    // Обработка ошибки
    std::cerr << "Failed to load image" << std::endl;
  } else {
    // data содержит изображение в виде массива байтов (грейскейл)
    // width и height — размеры изображения
    // channels — число каналов (будет 1, так как мы указали 1)
  
    for (int i = 0; i < width * height; ++i) {
      heightmap->heightmap[i] = (data[i] / 255.0f)*20.0f; // преобразование к диапазону 0-1
    }
  }
  stbi_image_free(data);
  createTerrainMesh(heightmap,terrainVertices);
  setupTerrainBuffers(terrainVAO,terrainVertices);
}

void SaveToImage(Heightmap *heightmap)
{
  // Создаем массив байтов для хранения изображения
  unsigned char* imageData = new unsigned char[terrainSize * terrainSize];

  for (int i = 0; i < terrainSize * terrainSize; ++i) {
      float value = heightmap->heightmap[i];
      // Ограничим значение в диапазоне 0-1
      if (value < 0.0f) value = 0.0f;
      if (value > 1.0f) value = 1.0f;

      imageData[i] = static_cast<unsigned char>(value * 255.0f);
  }
  // Сохраняем как PNG (grayscale)
  stbi_write_png("heightmap10.png", terrainSize, terrainSize, 1, imageData, terrainSize);

  delete[] imageData;
}