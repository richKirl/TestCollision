#include "glm/ext/matrix_clip_space.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
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

// --- Структуры объектов и BVH ---
#define CXX 100 // step
#define tCXX 2*CXX
#define tN 3000
int wi=800;
int he=600;
struct Object3D {
  glm::vec3 position;
  glm::vec3 prevpos;
  glm::vec3 velocity;
  float spped;
  glm::vec3 size;     // для кубика
  glm::vec3 radius;   // для сферы
  bool isSphere;
  bool collided = false; // для визуализации столкновений
  bool stayObj=false;
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
    //float massA = a.isSphere ? glm::length(a.radius) : glm::length(a.size);
    //float massB = b.isSphere ? glm::length(b.radius) : glm::length(b.size);

float massA = a.stayObj ? 1e9f : glm::length(a.isSphere ? a.radius : a.size);
float massB = b.stayObj ? 1e9f : glm::length(b.isSphere ? b.radius : b.size);

  
    // Вектор столкновения
    glm::vec3 collisionNormal = glm::normalize(b.position - a.position);
    float relativeVelocity = glm::dot(b.velocity - a.velocity, collisionNormal);

    if (relativeVelocity > 0) return; // объекты удаляются друг от друга

    // Расчет импульса
    float restitution = 0.1f; // коэффициент восстановления, 1 — идеально упругий
    float impulseMag = -(1 + restitution) * relativeVelocity / (1/massA + 1/massB);

    glm::vec3 impulse = impulseMag * collisionNormal;

    if (!a.stayObj) a.velocity -= (1/massA) * impulse;
    if (!b.stayObj) b.velocity += (1/massB) * impulse;
}


