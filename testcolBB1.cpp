#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/noise.hpp> // для функций шума
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <string>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <random>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <ft2build.h>
#include FT_FREETYPE_H

FT_Library ft;
FT_Face face;

class Shader
{
public:
  unsigned int ID;
  // constructor generates the shader on the fly
  // ------------------------------------------------------------------------
  Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr)
  {
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    std::ifstream gShaderFile;
    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    gShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    try 
      {
	// open files
	vShaderFile.open(vertexPath);
	fShaderFile.open(fragmentPath);
	std::stringstream vShaderStream, fShaderStream;
	// read file's buffer contents into streams
	vShaderStream << vShaderFile.rdbuf();
	fShaderStream << fShaderFile.rdbuf();		
	// close file handlers
	vShaderFile.close();
	fShaderFile.close();
	// convert stream into string
	vertexCode = vShaderStream.str();
	fragmentCode = fShaderStream.str();			
	// if geometry shader path is present, also load a geometry shader
	if(geometryPath != nullptr)
	  {
	    gShaderFile.open(geometryPath);
	    std::stringstream gShaderStream;
	    gShaderStream << gShaderFile.rdbuf();
	    gShaderFile.close();
	    geometryCode = gShaderStream.str();
	  }
      }
    catch (std::ifstream::failure& e)
      {
	std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
      }
    const char* vShaderCode = vertexCode.c_str();
    const char * fShaderCode = fragmentCode.c_str();
    // 2. compile shaders
    unsigned int vertex, fragment;
    // vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");
    // if geometry shader is given, compile geometry shader
    unsigned int geometry;
    if(geometryPath != nullptr)
      {
	const char * gShaderCode = geometryCode.c_str();
	geometry = glCreateShader(GL_GEOMETRY_SHADER);
	glShaderSource(geometry, 1, &gShaderCode, NULL);
	glCompileShader(geometry);
	checkCompileErrors(geometry, "GEOMETRY");
      }
    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    if(geometryPath != nullptr)
      glAttachShader(ID, geometry);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if(geometryPath != nullptr)
      glDeleteShader(geometry);

  }
  // activate the shader
  // ------------------------------------------------------------------------
  void use() 
  { 
    glUseProgram(ID); 
  }
  // utility uniform functions
  // ------------------------------------------------------------------------
  void setBool(const std::string &name, bool value) const
  {         
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
  }
  // ------------------------------------------------------------------------
  void setInt(const std::string &name, int value) const
  { 
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
  }
  // ------------------------------------------------------------------------
  void setFloat(const std::string &name, float value) const
  { 
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
  }
  // ------------------------------------------------------------------------
  void setVec2(const std::string &name, const glm::vec2 &value) const
  { 
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
  }
  void setVec2(const std::string &name, float x, float y) const
  { 
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y); 
  }
  // ------------------------------------------------------------------------
  void setVec3(const std::string &name, const glm::vec3 &value) const
  { 
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
  }
  void setVec3(const std::string &name, float x, float y, float z) const
  { 
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z); 
  }
  // ------------------------------------------------------------------------
  void setVec4(const std::string &name, const glm::vec4 &value) const
  { 
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
  }
  void setVec4(const std::string &name, float x, float y, float z, float w) 
  { 
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w); 
  }
  // ------------------------------------------------------------------------
  void setMat2(const std::string &name, const glm::mat2 &mat) const
  {
    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
  }
  // ------------------------------------------------------------------------
  void setMat3(const std::string &name, const glm::mat3 &mat) const
  {
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
  }
  // ------------------------------------------------------------------------
  void setMat4(const std::string &name, const glm::mat4 &mat) const
  {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
  }

private:
  // utility function for checking shader compilation/linking errors.
  // ------------------------------------------------------------------------
  void checkCompileErrors(GLuint shader, std::string type)
  {
    GLint success;
    GLchar infoLog[1024];
    if(type != "PROGRAM")
      {
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if(!success)
	  {
	    glGetShaderInfoLog(shader, 1024, NULL, infoLog);
	    std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
	  }
      }
    else
      {
	glGetProgramiv(shader, GL_LINK_STATUS, &success);
	if(!success)
	  {
	    glGetProgramInfoLog(shader, 1024, NULL, infoLog);
	    std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
	  }
      }
  }
};




float dvertices[] = {
  // позиции        // текстурные координаты
  //  x, y, z       u, v
  0.5f,  0.5f, 0.0f, 1.0f, 1.0f, // верхний правый
  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // нижний правый
  -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // нижний левый
  -0.5f,  0.5f, 0.0f, 0.0f, 1.0f  // верхний левый
};

int dindices[] = {0, 1, 3, 1, 2, 3};



// --- Структуры объектов и BVH ---
#define CXX 256 // step
#define tCXX 2*CXX
#define tN 1000
int wi=800;
int he = 600;
const int SIZE = 512;
using HeightMap = std::vector<float>;
// Высотная карта
const int terrainSize = 512;

HeightMap heightMap(SIZE * SIZE, 0.0);

bool teleportationSTATUS=false;

struct Object3D {
  glm::vec3 position;
  glm::vec3 prevpos;
  glm::vec3 velocity;
  float spped;
  glm::vec3 size;     // для кубика
  glm::vec3 radius;   // для сферы
  bool isSphere;
  bool collided = false; // для визуализации столкновений
  bool stayObj = false;
  glm::vec3 NormalWall;//(1, 0, 0)
  std::string name;
};




struct AABB {
  glm::vec3 min, max;
  AABB() : min(glm::vec3(0)), max(glm::vec3(0)) {}
  AABB(const glm::vec3& min_, const glm::vec3& max_) : min(min_), max(max_) {}
  void expand(const Object3D& obj) {
    glm::vec3 minO, maxO;
    if (obj.isSphere) {
      minO = obj.position - glm::vec3(obj.radius);
      maxO = obj.position + glm::vec3(obj.radius);
    } else {
      minO = obj.position - glm::vec3(obj.size.x * 0.5f,obj.size.y * 0.5f,obj.size.z * 0.5f);
      maxO = obj.position + glm::vec3(obj.size.x * 0.5f,obj.size.y * 0.5f,obj.size.z * 0.5f);
    }
    min = glm::min(min, minO);
    max = glm::max(max, maxO);
  }
  bool intersects(const AABB& other) const {
    return (max.x >= other.min.x && min.x <= other.max.x) &&
      (max.y >= other.min.y && min.y <= other.max.y) &&
      (max.z >= other.min.z && min.z <= other.max.z);
  }
};

AABB getExpandedAABB(const Object3D& obj, float dt) {
  glm::vec3 startPos = obj.prevpos;
  glm::vec3 endPos = obj.position;
  glm::vec3 minPos = glm::min(startPos, endPos);
  glm::vec3 maxPos = glm::max(startPos, endPos);
  glm::vec3 size = (maxPos - minPos);
  // Добавьте радиусы или размеры
  if (obj.isSphere) {
    minPos -= glm::vec3(obj.radius);
    maxPos += glm::vec3(obj.radius);
  } else {
    minPos -= glm::vec3(obj.size * 0.5f);
    maxPos += glm::vec3(obj.size * 0.5f);
  }
  return AABB(minPos, maxPos);
}

struct BVHNode {
  AABB box;
  BVHNode* left = nullptr;
  BVHNode* right = nullptr;
  std::vector<Object3D*> objects; // листовой узел
  bool isLeaf() const { return !objects.empty(); }
  ~BVHNode() {
    delete left;
    delete right;
  }
};


glm::mat4 view;
glm::mat4 projection;
// И также глобальные переменные для хранения результата
Object3D* selectedObject = nullptr;
float rayDistance = FLT_MAX;
BVHNode* bvhRoot;
std::vector<Object3D> objects;








// float Cvertices[] = {
//     // позиция           нормаль
//     -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f, //0
//      0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f, //1
//      0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f, //2
//     -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f, //3
//     -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  1.0f, //4
//      0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  1.0f, //5
//      0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  1.0f, //6
//     -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  1.0f  //7
// };

// unsigned int Cindices[] = {
//     // Передняя грань
//     4, 5, 6,
//     6, 7, 4,
//     // Задняя грань
//     0, 1, 2,
//     2, 3, 0,
//     // Левая грань
//     0, 4, 7,
//     7, 3, 0,
//     // Правая грань
//     1, 5, 6,
//     6, 2, 1,
//     // Верхняя грань
//     3, 2, 6,
//     6, 7, 3,
//     // Нижняя грань
//     0, 1, 5,
//     5, 4, 0
// };


