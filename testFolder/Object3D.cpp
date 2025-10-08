#include "Object3D.h"

void createOneBigCube(std::vector<Object3D>& objects) {
  for (int i=0; i<tN; ++i) {
    float x= (rand()%100/10.f)-5.f;
    float y= (rand()%100/10.f)-5.f;
    float z= (rand()%100/10.f)-5.f;
    // кубы
    std::string t="cube";
    objects.push_back(Object3D{
        glm::vec3(x, y, z),
        glm::vec3(x, y, z),
        {((rand() % 100) / 100.0f - 0.5f) * .1f,
         ((rand() % 100) / 100.0f - 0.5f) * .1f,
         ((rand() % 100) / 100.0f - 0.5f) * .1f},
        1.0f,
        glm::vec3(1.f),
        glm::vec3(0.f),
        false,
        true,
        false,
	glm::vec3(0,0,0),
	t + std::to_string(i)
      });
  }

  // Статичные стены, которые не движутся
  Object3D wallLeft;
  wallLeft.position = glm::vec3(-CXX, 0.0f, 0.0f);
  wallLeft.size = glm::vec3(1.f,tCXX,tCXX); // длина по X
  wallLeft.radius =  glm::vec3(0.0f);
  wallLeft.isSphere = false;
  wallLeft.name = "WallLeft";
  wallLeft.stayObj = true; // статичный объект

  Object3D wallRight;
  wallRight.position = glm::vec3(CXX, 0.0f, 0.0f);
  wallRight.size = glm::vec3(1.f,tCXX,tCXX);;
  wallRight.radius = glm::vec3(0.0f);
  wallRight.isSphere = false;
  wallRight.name = "WallRight";
  wallRight.stayObj = true;

  Object3D wallTop;
  wallTop.position = glm::vec3(0.0f, CXX, 0.0f);
  wallTop.size = glm::vec3(tCXX,1.0f,tCXX); // длина по X
  wallTop.radius = glm::vec3(0.0f);
  wallTop.isSphere = false;
  wallTop.name = "WallTop";
  wallTop.stayObj = true;

  Object3D wallBottom;
  wallBottom.position = glm::vec3(0.0f, -CXX, 0.0f);
  wallBottom.size = glm::vec3(tCXX,1.0f,tCXX); // длина по X
  wallBottom.radius = glm::vec3(0.0f);
  wallBottom.isSphere = false;
  wallBottom.name = "WallBottom";
  wallBottom.stayObj = true;

  Object3D wallFront;
  wallFront.position = glm::vec3(0.0f, 0.0f, CXX);
  wallFront.size = glm::vec3(tCXX,tCXX,1.0f); // длина по X
  wallFront.radius = glm::vec3(0.0f);
  wallFront.isSphere = false;
  wallFront.name = "WallFront";
  wallFront.stayObj = true;

  Object3D wallBack;
  wallBack.position = glm::vec3(0.0f, 0.0f, -CXX);
  wallBack.size = glm::vec3(tCXX,tCXX,1.0f); // длина по X
  wallBack.radius = glm::vec3(0.0f);
  wallBack.isSphere = false;
  wallBack.name = "WallBack";
  wallBack.stayObj = true;

  // Добавляем в список объектов
  objects.push_back(wallLeft);
  objects.push_back(wallRight);
  objects.push_back(wallTop);
  objects.push_back(wallBottom);
  objects.push_back(wallFront);
  objects.push_back(wallBack);
}

void createOneBigCubeCoords(std::vector<Object3D>& objects,glm::vec3 &v) {
  for (int i=0; i<tN; ++i) {
    float x= (rand()%100/10.f)-5.f;
    float y= (rand()%100/10.f)-5.f;
    float z= (rand()%100/10.f)-5.f;
    // кубы
    std::string t="cube";
    objects.push_back(Object3D{
        glm::vec3(404-256, y + v.y, 40-256),
        glm::vec3(x, y, z),
        {((rand() % 100) / 100.0f - 0.5f) * .51f,
         ((rand() % 100) / 100.0f - 0.5f) * .51f,
         ((rand() % 100) / 100.0f - 0.5f) * .51f},
        1.0f,
        glm::vec3(1.f),
        glm::vec3(0.f),
        false,
        true,
        false,
	glm::vec3(0,0,0),
	t + std::to_string(i)
      });
  }

  // Статичные стены, которые не движутся
  Object3D wallLeft;
  wallLeft.position = glm::vec3(-CXX+v.x, v.y, v.z);
  wallLeft.size = glm::vec3(1.f,tCXX,tCXX); // длина по X
  wallLeft.radius =  glm::vec3(0.0f);
  wallLeft.isSphere = false;
  wallLeft.name = "WallLeft";
  wallLeft.NormalWall = glm::vec3(0,0,0);
  wallLeft.stayObj = true; // статичный объект

  Object3D wallRight;
  wallRight.position = glm::vec3(CXX+v.x, v.y, v.z);
  wallRight.size = glm::vec3(1.f,tCXX,tCXX);;
  wallRight.radius = glm::vec3(0.0f);
  wallRight.isSphere = false;
  wallRight.name = "WallRight";
  wallRight.NormalWall = glm::vec3(0,0,0);
  wallRight.stayObj = true;

  Object3D wallTop;
  wallTop.position = glm::vec3(v.x, CXX+v.y, v.z);
  wallTop.size = glm::vec3(tCXX,1.0f,tCXX); // длина по X
  wallTop.radius = glm::vec3(0.0f);
  wallTop.isSphere = false;
  wallTop.name = "WallTop";
  wallTop.NormalWall = glm::vec3(0,0,0);
  wallTop.stayObj = true;

  Object3D wallBottom;
  wallBottom.position = glm::vec3(v.x, -CXX+v.y, v.z);
  wallBottom.size = glm::vec3(tCXX,1.0f,tCXX); // длина по X
  wallBottom.radius = glm::vec3(0.0f);
  wallBottom.isSphere = false;
  wallBottom.name = "WallBottom";
  wallBottom.NormalWall = glm::vec3(0,0,0);
  wallBottom.stayObj = true;

  Object3D wallFront;
  wallFront.position = glm::vec3(v.x, v.y, CXX+v.z);
  wallFront.size = glm::vec3(tCXX,tCXX,1.0f); // длина по X
  wallFront.radius = glm::vec3(0.0f);
  wallFront.isSphere = false;
  wallFront.name = "WallFront";
  wallFront.NormalWall = glm::vec3(0,0,0);
  wallFront.stayObj = true;

  Object3D wallBack;
  wallBack.position = glm::vec3(v.x, v.y, -CXX+v.z);
  wallBack.size = glm::vec3(tCXX,tCXX,1.0f); // длина по X
  wallBack.radius = glm::vec3(0.0f);
  wallBack.isSphere = false;
  wallBack.name = "WallBack";
  wallBack.NormalWall = glm::vec3(0.0f,0.0f,0.0f);
  wallBack.stayObj = true;

  // Добавляем в список объектов
  objects.push_back(wallLeft);
  objects.push_back(wallRight);
  objects.push_back(wallTop);
  objects.push_back(wallBottom);
  objects.push_back(wallFront);
  objects.push_back(wallBack);
}

