#ifndef OBJECT3D_HPP
#define OBJECT3D_HPP
#include <string>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#define CXX 256 // step
#define tCXX 2 * CXX
#define tN 2
struct Object3D
{
  glm::vec3 position;
  glm::vec3 prevpos;
  glm::vec3 velocity;
  float spped;
  glm::vec3 size;   // cube
  glm::vec3 radius; // sphere
  bool isSphere;
  bool collided = false; // visualisation
  bool stayObj = false;
  glm::vec3 NormalWall; //(1, 0, 0)
  std::string name;
};

void createOneBigCube(std::vector<Object3D> &objects);

void createOneBigCubeCoords(std::vector<Object3D> &objects, glm::vec3 &v);

#endif // OBJECT3D_HPP