// Создаем меш куба
float cubeVertices[] = {
  // positions
  -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,//0.0f, 0.0f,
  0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,//1.0f, 0.0f,
  0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,//1.0f, 1.0f,
  0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,//1.0f, 1.0f,
  -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,//0.0f, 1.0f,
  -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,//0.0f, 0.0f,

  -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,//0.0f, 0.0f,
  0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,//1.0f, 0.0f,
  0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,//1.0f, 1.0f,
  0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,//1.0f, 1.0f,
  -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,//0.0f, 1.0f,
  -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,//0.0f, 0.0f,

  -0.5f,  0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,//1.0f, 0.0f,
  -0.5f,  0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,//1.0f, 1.0f,
  -0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,//0.0f, 1.0f,
  -0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,//0.0f, 1.0f,
  -0.5f, -0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,//0.0f, 0.0f,
  -0.5f,  0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,//1.0f, 0.0f,

  0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,//1.0f, 0.0f,
  0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,//1.0f, 1.0f,
  0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,//0.0f, 1.0f,
  0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,//0.0f, 1.0f,
  0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,//0.0f, 0.0f,
  0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,//1.0f, 0.0f,

  -0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,//0.0f, 1.0f,
  0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,//1.0f, 1.0f,
  0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f,//1.0f, 0.0f,
  0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f,//1.0f, 0.0f,
  -0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f,//0.0f, 0.0f,
  -0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,//0.0f, 1.0f,

  -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,//0.0f, 1.0f,
  0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,//1.0f, 1.0f,
  0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,//1.0f, 0.0f,
  0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,//1.0f, 0.0f,
  -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,//0.0f, 0.0f,
  -0.5f,  0.5f, -0.5f,0.0f, 1.0f, 0.0f,//  0.0f, 1.0f
};

// --- main() ---
// camera
glm::vec3 cameraPos   = glm::vec3(50.0f-256.0f, -220.0f,440.0f-256.0f );
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);
bool firstMouse = true;
bool mousetoogle=true;
float yaw   = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch =  0.0f;
float lastX =  wi/ 2.0;
float lastY =  he/ 2.0;
float fov   =  60.0f;
// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;


glm::vec3 lightPos = glm::vec3(0.0f, 512.0f, 0.0f);
float angle = 0.0f; // угол для движения по окружности



// OpenGL buffers
GLuint terrainVAO, terrainVBO, terrainEBO;

// Вершина
struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
};

// Создать меш и нормали
std::vector<Vertex> terrainVertices;
std::vector<unsigned int> terrainIndices;

    
glm::mat4 lightSpaceMatrix;
glm::mat4 lightProjection, lightView;

glm::mat4 termodel = glm::translate(glm::mat4(1.f), glm::vec3(0.0f,-240.0f,0.0f));

// Создание луча
struct Ray {
  glm::vec3 origin;
  glm::vec3 direction;
  glm::vec3 invDirection;
};

Ray CreateRay(const glm::vec3& p1, const glm::vec3& p2)
{
  glm::vec3 dir = p2 - p1;
  glm::vec3 invDir;
  invDir.x = (fabs(dir.x) > 1e-8) ? 1.0f / dir.x : FLT_MAX;
  invDir.y = (fabs(dir.y) > 1e-8) ? 1.0f / dir.y : FLT_MAX;
  invDir.z = (fabs(dir.z) > 1e-8) ? 1.0f / dir.z : FLT_MAX;
  return {p1, dir, invDir};
}


// Проверка пересечения луча с AABB
bool RayCast(const AABB& box, const Ray& ray)
{
  float tmin = -FLT_MAX;
  float tmax = FLT_MAX;

  for (int i = 0; i < 3; ++i)
    {
      float invD = (i == 0) ? ray.invDirection.x : (i == 1) ? ray.invDirection.y : ray.invDirection.z;
      float originComponent = (i == 0) ? ray.origin.x : (i == 1) ? ray.origin.y : ray.origin.z;
      float minB = (i == 0) ? box.min.x : (i == 1) ? box.min.y : box.min.z;
      float maxB = (i == 0) ? box.max.x : (i == 1) ? box.max.y : box.max.z;

      if (invD == FLT_MAX) // параллельно оси
        {
	  if (originComponent < minB || originComponent > maxB)
	    return false;
        }
      else
        {
	  float t0 = (minB - originComponent) * invD;
	  float t1 = (maxB - originComponent) * invD;
	  if (invD < 0.0f) std::swap(t0, t1);
	  tmin = std::max(tmin, t0);
	  tmax = std::min(tmax, t1);
	  if (tmax < tmin)
	    return false;
        }
    }
  return true;
}



BVHNode* buildBVH(std::vector<Object3D*>& objs, int depth=0) {
  BVHNode* node = new BVHNode();
  // Создаем изначальный AABB
  node->box = AABB(glm::vec3(FLT_MAX), glm::vec3(-FLT_MAX));
  for (auto* o : objs) node->box.expand(*o);
  if (objs.size() <= 2) {
    node->objects = objs;
    return node;
  }
  int axis = depth % 3;
  auto compareAxis = [axis](Object3D* a, Object3D* b) {
    float aCoord = (axis==0) ? a->position.x : (axis==1) ? a->position.y : a->position.z;
    float bCoord = (axis==0) ? b->position.x : (axis==1) ? b->position.y : b->position.z;
    return aCoord < bCoord;
  };
  std::sort(objs.begin(), objs.end(), compareAxis);
  size_t mid = objs.size()/2;
  std::vector<Object3D *> leftObjs(objs.begin(), objs.begin()+mid);
  std::vector<Object3D *> rightObjs(objs.begin() + mid, objs.end());
  //std::cout<<depth<<std::endl;
  node->left = buildBVH(leftObjs, depth+1);
  node->right = buildBVH(rightObjs, depth+1);
  return node;
}

bool checkCollision(const Object3D& a, const Object3D& b) {
  if (a.isSphere && b.isSphere) {
    // Проверка сфер
    float distSq = glm::dot(a.position - b.position, a.position - b.position);
    float radiusSum = a.radius.x + b.radius.x; // предполагаю радиусы одинаковые по x,y,z
    return distSq <= radiusSum * radiusSum;
  } else {
    // Проверка кубов (или смешанных)
    // Создаем AABB для каждого и проверяем пересечение
    AABB aabbA;
    if (a.isSphere) {
      aabbA.min = a.position - glm::vec3(a.radius);
      aabbA.max = a.position + glm::vec3(a.radius);
    } else {
      aabbA.min = a.position - glm::vec3(a.size.x * 0.5f, a.size.y * 0.5f, a.size.z * 0.5f);
      aabbA.max = a.position + glm::vec3(a.size.x * 0.5f, a.size.y * 0.5f, a.size.z * 0.5f);
    }

    AABB aabbB;
    if (b.isSphere) {
      aabbB.min = b.position - glm::vec3(b.radius);
      aabbB.max = b.position + glm::vec3(b.radius);
    } else {
      aabbB.min = b.position - glm::vec3(b.size.x * 0.5f, b.size.y * 0.5f, b.size.z * 0.5f);
      aabbB.max = b.position + glm::vec3(b.size.x * 0.5f, b.size.y * 0.5f, b.size.z * 0.5f);
    }

    return aabbA.intersects(aabbB);
  }
}


void resolveCollision(Object3D& a, Object3D& b) {
  // Предположим, что масса обратно пропорциональна объему

  float massA = a.stayObj ? 1e9f : glm::length(a.isSphere ? a.radius : a.size);
  float massB = b.stayObj ? 1e9f : glm::length(b.isSphere ? b.radius : b.size);

  
  // Вектор столкновения
  glm::vec3 collisionNormal = glm::normalize(b.position - a.position);
  float relativeVelocity = glm::dot(b.velocity - a.velocity, collisionNormal);

  if (relativeVelocity > 0) return; // объекты удаляются друг от друга

  // Расчет импульса
  float restitution = 0.8f; // коэффициент восстановления, 1 — идеально упругий
  float impulseMag = -(1 + restitution) * relativeVelocity / (1/massA + 1/massB);

  glm::vec3 impulse = impulseMag * collisionNormal;

  if (!a.stayObj) a.velocity -= (1/massA) * impulse;
  if (!b.stayObj) b.velocity += (1/massB) * impulse;
}



void reflectVelocity(Object3D& obj, const glm::vec3& normal) {
  obj.velocity = obj.velocity - 2.0f*glm::dot(obj.velocity, normal)*normal;
}