void traverseBVH(BVHNode* node, Object3D* obj) {
  if (!node || !node->box.intersects(AABB(obj->position - (obj->isSphere ? obj->radius : obj->size),
					  obj->position + (obj->isSphere ? obj->radius : obj->size)))) return;
  if (node->isLeaf()) {
    for (auto* other : node->objects) {
      if (other != obj && checkCollision(*obj, *other)) {
        // if (!obj->stayObj) {
        //   //resolveCollision(*obj,*other);
        //   obj->velocity *= -1;
        //   obj->collided = true;
        // }
        if (other->stayObj) {
          //resolveCollision(*obj,*other);
          //other->velocity *= -1;
	  obj->velocity *= -1;
          //other->collided = true;
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

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

const char *fragmentShaderSrc = R"(
#version 460 core
out vec4 FragColor;


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

    float bias = 0.005;
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    // Можно добавить мягкое затенение (PCF) для более гладких теней
    return shadow;
}


void main() {
    // Амбиентное
    vec3 ambient = 0.3 * objectColor;
    // Диффузное
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);

    float shadow = ShadowCalculation(FragPosLightSpace);

    vec3 diffuse = (1.0 - shadow) *diff * objectColor*2.0f;
    // Блик
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = vec3(0.5) * spec;
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
)";

const char *depth_vertex_shadersrc = R"(
#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 lightSpaceMatrix;

void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}
)";
const char* depth_fragment_shader = R"(
#version 330 core
void main()
{
    // ничего не нужно, depth берется автоматически
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

  // Создать текстуру для depth
  
  glGenTextures(1, &depthMap);
  glBindTexture(GL_TEXTURE_2D, depthMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float borderColor[] = {1.0, 1.0, 1.0, 1.0};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

  // Attach depth texture to FBO
  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}










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
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 30.0f);
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
	glm::vec3(x+v.x, y+v.y, z+v.z),
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
  wallLeft.stayObj = true; // статичный объект

  Object3D wallRight;
  wallRight.position = glm::vec3(CXX+v.x, v.y, v.z);
  wallRight.size = glm::vec3(1.f,tCXX,tCXX);;
  wallRight.radius = glm::vec3(0.0f);
  wallRight.isSphere = false;
  wallRight.name = "WallRight";
  wallRight.stayObj = true;

  Object3D wallTop;
  wallTop.position = glm::vec3(v.x, CXX+v.y, v.z);
  wallTop.size = glm::vec3(tCXX,1.0f,tCXX); // длина по X
  wallTop.radius = glm::vec3(0.0f);
  wallTop.isSphere = false;
  wallTop.name = "WallTop";
  wallTop.stayObj = true;

  Object3D wallBottom;
  wallBottom.position = glm::vec3(v.x, -CXX+v.y, v.z);
  wallBottom.size = glm::vec3(tCXX,1.0f,tCXX); // длина по X
  wallBottom.radius = glm::vec3(0.0f);
  wallBottom.isSphere = false;
  wallBottom.name = "WallBottom";
  wallBottom.stayObj = true;

  Object3D wallFront;
  wallFront.position = glm::vec3(v.x, v.y, CXX+v.z);
  wallFront.size = glm::vec3(tCXX,tCXX,1.0f); // длина по X
  wallFront.radius = glm::vec3(0.0f);
  wallFront.isSphere = false;
  wallFront.name = "WallFront";
  wallFront.stayObj = true;

  Object3D wallBack;
  wallBack.position = glm::vec3(v.x, v.y, -CXX+v.z);
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
glm::vec3 lightPos = glm::vec3(200.0f, 400.0f, 200.0f);
float angle = 0.0f; // угол для движения по окружности




void updateObjects(std::vector<Object3D>& objects, BVHNode* bvhRoot) {
    for (auto& o : objects) {
        o.collided = false;
        if (!o.stayObj) {
            o.position += o.velocity;
            traverseBVH(bvhRoot, &o);
            // Можно добавить ограничение по границам сцены
            // например, чтобы объекты не выходили за рамки
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
    }
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
    glViewport(0, 0, wi, he);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Используем шейдер для depth
    glUseProgram(shaderDepthProgram);

    // Создаем матрицу света
    glm::mat4 lightProjection, lightView;
    int near_plane = 1.0f, far_plane = 2000.0f;
    lightProjection = glm::ortho(-CXX, CXX, -CXX, CXX,near_plane,far_plane);//
    lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    lightSpaceMatrix = lightProjection * lightView;

    // Передача матрицы в шейдер
    glUniformMatrix4fv(glGetUniformLocation(shaderDepthProgram, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);

    RObjects(objects,cubeVAO,shaderDepthProgram);
    RObjects(objects1, cubeVAO, shaderDepthProgram);
    RObjects(objects2, cubeVAO, shaderDepthProgram);
    RObjects(objects3, cubeVAO, shaderDepthProgram);
    RObjects(objects4, cubeVAO, shaderDepthProgram);
    RObjects(objects5, cubeVAO, shaderDepthProgram);
    RObjects(objects6, cubeVAO, shaderDepthProgram);
    RObjects(objects7,cubeVAO,shaderDepthProgram);

    // Окончание рендеринга в depth карту
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



void RObjects(std::vector<Object3D>& objects,GLuint cubeVAO,GLint locModel,GLint locColor) {
    for (auto &o : objects) {
      if (o.stayObj) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

      // Перед отрисовкой
      glm::vec3 scaleVal = o.isSphere ? glm::vec3(o.radius.x * 2.0f) : o.size;
      glm::mat4 model = glm::translate(glm::mat4(1.f), o.position);
      model = glm::scale(model, scaleVal);
      glUniformMatrix4fv(locModel,1,false,&model[0][0]);
      glUniform3f(locColor, o.collided ? 1.f:0.f, 0.f, o.collided ? 0.f : 1.f);
      glDrawArrays(GL_TRIANGLES,0,36);
    }
}


void RenderObjsMain(GLuint shaderProgram, GLuint depthMap, GLuint cubeVAO,
                    glm::mat4 lightSpaceMatrix, glm::mat4 view,
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
    glUseProgram(shaderProgram);
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

    GLint locView = glGetUniformLocation(shaderProgram,"view");
    GLint locProj = glGetUniformLocation(shaderProgram,"projection");
    glUniformMatrix4fv(locView,1,false,&view[0][0]);
    glUniformMatrix4fv(locProj,1,false,&projection[0][0]);
    // Установка uniform-параметров освещения
    GLint locLightPos = glGetUniformLocation(shaderProgram, "lightPos");
    GLint locViewPos = glGetUniformLocation(shaderProgram, "viewPos");
    //glUseProgram(shaderProgram);
    glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
    glUniform3f(locViewPos, cameraPos.x, cameraPos.y, cameraPos.z);
    GLint locModel = glGetUniformLocation(shaderProgram,"model");
    GLint locColor = glGetUniformLocation(shaderProgram,"objectColor");

    glBindVertexArray(cubeVAO);
    RObjects(objects,cubeVAO,locModel,locColor);
    RObjects(objects1, cubeVAO, locModel, locColor);
    RObjects(objects2, cubeVAO, locModel, locColor);
    RObjects(objects3, cubeVAO, locModel, locColor);
    RObjects(objects4, cubeVAO, locModel, locColor);
    RObjects(objects5, cubeVAO, locModel, locColor);
    RObjects(objects6, cubeVAO, locModel, locColor);
    RObjects(objects7,cubeVAO,locModel,locColor);

    glBindVertexArray(0);
}










int main() {
  srand(time(nullptr));
  // Инициализация GLFW
  if (!glfwInit()) return -1;
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
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

   
  GLuint shaderProgram = createProgram(vertexShaderSrc,fragmentShaderSrc);
  GLuint shaderDepthProgram = createProgram(depth_vertex_shadersrc,depth_fragment_shader);
  // VAO/VBO для куба
  GLuint cubeVAO, cubeVBO;
  createVAOVBObufs(cubeVAO, cubeVBO);


  GLuint depthMapFBO;
  GLuint depthMap;
  const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

  createDBuffer(depthMapFBO,depthMap);
    

  // Создаем объекты
  std::vector<Object3D> objects;
  glm::vec3 v=glm::vec3{0,0,0};
  createOneBigCubeCoords(objects, v);
    
  // Создаем объекты
  std::vector<Object3D> objects1;
  glm::vec3 v1=glm::vec3{200,0,0};
  createOneBigCubeCoords(objects1, v1);

  // Создаем объекты
  std::vector<Object3D> objects2;
  glm::vec3 v2=glm::vec3{0,0,-200};
  createOneBigCubeCoords(objects2, v2);
    
  // Создаем объекты
  std::vector<Object3D> objects3;
  glm::vec3 v3=glm::vec3{200,0,-200};
  createOneBigCubeCoords(objects3,v3);


  // Создаем объекты
  std::vector<Object3D> objects4;
  glm::vec3 v4=glm::vec3{0,200,0};
  createOneBigCubeCoords(objects4, v4);
    
  // Создаем объекты
  std::vector<Object3D> objects5;
  glm::vec3 v5=glm::vec3{200,200,0};
  createOneBigCubeCoords(objects5, v5);

  // Создаем объекты
  std::vector<Object3D> objects6;
  glm::vec3 v6=glm::vec3{0,200,-200};
  createOneBigCubeCoords(objects6, v6);
    
  // Создаем объекты
  std::vector<Object3D> objects7;
  glm::vec3 v7=glm::vec3{200,200,-200};
  createOneBigCubeCoords(objects7,v7);

    
  // 
  // Построение BVH
  std::vector<Object3D*> objPtrs;
  for (auto& o: objects) objPtrs.push_back(&o);
  BVHNode* bvhRoot = buildBVH(objPtrs);

  // Построение BVH
  std::vector<Object3D*> objPtrs1;
  for (auto& o: objects1) objPtrs1.push_back(&o);
  BVHNode *bvhRoot1 = buildBVH(objPtrs1);

  // 
  // Построение BVH
  std::vector<Object3D*> objPtrs2;
  for (auto& o: objects2) objPtrs2.push_back(&o);
  BVHNode* bvhRoot2 = buildBVH(objPtrs2);

  // Построение BVH
  std::vector<Object3D*> objPtrs3;
  for (auto& o: objects3) objPtrs3.push_back(&o);
  BVHNode *bvhRoot3 = buildBVH(objPtrs3);

  // 
  // Построение BVH
  std::vector<Object3D*> objPtrs4;
  for (auto& o: objects4) objPtrs4.push_back(&o);
  BVHNode* bvhRoot4 = buildBVH(objPtrs4);

  // Построение BVH
  std::vector<Object3D*> objPtrs5;
  for (auto& o: objects5) objPtrs5.push_back(&o);
  BVHNode *bvhRoot5 = buildBVH(objPtrs5);

  // 
  // Построение BVH
  std::vector<Object3D*> objPtrs6;
  for (auto& o: objects6) objPtrs6.push_back(&o);
  BVHNode* bvhRoot6 = buildBVH(objPtrs6);

  // Построение BVH
  std::vector<Object3D*> objPtrs7;
  for (auto& o: objects7) objPtrs7.push_back(&o);
  BVHNode *bvhRoot7 = buildBVH(objPtrs7);
    
    
  glm::vec3 min = {-10, -10, -10};
  glm::vec3 max={10,10,10};
  glm::vec3 velB1={((rand() % 100) / 100.0f - 0.5f) * 0.01f,
		   ((rand() % 100) / 100.0f - 0.5f) * 0.01f,
		   ((rand() % 100) / 100.0f - 0.5f) * 0.01f};
  // Основной цикл
  while (!glfwWindowShouldClose(window)) {
    // per-frame time logic
    // --------------------
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    // input
    // -----
    processInput(window);
    angle += glm::radians(20.0f) * deltaTime; // скорость вращения
    lightPos.x = 200.0f * cos(angle);
    lightPos.z = 200.0f * sin(angle);
    glClearColor(0.1f,0.1f,0.15f,1.f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

updateObjects(objects, bvhRoot);
updateObjects(objects1, bvhRoot1);
updateObjects(objects2, bvhRoot2);
updateObjects(objects3, bvhRoot3);
updateObjects(objects4, bvhRoot4);
updateObjects(objects5, bvhRoot5);
updateObjects(objects6, bvhRoot6);
updateObjects(objects7, bvhRoot7);
    // // Проверка столкновений

    // Камера
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    glm::mat4 projection =
      glm::perspective(glm::radians(fov), (float)wi / he, 0.1f, 2000.f);



glm::mat4 lightSpaceMatrix;
renderShadow(shaderDepthProgram,depthMapFBO,cubeVAO,
		  objects,
                  objects1,
                  objects2,
                  objects3,
                  objects4,
                  objects5,
                  objects6,
	     objects7,lightSpaceMatrix);



 RenderObjsMain(shaderProgram, depthMap, cubeVAO,
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


      

 
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  delete bvhRoot;
  delete bvhRoot1;
  delete bvhRoot2;
  delete bvhRoot3;
  delete bvhRoot4;
  delete bvhRoot5;
  delete bvhRoot6;
  delete bvhRoot7;
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
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS && !mouseButtonLastState) {
      // Первое нажатие
      mouseButtonLastState = true;

      if (!mousetoogle) {
	//
	      
	mousetoogle = true;
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      } 
    } else if (action == GLFW_RELEASE) {
      // Освободить флаг при отпускании
      mouseButtonLastState = false;
    }
  }
}
