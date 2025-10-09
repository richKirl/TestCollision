#include "Light.h"

void Light::update()
{
    lightProjection = glm::perspective(glm::radians(45.0f), asp, 0.1f, 2000.f); // glm::ortho(-SIZE, SIZE, -SIZE, SIZE,-SIZE,SIZE);//
    lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    lightSpaceMatrix = lightProjection * lightView * glm::mat4(1.0);
}