void traverseBVH(BVHNode *node, Object3D *obj) {
  //   AABB aabbExpanded = getExpandedAABB(*obj, 0.16f);
  // if (!node || !node->box.intersects(aabbExpanded)) return;
  if (!node || !node->box.intersects(AABB(obj->position - (obj->isSphere ? obj->radius : obj->size),
					  obj->position + (obj->isSphere ? obj->radius : obj->size)))) return;
  if (node->isLeaf()) {
    for (auto* other : node->objects) {
      if (other != obj && checkCollision(*obj, *other)) {
        if (other->stayObj) {
          obj->velocity *= -1;
	  reflectVelocity(*obj, other->NormalWall);
	  obj->collided = true;
        }
	else{
	  resolveCollision(*obj, *other);
	  obj->collided = true;
	  other->collided = true;
	}
      }
    }
  } else {
    traverseBVH(node->left, obj);
    traverseBVH(node->right, obj);
  }
}





// --- Шейдеры ---

// const char* vertexShaderSrc = R"(
// #version 460 core
// layout(location=0) in vec3 aPos;
// uniform mat4 model;
// uniform mat4 view;
// uniform mat4 projection;
// void main() {
//     gl_Position = projection * view * model * vec4(aPos,1.0);
// }
// )";

// const char* fragmentShaderSrc = R"(
// #version 460 core
// out vec4 FragColor;
// uniform vec3 color;
// void main() {
//     FragColor = vec4(color,1.0);
// }
// )";




const char *vertexShaderSrc = R"(
#version 460 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

out vec4 FragPosLightSpace;
out vec3 FragPos;
out vec3 Normal;
out vec3 fragPosition;     // позиция фрагмента
out vec3 sceneColor;       // исходный цвет сцены
void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    fragPosition = FragPos;
    Normal = mat3(transpose(inverse(model))) * aNormal;
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
    sceneColor=vec3(0.5,0.7,0.15);
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

const char *fragmentShaderSrc = R"(
#version 460 core
out vec4 FragColor;

in vec3 fragPosition;     // позиция фрагмента
in vec3 sceneColor;       // исходный цвет сцены

uniform vec3 fogColor;
uniform float fogDensity;
uniform vec3 cameraPos;
in vec4 FragPosLightSpace;
in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 objectColor;
uniform sampler2D shadowMap;

// Расчет тени
float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; // преобразование в диапазон 0..1

    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    float bias = 0.001;
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    // Можно добавить мягкое затенение (PCF) для более гладких теней
    if(projCoords.z > 1.0)
        shadow = 0.0;
    return shadow;
}


void main() {
    // // Амбиентное
    // vec3 ambient = 0.3 * objectColor;
    // // Диффузное
    // vec3 norm = normalize(Normal);
    // vec3 lightDir = normalize(lightPos - FragPos);
    // float diff = max(dot(norm, lightDir), 0.0);

    // float shadow = ShadowCalculation(FragPosLightSpace);

    // vec3 diffuse = (1.0 - shadow) * diff * objectColor * 2.0;
    // // Блик
    // vec3 viewDir = normalize(viewPos - FragPos);
    // vec3 reflectDir = reflect(-lightDir, norm);
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    // vec3 specular = vec3(0.5) * spec;

    // vec3 result = ambient + diffuse + specular;

    // Основной расчет освещения (можете оставить ваш код)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 ambient = 0.1 * objectColor;
    //float shadow = ShadowCalculation(FragPosLightSpace);
    vec3 diffuse = diff * objectColor;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    vec3 specular = vec3(0.2) * spec;
    //vec3 result = ambient + diffuse + specular;
//float shadow = ShadowCalculation(FragPosLightSpace);
vec3 result = (ambient +  diffuse + specular) * objectColor;
    // Расчет расстояния до камеры
    float distance = length(fragPosition - cameraPos);

    // Объемный туман с градиентом
    float fogFactor = 1.0 - exp(-pow(distance * fogDensity, 20.0));
    float fogFactor1 = 0.5 - exp(-pow(distance * fogDensity, 20.0));
    fogFactor = clamp(fogFactor,fogFactor1, 1.0);

    // Мягкое смешивание с прозрачностью
    float alpha = 1.0; // Можно сделать плавным переходом или добавить альфу
    vec3 color = mix(result, fogColor, fogFactor);

    // Можно добавить альфу для прозрачности
    FragColor = vec4(result, 1.0);
    //FragColor = vec4(color, alpha);
}

)";

const char *depth_vertex_shadersrc = R"(
#version 460 core
layout(location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 lightSpaceMatrix;

void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}
)";
const char *depth_fragment_shader = R"(
#version 460 core
void main()
{
    // ничего не нужно, depth берется автоматически

}
)";

//descriptorCentr
// vertex shader
const char *vDescriptorCentrSrc = R"(
#version 460 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;

uniform mat4 uProjection;
uniform mat4 uModel;

out vec2 TexCoord;

void main() {
    gl_Position = uProjection * uModel * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}
)";

// fragment shader
const char* fDescriptorCentrSrc = R"(
#version 460 core
in vec2 TexCoord;
layout(location = 0) out vec4 FragColor;

uniform sampler2D uTexture;

void main() {
        vec4 texel = texture(uTexture,TexCoord);
	if(texel.a == 0.0)
		discard;
	FragColor= texel*vec4(1.0,1.0,1.0,1.0);
}
)";



//HDR
// vertex shader
const char *vHDRSrc = R"(
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 1.0);
}
)";

// fragment shader
const char* fHDRSrc = R"(
#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hdrBuffer;
uniform float exposure; // параметр экспозиции

void main()
{
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
    // Тонмэппинг (например, Reinhard)
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    // Увеличение яркости
    mapped = vec3(1.0) - exp(-mapped * exposure);
    FragColor = vec4(mapped, 1.0);
}
)";

// vertex shader
const char* vTSrc = R"(
#version 460 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
)";

// fragment shader

const char *fTSrc = R"(
#version 460 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    color = vec4(textColor, 1.0) * sampled;
}
)";





// Компиляция шейдера
GLuint compileShader(GLenum type, const char* src) {
  GLuint shader=glCreateShader(type);
  glShaderSource(shader,1,&src,nullptr);
  glCompileShader(shader);
  int success;
  glGetShaderiv(shader,GL_COMPILE_STATUS,&success);
  if(!success){
    char infoLog[512];
    glGetShaderInfoLog(shader,512,nullptr,infoLog);
    std::cerr<<"Shader compile error: "<<infoLog<<std::endl;
  }
  return shader;
}

GLuint createProgram(const char* a,const char* b) {
  GLuint vShader=compileShader(GL_VERTEX_SHADER,a);
  GLuint fShader=compileShader(GL_FRAGMENT_SHADER, b);
  GLuint prog=glCreateProgram();
  glAttachShader(prog,vShader);
  glAttachShader(prog,fShader);
  glLinkProgram(prog);
  int success;
  glGetProgramiv(prog,GL_LINK_STATUS,&success);
  if(!success){
    char infoLog[512];
    glGetProgramInfoLog(prog,512,nullptr,infoLog);
    std::cerr<<"Link error: "<<infoLog<<std::endl;
  }
  glDeleteShader(vShader);
  glDeleteShader(fShader);
  return prog;
}



void createDBuffer(  GLuint &depthMapFBO, GLuint &depthMap,const unsigned int SHADOW_WIDTH = 1024, const unsigned int  SHADOW_HEIGHT = 1024){
  glGenFramebuffers(1, &depthMapFBO);

  glGenTextures(1, &depthMap);
  glBindTexture(GL_TEXTURE_2D, depthMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 4096, 4096, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow *window);


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
        glm::vec3(x + v.x, y + v.y, z + v.z),
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
  wallLeft.NormalWall = glm::vec3(1,0,0);
  wallLeft.stayObj = true; // статичный объект

  Object3D wallRight;
  wallRight.position = glm::vec3(CXX+v.x, v.y, v.z);
  wallRight.size = glm::vec3(1.f,tCXX,tCXX);;
  wallRight.radius = glm::vec3(0.0f);
  wallRight.isSphere = false;
  wallRight.name = "WallRight";
  wallLeft.NormalWall = glm::vec3(-1,0,0);
  wallRight.stayObj = true;

  Object3D wallTop;
  wallTop.position = glm::vec3(v.x, CXX+v.y, v.z);
  wallTop.size = glm::vec3(tCXX,1.0f,tCXX); // длина по X
  wallTop.radius = glm::vec3(0.0f);
  wallTop.isSphere = false;
  wallTop.name = "WallTop";
  wallLeft.NormalWall = glm::vec3(0,-1,0);
  wallTop.stayObj = true;

  Object3D wallBottom;
  wallBottom.position = glm::vec3(v.x, -CXX+v.y, v.z);
  wallBottom.size = glm::vec3(tCXX,1.0f,tCXX); // длина по X
  wallBottom.radius = glm::vec3(0.0f);
  wallBottom.isSphere = false;
  wallBottom.name = "WallBottom";
  wallLeft.NormalWall = glm::vec3(0,1,0);
  wallBottom.stayObj = true;

  Object3D wallFront;
  wallFront.position = glm::vec3(v.x, v.y, CXX+v.z);
  wallFront.size = glm::vec3(tCXX,tCXX,1.0f); // длина по X
  wallFront.radius = glm::vec3(0.0f);
  wallFront.isSphere = false;
  wallFront.name = "WallFront";
  wallLeft.NormalWall = glm::vec3(0,0,-1);
  wallFront.stayObj = true;

  Object3D wallBack;
  wallBack.position = glm::vec3(v.x, v.y, -CXX+v.z);
  wallBack.size = glm::vec3(tCXX,tCXX,1.0f); // длина по X
  wallBack.radius = glm::vec3(0.0f);
  wallBack.isSphere = false;
  wallBack.name = "WallBack";
  wallLeft.NormalWall = glm::vec3(0,0,1);
  wallBack.stayObj = true;

  // Добавляем в список объектов
  objects.push_back(wallLeft);
  objects.push_back(wallRight);
  objects.push_back(wallTop);
  objects.push_back(wallBottom);
  objects.push_back(wallFront);
  objects.push_back(wallBack);
}



void createVAOVBObufs(GLuint &cubeVAO, GLuint &cubeVBO) {
  glGenVertexArrays(1,&cubeVAO);
  glGenBuffers(1,&cubeVBO);
  glBindVertexArray(cubeVAO);
  glBindBuffer(GL_ARRAY_BUFFER,cubeVBO);
  glBufferData(GL_ARRAY_BUFFER,sizeof(cubeVertices),cubeVertices,GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),(void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
  glBindVertexArray(0);
}




AABB getSweptAABB(const Object3D& obj, const glm::vec3& velocity, float deltaTime) {
  glm::vec3 startMin, startMax;
  if (obj.isSphere) {
    startMin = obj.position - glm::vec3(obj.radius);
    startMax = obj.position + glm::vec3(obj.radius);
  } else {
    startMin = obj.position - glm::vec3(obj.size * 0.5f);
    startMax = obj.position + glm::vec3(obj.size * 0.5f);
  }

  glm::vec3 endPos = obj.position + velocity * deltaTime;

  glm::vec3 endMin, endMax;
  if (obj.isSphere) {
    endMin = endPos - glm::vec3(obj.radius);
    endMax = endPos + glm::vec3(obj.radius);
  } else {
    endMin = endPos - glm::vec3(obj.size * 0.5f);
    endMax = endPos + glm::vec3(obj.size * 0.5f);
  }

  glm::vec3 minCoords = glm::min(startMin, endMin);
  glm::vec3 maxCoords = glm::max(startMax, endMax);

  return AABB(minCoords, maxCoords);
}

void checkCollisionsBVH(BVHNode* node, Object3D* obj, float deltaTime) {
  AABB sweptAABB = getSweptAABB(*obj, obj->velocity, deltaTime);
  if (!node || !node->box.intersects(sweptAABB)) return;

  if (node->isLeaf()) {
    for (auto* other : node->objects) {
      if (other != obj) {
	// Проверка пересечения AABB
	AABB otherAABB;
	if (other->isSphere) {
	  otherAABB.min = other->position - glm::vec3(other->radius);
	  otherAABB.max = other->position + glm::vec3(other->radius);
	} else {
	  otherAABB.min = other->position - glm::vec3(other->size * 0.5f);
	  otherAABB.max = other->position + glm::vec3(other->size * 0.5f);
	}

	if (sweptAABB.intersects(otherAABB)) {
	  // Обработка столкновения (например, resolveCollision)
	  if (other->stayObj) {
	    obj->velocity *= -1;
	    reflectVelocity(*obj, other->NormalWall);
	    obj->collided = true;
	  }
	  else{
	    resolveCollision(*obj, *other);
	    obj->collided = true;
	    other->collided = true;
	  }
	}
      }
    }
  } else {
    checkCollisionsBVH(node->left, obj, deltaTime);
    checkCollisionsBVH(node->right, obj, deltaTime);
  }
}

int counterRR=0;
void traverseBVHR(BVHNode *node, Object3D *obj) {
  if(counterRR >= 100){counterRR=0;}
  if(counterRR<100){
    AABB aabbExpanded = getExpandedAABB(*obj, 0.16f);
    if (!node || !node->box.intersects(aabbExpanded)) return;
  }
  else {
    if (!node ||
        !node->box.intersects(
			      AABB(obj->position - (obj->isSphere ? obj->radius : obj->size),
				   obj->position + (obj->isSphere ? obj->radius : obj->size))))
      return;
  }
  if (node->isLeaf()) {
    for (auto *other : node->objects) {
      // Создаем AABB объекта
      AABB aabb;
      if (other->isSphere) {
	aabb.min = other->position - glm::vec3(other->radius);
	aabb.max = other->position + glm::vec3(other->radius);
      } else {
	aabb.min = other->position - glm::vec3(other->size * 0.5f);
	aabb.max = other->position + glm::vec3(other->size * 0.5f);
      }
      glm::vec3 startPos = obj->prevpos;
      glm::vec3 endPos = obj->position; // текущая позиция
      Ray ray = CreateRay(startPos, endPos);
      //Ray ray=CreateRay(obj->position, obj->position+(obj->velocity*20000.0f));
      if (other != obj && RayCast(aabb, ray)) {
        if (other->stayObj) {
          obj->velocity *= -1;
	  reflectVelocity(*obj, other->NormalWall);
          obj->collided = true;
	  //break;
        }
	else {
	  resolveCollision(*obj, *other);
	  obj->collided = true;
          other->collided = true;
	  //break;
        }
	
      }
    }
  } else {
    counterRR++;
    if (counterRR < 100) {
      traverseBVH(node->left, obj);
      traverseBVH(node->right, obj);
    }
    else {
      traverseBVHR(node->left, obj);
      traverseBVHR(node->right, obj);
    }
  }

}



float getHeightAt(float x, float z) {
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
  float h00 = heightMap[z0 * terrainSize + x0];
  float h10 = heightMap[z0 * terrainSize + x1];
  float h01 = heightMap[z1 * terrainSize + x0];
  float h11 = heightMap[z1 * terrainSize + x1];

  // Билинейная интерполяция
  float tx = u - x0;
  float tz = v - z0;

  float h0 = h00 * (1 - tx) + h10 * tx;
  float h1 = h01 * (1 - tx) + h11 * tx;

  float height = h0 * (1 - tz) + h1 * tz;

  return height;
}

void adjustObjectToTerrain(Object3D& obj, float offset = 0.1f) {
    float terrainHeight = getHeightAt(obj.position.x, obj.position.z)-239.0f;
    obj.position.y = terrainHeight + offset; // небольшой отступ сверху
    // Можно также сбросить вертикальную скорость, если нужно
    obj.velocity.y = 0.0f;
}

void updateObjects(std::vector<Object3D>& objects, BVHNode* bvhRoot) {
  for (auto& o : objects) {
    o.collided = false;

    if (!o.stayObj) {
          adjustObjectToTerrain(o);
      // o.prevpos = o.position; // запомнить позицию перед обновлением
      // o.position.y = getHeightAt(o.position.x,o.position.z);
      // o.position += o.velocity;
      // traverseBVHR(bvhRoot, &o);
      // checkCollisionsBVH(bvhRoot,&o,0.1f);
      // Можно добавить ограничение по границам сцены
      // например, чтобы объекты не выходили за рамки
            o.prevpos = o.position; // запоминаем старую позицию
            o.position += o.velocity;
	    traverseBVHR(bvhRoot, &o);
	    //checkCollisionsBVH(bvhRoot,&o,0.1f);
            // Теперь корректируем высоту по terrain

    }
  }
}


void RObjects(std::vector<Object3D>& objects,GLuint cubeVAO,GLuint shaderDepthProgram) {
  // Отрисовка всех объектов сцены
  for (auto &o : objects) {
    glm::mat4 model = glm::translate(glm::mat4(1.f), o.position);
    model = glm::scale(model, o.isSphere ? glm::vec3(o.radius.x * 2.0f) : o.size);
    glUniformMatrix4fv(glGetUniformLocation(shaderDepthProgram, "model"), 1, GL_FALSE, &model[0][0]);
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
  }
}



void renderTerrainS(GLuint shaderProgram) {
  // Модель — земля
  glm::mat4 model = termodel;
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
  // glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), 100.0f, 200.0f, 100.0f);
  // glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);
  // glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.4f, 0.8f, 0.4f);
  //glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);
  glBindVertexArray(terrainVAO);
  glDrawElements(GL_TRIANGLES, terrainIndices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}



void renderTerrainM(GLuint shaderProgram) {
  // Модель — земля
  glm::mat4 model = termodel;
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
  // glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), 100.0f, 200.0f, 100.0f);
  // glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);
  glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.4f, 0.8f, 0.4f);
  //glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(terrainVAO);
  glDrawElements(GL_TRIANGLES, terrainIndices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void renderShadow(GLuint shaderDepthProgram, GLuint depthMapFBO, GLuint cubeVAO,
		  std::vector<Object3D> &objects,
                  std::vector<Object3D> &objects1,
                  std::vector<Object3D> &objects2,
                  std::vector<Object3D> &objects3,
                  std::vector<Object3D> &objects4,
                  std::vector<Object3D> &objects5,
                  std::vector<Object3D> &objects6,
                  std::vector<Object3D>& objects7,glm::mat4 &lightSpaceMatrix) {
  // Размер viewport для depth карты
  glViewport(0, 0, 4096, 4096);
  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  glClear(GL_DEPTH_BUFFER_BIT);

  // Используем шейдер для depth
  glUseProgram(shaderDepthProgram);

  // Создаем матрицу света

  //int near_plane = 1.0f, far_plane = 2000.0f;


  // Передача матрицы в шейдер
  glUniformMatrix4fv(glGetUniformLocation(shaderDepthProgram, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);

  //glBindVertexArray(terrainVAO);
  renderTerrainS(shaderDepthProgram);
  //glBindVertexArray(0);
  RObjects(objects,cubeVAO,shaderDepthProgram);
  // RObjects(objects1, cubeVAO, shaderDepthProgram);
  // RObjects(objects2, cubeVAO, shaderDepthProgram);
  // RObjects(objects3, cubeVAO, shaderDepthProgram);
  // RObjects(objects4, cubeVAO, shaderDepthProgram);
  // RObjects(objects5, cubeVAO, shaderDepthProgram);
  // RObjects(objects6, cubeVAO, shaderDepthProgram);
  // RObjects(objects7,cubeVAO,shaderDepthProgram);

  // Окончание рендеринга в depth карту
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



void RObjects(std::vector<Object3D>& objects,GLint locModel,GLint locColor) {
  for (auto &o : objects) {
    if (o.stayObj) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Перед отрисовкой
    glm::vec3 scaleVal = o.isSphere ? glm::vec3(o.radius.x * 2.0f) : o.size;
    glm::mat4 model = glm::translate(glm::mat4(1.f), o.position);
    model = glm::scale(model, scaleVal);
    glUniformMatrix4fv(locModel,1,false,&model[0][0]);
    // Проверка, выбран ли объект
    if (&o == selectedObject) {
      glUniform3f(locColor, 0.0f, 1.0f, 0.0f); // Зеленый
    } else {
      glUniform3f(locColor, o.collided ? 1.f : 0.f, 0.f, o.collided ? 0.f : 1.f); // исходный цвет
    }
    glDrawArrays(GL_TRIANGLES,0,36);
  }
}


void RenderObjsMain(GLuint shaderProgram, GLuint depthMap, GLuint cubeVAO,
                    glm::mat4 &lightSpaceMatrix, glm::mat4 view,
                    glm::mat4 projection,
		    std::vector<Object3D> &objects,
		    std::vector<Object3D> &objects1,
		    std::vector<Object3D> &objects2,
		    std::vector<Object3D> &objects3,
		    std::vector<Object3D> &objects4,
		    std::vector<Object3D> &objects5,
		    std::vector<Object3D> &objects6,
		    std::vector<Object3D>& objects7) {
  // Рендеринг объектов
  //glUseProgram(shaderProgram);
  // Устанавливаем viewport для финального изображения
  glViewport(0, 0, wi, he);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(shaderProgram);

  // Передача матрицы светового пространства
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);

  // Передача depth карты
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, depthMap);
  glUniform1i(glGetUniformLocation(shaderProgram, "shadowMap"), 0);
  // glUniform1i(glGetUniformLocation(shaderProgram, "shadowMap"), 0);
  glUniform3f(glGetUniformLocation(shaderProgram, "fogColor"), 0.5f, 0.5f, 0.7f);
  glUniform1f(glGetUniformLocation(shaderProgram, "fogDensity"), 0.02f);
  glUniform3f(glGetUniformLocation(shaderProgram, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
  GLint locView = glGetUniformLocation(shaderProgram,"view");
  GLint locProj = glGetUniformLocation(shaderProgram,"projection");
  glUniformMatrix4fv(locView,1,false,&view[0][0]);
  glUniformMatrix4fv(locProj,1,false,&projection[0][0]);
  // Установка uniform-параметров освещения
  GLint locLightPos = glGetUniformLocation(shaderProgram, "lightPos");
  GLint locViewPos = glGetUniformLocation(shaderProgram, "viewPos");
  //glUseProgram(shaderProgram);
  glUniform3f(locLightPos, lightPos.x, lightPos.y, lightPos.z);
  glUniform3f(locViewPos, cameraPos.x, cameraPos.y, cameraPos.z);
  GLint locModel = glGetUniformLocation(shaderProgram,"model");
  GLint locColor = glGetUniformLocation(shaderProgram,"objectColor");
  // glActiveTexture(GL_TEXTURE0);
  // glBindTexture(GL_TEXTURE_2D, depthMap);

  //glBindVertexArray(terrainVAO);
  renderTerrainM(shaderProgram);
  //glBindVertexArray(0);
  glBindVertexArray(cubeVAO);
  RObjects(objects,locModel,locColor);
  // RObjects(objects1, cubeVAO, locModel, locColor);
  // RObjects(objects2, cubeVAO, locModel, locColor);
  // RObjects(objects3, cubeVAO, locModel, locColor);
  // RObjects(objects4, cubeVAO, locModel, locColor);
  // RObjects(objects5, cubeVAO, locModel, locColor);
  // RObjects(objects6, cubeVAO, locModel, locColor);
  // RObjects(objects7,cubeVAO,locModel,locColor);

  glBindVertexArray(0);
}


GLuint loadTexture(const char* filename)
{
  int width, height, nrChannels;
  unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 4); //force RGBA
  if (!data)
    {
      std::cerr << "Failed to load texture: " << filename << std::endl;
      return 0;
    }

  GLuint textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  stbi_image_free(data);
  return textureID;
}

void createDescriptor(GLuint &VAO,GLuint &VBO,GLuint &EBO) {
  // Создаем VAO/VBO/EBO
    
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(dvertices), dvertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(dindices), dindices, GL_STATIC_DRAW);

  // позиция
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  // текстурные координаты
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
}

void drawDescriptor(GLuint dshaderProgram,GLuint textureID,GLuint VAO,glm::mat4 view,glm::mat4 projection) {
  // В основном цикле
  // Установите uniform матрицы проекции
  int projLoc = glGetUniformLocation(dshaderProgram, "uProjection");
  glUseProgram(dshaderProgram);
  glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

  // Также задайте модельную матрицу, чтобы разместить квадратик
  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(wi/2, he/2, 0.0f)); // например, по центру
  model = glm::scale(model, glm::vec3(20.0f, 20.0f, 1.0f)); // размер 100x100 пикселей
  model = view*model;
  int modelLoc = glGetUniformLocation(dshaderProgram, "uModel");
  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureID);
  glUniform1i(glGetUniformLocation(dshaderProgram, "uTexture"), 0);

  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}



void pickObject(double mouseX, double mouseY, std::vector<Object3D>& objects, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos) {
  // 1. Нормализованные координаты
  float xNDC = (mouseX / wi) * 2.0f - 1.0f;
  float yNDC = 1.0f - (mouseY / he) * 2.0f;

  // 2. Создайте точку в NDC у Near plane
  glm::vec4 clipCoords = glm::vec4(xNDC, yNDC, -1.0f, 1.0f);
  glm::mat4 invVP = glm::inverse(projection * view);
  glm::vec4 worldCoords = invVP * clipCoords;
  worldCoords /= worldCoords.w;

  // 3. Создать луч
  glm::vec3 rayOrigin = cameraPos;
  //glm::vec3 rayDir = glm::normalize(glm::vec3(worldCoords) - rayOrigin)*200.0f;
  glm::vec3 temp = cameraFront*20000.f;
  Ray ray=CreateRay(cameraPos, temp);


  // 4. Проверка пересечений
  float closestDist = FLT_MAX;
  //Object3D *selectedObj = nullptr;
  //selectedObject = nullptr;
  for (auto& obj : objects) {
    // Создаем AABB объекта
    AABB aabb;
    if (obj.isSphere) {
      aabb.min = obj.position - glm::vec3(obj.radius);
      aabb.max = obj.position + glm::vec3(obj.radius);
    } else {
      aabb.min = obj.position - glm::vec3(obj.size * 0.5f);
      aabb.max = obj.position + glm::vec3(obj.size * 0.5f);
    }

    if (RayCast(aabb, ray)) {
      // Можно дополнительно проверить точное пересечение с мешем, но для
      // выбора достаточно AABB Оцените "расстояние" до объекта — например,
      // по расстоянию от origin до точки пересечения Для простоты можно
      // считать, что ближайший объект — это тот, у которого t минимальный
      // Но t внутри RayCast не возвращается, нужно расширить функцию для
      // этого Для этого лучше переписать RayCast так, чтобы возвращал t или
      // false
      selectedObject = &obj;
      return;
    }
    else { selectedObject = nullptr; }
  }

  // Вариант: перепишем RayCast, чтобы он возвращал t
  // или создадим отдельную функцию для проверки пересечения и получения t
}



void carveWormTunnel(std::vector<float>& heightMap, int mapSize, int numWorms, int wormLength, float stepSize, float radius) {
  std::mt19937 rng(std::random_device{}());
  std::uniform_real_distribution<float> distPos(0.0f, mapSize - 1);
  std::uniform_real_distribution<float> distAngle(0.0f, 2 * M_PI);

  for (int i = 0; i < numWorms; ++i) {
    float x = distPos(rng);
    float y = distPos(rng);
    float angle = distAngle(rng);

    for (int j = 0; j < wormLength; ++j) {
      int ix = static_cast<int>(x);
      int iy = static_cast<int>(y);

      // Вырезаем туннель с мягкими краями
      int minX = std::max(0, ix - (int)radius - 1);
      int maxX = std::min(mapSize - 1, ix + (int)radius + 1);
      int minY = std::max(0, iy - (int)radius - 1);
      int maxY = std::min(mapSize - 1, iy + (int)radius + 1);

      for (int yy = minY; yy <= maxY; ++yy) {
	for (int xx = minX; xx <= maxX; ++xx) {
	  float dist = glm::length(glm::vec2(xx - x, yy - y));
	  if (dist < radius) {
	    // Внутри радиуса — делаем "отверстие" или понижаем высоты
	    float t = dist / radius; // 0..1
	    float smoothFactor = 1.0f - t; // чем ближе к центру, тем больше понижение
	    int index = yy * mapSize + xx;

	    // Плавное снижение высоты
	    heightMap[index] = glm::mix(heightMap[index], -10.0f, smoothFactor);
	  }
	}
      }

      // Обновляем позицию
      float dx = cos(angle);
      float dy = sin(angle);
      x += dx * stepSize + ((float)rand() / RAND_MAX - 0.5f) * 0.2f;
      y += dy * stepSize + ((float)rand() / RAND_MAX - 0.5f) * 0.2f;

      // Меняем угол для "завитков"
      angle += ((float)rand() / RAND_MAX - 0.5f) * 0.3f;

      // Ограничение, чтобы не вышли за границы
      x = std::clamp(x, 0.0f, (float)(mapSize - 1));
      y = std::clamp(y, 0.0f, (float)(mapSize - 1));
    }
  }
}





// Обновленная функция createTerrainMesh, использующая heightMap
void createTerrainMesh() {
  terrainVertices.clear();
  terrainIndices.clear();

  size_t vertCount = terrainSize * terrainSize;
  terrainVertices.reserve(vertCount);
  terrainIndices.reserve((terrainSize - 1) * (terrainSize - 1) * 6);

  // Создаем вершины по карте высот
  for (int z = 0; z < terrainSize; ++z) {
    for (int x = 0; x < terrainSize; ++x) {
      float h = heightMap[z * terrainSize + x]; // Используем вашу карту высот
      terrainVertices.push_back({ glm::vec3(x - terrainSize / 2.0f, h, z - terrainSize / 2.0f), glm::vec3(0.0f) });
    }
  }

  // Создаем индексы треугольников (без изменений)
  for (int z = 0; z < terrainSize - 1; ++z) {
    for (int x = 0; x < terrainSize - 1; ++x) {
      int topLeft = z * terrainSize + x;
      int topRight = topLeft + 1;
      int bottomLeft = (z + 1) * terrainSize + x;
      int bottomRight = bottomLeft + 1;

      terrainIndices.push_back(topLeft);
      terrainIndices.push_back(bottomLeft);
      terrainIndices.push_back(topRight);

      terrainIndices.push_back(topRight);
      terrainIndices.push_back(bottomLeft);
      terrainIndices.push_back(bottomRight);
    }
  }

  // Вычисляем нормали (без изменений)
  for (auto& v : terrainVertices) {
    v.normal = glm::vec3(0.0f);
  }
  for (size_t i = 0; i < terrainIndices.size(); i += 3) {
    unsigned int ia = terrainIndices[i];
    unsigned int ib = terrainIndices[i + 1];
    unsigned int ic = terrainIndices[i + 2];

    glm::vec3 v0 = terrainVertices[ia].position;
    glm::vec3 v1 = terrainVertices[ib].position;
    glm::vec3 v2 = terrainVertices[ic].position;

    glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

    terrainVertices[ia].normal += normal;
    terrainVertices[ib].normal += normal;
    terrainVertices[ic].normal += normal;
  }
  for (auto& v : terrainVertices) {
    v.normal = glm::normalize(v.normal);
  }
}

void generateHeightMap() {
  float scale = 50.0f;
  glm::vec2 center(SIZE / 2.0f, SIZE / 2.0f);

  glm::vec2 holeCenter(SIZE / 3.0f, SIZE / 2.0f);
  float holeRadius = 20.0f; // радиус отверстия

  for (int z = 0; z < SIZE; ++z) {
    for (int x = 0; x < SIZE; ++x) {
      glm::vec2 pos(x, z);
      glm::vec2 delta = pos - center;
      float dist = glm::length(delta) / (SIZE / 2.0f); // 0..1

      float valley = std::exp(-10.0f * dist * dist);
      float mountain = std::pow(1.f - dist, 3);
      float noiseVal = glm::perlin(glm::vec2(x,z) / scale) * 0.2f;
      float entrance = std::exp(-50.0f * dist * dist);

      float height = (valley * 0.2f + mountain * 0.8f + noiseVal * 0.3f) - entrance * 0.5f;

      // // Создаем отверстие (например, в центре)
      // float distToHoleCenter = glm::length(glm::vec2(x, z) - holeCenter);
      // if (distToHoleCenter < holeRadius) {
      //     float t = distToHoleCenter / holeRadius; // 0..1
      //     height = glm::mix(height, -10.0f, 1.0f - t); // плавное понижение
      // }

      // Масштабируем итоговую высоту
      height *= 50.0f;

      heightMap[z * 512 + x] = height;
    }
  }
}


void setupTerrainBuffers() {
  glGenVertexArrays(1, &terrainVAO);
  glGenBuffers(1, &terrainVBO);
  glGenBuffers(1, &terrainEBO);

  glBindVertexArray(terrainVAO);

  glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
  glBufferData(GL_ARRAY_BUFFER, terrainVertices.size() * sizeof(Vertex), terrainVertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, terrainIndices.size() * sizeof(unsigned int), terrainIndices.data(), GL_STATIC_DRAW);

  // Позиции
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
  glEnableVertexAttribArray(0);
  // Нормали
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normal)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
}

void smoothHeightMap(std::vector<float> &heightMap, int size, int iterations) {
  //float* temp = new float[size * size];
HeightMap temp(SIZE * SIZE, 0.0);
    for (int iter = 0; iter < iterations; ++iter) {
        for (int z = 0; z < size; ++z) {
            for (int x = 0; x < size; ++x) {
                float sum = 0.0f;
                int count = 0;

                // Собираем соседние ячейки (8 направлений + центр)
                for (int dz = -1; dz <= 1; ++dz) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        int nx = x + dx;
                        int nz = z + dz;
                        if (nx >= 0 && nx < size && nz >= 0 && nz < size) {
                            sum += heightMap[nz * size + nx];
                            count++;
                        }
                    }
                }
                temp[z * size + x] = sum / count;
            }
        }
        std::swap(heightMap, temp);
    }
    //delete[] temp;
}

// void generateHeightMap() {
//     float scale = 50.0f;
//     glm::vec2 center(SIZE / 2.0f, SIZE / 2.0f);

//     for (int z = 0; z < SIZE; ++z) {
//         for (int x = 0; x < SIZE; ++x) {
//             glm::vec2 pos(x, z);
//             glm::vec2 delta = pos - center;
//             float dist = glm::length(delta) / (SIZE / 2.0f); // 0..1

//             float valley = std::exp(-10.0f * dist * dist);
//             float mountain = std::pow(1.f - dist, 3);
//             float noiseVal = glm::perlin(glm::vec2(x,z) / scale) * 0.2f;
//             float entrance = std::exp(-50.0f * dist * dist);

//             float height = (valley * 0.2f + mountain * 0.8f + noiseVal * 0.3f) - entrance * 0.5f;

//             // Масштабируем итоговую высоту
//             height *= 50.0f;

//             // Создаем круговой спуск/обрыв
//             float radius = glm::length(delta); // радиус от центра
//             float maxRadius = SIZE / 2.0f; // максимум радиуса по карте

//             // Определим границу круга (например, в 80% радиуса карты)
//             float circleRadius = 0.8f * maxRadius;

//             // Внутри круга: возвышенность
//             if (radius < circleRadius * 0.6f) {
//                 // Центральный возвышенный район
//                 height += 20.0f; // добавляем возвышенность
//             }
//             // Вблизи границы круга: крутой обрыв
//             else if (radius >= circleRadius * 0.6f && radius <= circleRadius) {
//                 // Уменьшаем высоту резко, создаем крутой обрыв
//                 float t = (radius - circleRadius * 0.6f) / (circleRadius - circleRadius * 0.6f); // 0..1
//                 float cliffHeight = glm::mix(20.0f, -10.0f, t); // от возвышенности к обрыву
//                 height = glm::mix(height, cliffHeight, 1.0f);
//             }
//             // Вне круга: низкая или отрицательная высота (можно оставить как есть)
//             else {
//                 // Можно оставить без изменений или задать низкую высоту
//                 height -= 10.0f; // например
//             }

//             heightMap[z * SIZE + x] = height;
//         }
//     }
// }


struct Character {
    GLuint TextureID;  // ID текстуры глифа
    glm::ivec2 Size;   // Размер глифа
    glm::ivec2 Bearing; // Смещение
    GLuint Advance;    // Шаг
};

std::map<char, Character> Characters;
void createTextureFont() {
  // disable byte-alignment restriction
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  for (unsigned char c = 0; c < 128; c++) {
    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
      std::cerr << "Failed to load glyph" << std::endl;
      continue;
    }
    // Создайте OpenGL текстуру из face->glyph->bitmap
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
		 GL_TEXTURE_2D,
		 0,
		 GL_RED,
		 face->glyph->bitmap.width,
		 face->glyph->bitmap.rows,
		 0,
		 GL_RED,
		 GL_UNSIGNED_BYTE,
		 face->glyph->bitmap.buffer
		 );
    // Настройка параметров текстуры
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Запоминаем информацию о символе
    Character character = {
      texture,
      glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
      glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
      (GLuint)face->glyph->advance.x
    };
    Characters.insert(std::pair<char, Character>(c, character));
  }
}
GLuint VAOtext, VBOtext;
void RenderText(GLuint &shader, std::string text, float x, float y, float scale, glm::vec3 color,glm::mat4 &proj) {
  // shader.use();
  glUseProgram(shader);
    int projLoc = glGetUniformLocation(shader, "projection");
    //glUseProgram(shader);
  glUniformMatrix4fv(projLoc, 1, GL_FALSE, &proj[0][0]);
  glUniform3f(glGetUniformLocation(shader, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAOtext); // VAO для текстовых символов

    for (char c : text) {
        Character ch = Characters[c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        // Обновите VBO для каждого символа
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };

        // Обновляем VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBOtext);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        // Рисуем глиф
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Обновляем позицию для следующего символа
        x += (ch.Advance >> 6) * scale; // сдвиг на следующий символ
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void configTextbufs() {
      // configure VAO/VBO for texture quads
    // -----------------------------------
    glGenVertexArrays(1, &VAOtext);
    glGenBuffers(1, &VBOtext);
    glBindVertexArray(VAOtext);
    glBindBuffer(GL_ARRAY_BUFFER, VBOtext);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

int main() {
  srand(time(nullptr));

  if (FT_Init_FreeType(&ft)) {
    std::cerr << "Could not init FreeType Library" << std::endl;
    // обработка ошибки
  }

  if (FT_New_Face(ft, "DejaVuSansMono.ttf", 0, &face)) {
    std::cerr << "Failed to load font" << std::endl;
    // обработка ошибки
  }

  FT_Set_Pixel_Sizes(face, 0, 48); // размер шрифта
  
  // Инициализация GLFW
  if (!glfwInit()) return -1;
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_SAMPLES, 4); 
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow* window = glfwCreateWindow(800,600,"OpenGL 4.6 Cube & Sphere Scene",nullptr,nullptr);
  if (!window) { glfwTerminate(); return -1; }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);

  // tell GLFW to capture our mouse
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glewExperimental=GL_TRUE;
  if (glewInit()!=GLEW_OK) { std::cerr<<"Glew init failed\n"; return -1; }

  glEnable(GL_DEPTH_TEST);
  // glDepthMask(1);
  // glEnable(GL_CULL_FACE);
  // glCullFace(GL_BACK);
  // glFrontFace(GL_CW);
  //  glEnable(GL_MULTISAMPLE);
  // generateHeightMap();
  //  generatePerlinWorms(heightMap, SIZE, 50, 100, 1.0f, 0.5f);
  // carveWormTunnel(heightMap, SIZE, 50, 100, 1.0f, 20.0f);
  // smoothHeightMap(heightMap,512,3);
  // generateComplexTunnel(heightMap,SIZE,2,20,1.0f,20.1f,10.0f);//
  
int width = SIZE; // ширина карты
int height = SIZE; // высота карты
int channels; 
unsigned char* data = stbi_load("heightmap3.png", &width, &height, &channels, 1); // 1 — для grayscale

if (data == nullptr) {
    // Обработка ошибки
    std::cerr << "Failed to load image" << std::endl;
} else {
  // data содержит изображение в виде массива байтов (грейскейл)
  // width и height — размеры изображения
  // channels — число каналов (будет 1, так как мы указали 1)
  
for (int i = 0; i < width * height; ++i) {
  heightMap[i] = (data[i] / 255.0f)*20.0f; // преобразование к диапазону 0-1
}
}
stbi_image_free(data);
  createTerrainMesh();
  setupTerrainBuffers();
  //glEnable(GL_FRAMEBUFFER_SRGB);
  GLuint shaderProgram = createProgram(vertexShaderSrc,fragmentShaderSrc);//main
  GLuint shaderDepthProgram = createProgram(depth_vertex_shadersrc, depth_fragment_shader);//shadow
  
  // VAO/VBO для куба
  GLuint cubeVAO, cubeVBO;
  createVAOVBObufs(cubeVAO, cubeVBO);


  //GLuint depthMapFBO;
  //GLuint depthMap;
  //const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

  //createDBuffer(depthMapFBO,depthMap);



  GLuint DshaderProgram = createProgram(vDescriptorCentrSrc,fDescriptorCentrSrc);//cross
  GLuint dVAO, dVBO, dEBO;
  createDescriptor(dVAO,dVBO,dEBO);
  // Загружаем текстуру крестика (укажи свой путь)
  GLuint textureID = loadTexture("descriptorCentr.png");
  if (textureID == 0)
    {
      std::cerr << "Ошибка загрузки текстуры.\n";
      return -1;
    }

    GLuint shaderText = createProgram(vTSrc, fTSrc); // text
    configTextbufs();
  createTextureFont();
  


   
    
  // Создаем объекты
  
  glm::vec3 v=glm::vec3{0,0,0};
  createOneBigCubeCoords(objects, v);
    
  // Создаем объекты
  std::vector<Object3D> objects1;
  // glm::vec3 v1=glm::vec3{200,0,0};
  // createOneBigCubeCoords(objects1, v1);

  // Создаем объекты
  std::vector<Object3D> objects2;
  // glm::vec3 v2=glm::vec3{0,0,-200};
  // createOneBigCubeCoords(objects2, v2);
    
  // Создаем объекты
  std::vector<Object3D> objects3;
  // glm::vec3 v3=glm::vec3{200,0,-200};
  // createOneBigCubeCoords(objects3,v3);


  // Создаем объекты
  std::vector<Object3D> objects4;
  // glm::vec3 v4=glm::vec3{0,200,0};
  // createOneBigCubeCoords(objects4, v4);
    
  // Создаем объекты
  std::vector<Object3D> objects5;
  // glm::vec3 v5=glm::vec3{200,200,0};
  // createOneBigCubeCoords(objects5, v5);

  // Создаем объекты
  std::vector<Object3D> objects6;
  // glm::vec3 v6=glm::vec3{0,200,-200};
  // createOneBigCubeCoords(objects6, v6);
    
  // Создаем объекты
  std::vector<Object3D> objects7;
  // glm::vec3 v7=glm::vec3{200,200,-200};
  // createOneBigCubeCoords(objects7,v7);

    
  // 
  // Построение BVH
  std::vector<Object3D*> objPtrs;
  for (auto& o: objects) objPtrs.push_back(&o);
  bvhRoot = buildBVH(objPtrs);

  // Построение BVH
  std::vector<Object3D*> objPtrs1;
  // for (auto& o: objects1) objPtrs1.push_back(&o);
  // BVHNode *bvhRoot1 = buildBVH(objPtrs1);

  // 
  // Построение BVH
  std::vector<Object3D*> objPtrs2;
  // for (auto& o: objects2) objPtrs2.push_back(&o);
  // BVHNode* bvhRoot2 = buildBVH(objPtrs2);

  // Построение BVH
  std::vector<Object3D*> objPtrs3;
  // for (auto& o: objects3) objPtrs3.push_back(&o);
  // BVHNode *bvhRoot3 = buildBVH(objPtrs3);

  // 
  // Построение BVH
  std::vector<Object3D*> objPtrs4;
  // for (auto& o: objects4) objPtrs4.push_back(&o);
  // BVHNode* bvhRoot4 = buildBVH(objPtrs4);

  // Построение BVH
  std::vector<Object3D*> objPtrs5;
  // for (auto& o: objects5) objPtrs5.push_back(&o);
  // BVHNode *bvhRoot5 = buildBVH(objPtrs5);

  // 
  // Построение BVH
  std::vector<Object3D*> objPtrs6;
  // for (auto& o: objects6) objPtrs6.push_back(&o);
  // BVHNode* bvhRoot6 = buildBVH(objPtrs6);

  // Построение BVH
  std::vector<Object3D*> objPtrs7;
  // for (auto& o: objects7) objPtrs7.push_back(&o);
  // BVHNode *bvhRoot7 = buildBVH(objPtrs7);
    
    
  glm::vec3 min = {-10, -10, -10};
  glm::vec3 max={10,10,10};
  glm::vec3 velB1 = {((rand() % 100) / 100.0f - 0.5f) * 0.01f,
                     ((rand() % 100) / 100.0f - 0.5f) * 0.01f,
                     ((rand() % 100) / 100.0f - 0.5f) * 0.01f};



// // Создаем массив байтов для хранения изображения
// unsigned char* imageData = new unsigned char[width * height];

// for (int i = 0; i < width * height; ++i) {
//     float value = heightMap[i];
//     // Ограничим значение в диапазоне 0-1
//     if (value < 0.0f) value = 0.0f;
//     if (value > 1.0f) value = 1.0f;

//     imageData[i] = static_cast<unsigned char>(value * 255.0f);
// }
// // Сохраняем как PNG (grayscale)
// stbi_write_png("heightmap.png", width, height, 1, imageData, width);

// delete[] imageData;


  
  // Настройка OpenGL
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  
  // Основной цикл
  while (!glfwWindowShouldClose(window)) {
    // Камера
    if(teleportationSTATUS){
      if (selectedObject != nullptr) {
	// Телепортируемся
	cameraPos = selectedObject->position + glm::vec3(0, 2, 5); // смещение для обзора
	//cameraFront = glm::normalize(selectedObject->position - cameraPos);
      }
    }

  lightProjection = glm::perspective(glm::radians(45.0f), (float)wi / he, 1.0f, 2000.f);//glm::ortho(-2024, 2024, -2024, 2024,near_plane,far_plane);//
  lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  lightSpaceMatrix = lightProjection * lightView;

    
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    glm::mat4 projection =
      glm::perspective(glm::radians(fov), (float)wi / he, 0.1f, 2000.f);
    glm::mat4 Oproj = glm::ortho(0.0f, float(wi), 0.0f, float(he),-1.f,1.f);
    // per-frame time logic
    // --------------------
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    // input
    // -----
    processInput(window);
    //angle += glm::radians(20.0f) * deltaTime; // скорость вращения
    //lightPos.x = 200.0f * cos(angle);
    //lightPos.z = 200.0f * sin(angle);
    updateObjects(objects, bvhRoot);
    glClearColor(0.1f,0.1f,0.15f,1.f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);



    
    // updateObjects(objects1, bvhRoot1);
    // updateObjects(objects2, bvhRoot2);
    // updateObjects(objects3, bvhRoot3);
    // updateObjects(objects4, bvhRoot4);
    // updateObjects(objects5, bvhRoot5);
    // updateObjects(objects6, bvhRoot6);
    // updateObjects(objects7, bvhRoot7);
    // // Проверка столкновений






    
    // renderShadow(shaderDepthProgram,depthMapFBO,cubeVAO,
    // 		 objects,
    // 		 objects1,
    // 		 objects2,
    // 		 objects3,
    // 		 objects4,
    // 		 objects5,
    // 		 objects6,
    // 		 objects7,lightSpaceMatrix);

    //renderTerrain(shaderProgram);
    RenderObjsMain(shaderProgram, NULL, cubeVAO,
		   lightSpaceMatrix, view,
		   projection,
		   objects,
		   objects1,
		   objects2,
		   objects3,
		   objects4,
		   objects5,
		   objects6,
		   objects7);

    //like help-UI draw(ortho)

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    drawDescriptor(DshaderProgram,textureID,dVAO,glm::mat4(1.0f),Oproj);
    //glDisable(GL_BLEND);
 
    RenderText(shaderText, "This is sample text", 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f),Oproj);
    RenderText(shaderText, "(C) LearnOpenGL.com",wi-280.0f, he-30.0f, 0.5f, glm::vec3(0.3, 0.7f, 0.9f),Oproj);
       
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  delete bvhRoot;
  // delete bvhRoot1;
  // delete bvhRoot2;
  // delete bvhRoot3;
  // delete bvhRoot4;
  // delete bvhRoot5;
  // delete bvhRoot6;
  // delete bvhRoot7;
  // Очистка ресурсов
  glDeleteProgram(shaderText);
  glDeleteVertexArrays(1, &VAOtext);
  glDeleteBuffers(1, &VBOtext);
  glDeleteProgram(shaderDepthProgram);
  glDeleteVertexArrays(1, &dVAO);
  glDeleteBuffers(1, &dVBO);
  glDeleteBuffers(1, &dEBO);
  glDeleteVertexArrays(1, &terrainVAO);
  glDeleteBuffers(1, &terrainVBO);
  glDeleteBuffers(1, &terrainEBO);
  glDeleteProgram(DshaderProgram);
  glDeleteTextures(1, &textureID);
  glDeleteBuffers(1,&cubeVBO);
  glDeleteVertexArrays(1,&cubeVAO);
  glDeleteProgram(shaderProgram);
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
bool escPressedLastFrame = false;
void processInput(GLFWwindow *window)
{
  // Обработка ESC
  int escState = glfwGetKey(window, GLFW_KEY_ESCAPE);
  if (escState == GLFW_PRESS && !escPressedLastFrame) {
    // Первое нажатие
    escPressedLastFrame = true;
    if (mousetoogle) {
      //
      mousetoogle = false;
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
      // Закрыть окно
      glfwSetWindowShouldClose(window, true);
    }
  } else if (escState == GLFW_RELEASE) {
    // Освободить флаг
    escPressedLastFrame = false;
  }
  float cameraSpeed = static_cast<float>(20.5 * deltaTime);
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    cameraPos += cameraSpeed * cameraFront;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    cameraPos -= cameraSpeed * cameraFront;
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
  // В функции обработки клавиш (например, в processInput):
  if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
    if(teleportationSTATUS==false)teleportationSTATUS=true;
  }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina displays.
  wi = width;
  he = height;
  glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
  if(mousetoogle){
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
      lastX = xpos;
      lastY = ypos;
      firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
      pitch = 89.0f;
    if (pitch < -89.0f)
      pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
  }
}


bool mouseButtonLastState = false;
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS && !mouseButtonLastState) {
      // Первое нажатие
      mouseButtonLastState = true;

      if (!mousetoogle) {
	//
	      
	mousetoogle = true;
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      } else if (mousetoogle) {
        double xpos, ypos;
	glfwGetCursorPos(window,&xpos,&ypos);
	pickObject(xpos, ypos,objects, view, projection,cameraPos);
      }
    }
    else if (action == GLFW_RELEASE) {
      // Освободить флаг при отпускании
      mouseButtonLastState = false;
    }
  }
}
