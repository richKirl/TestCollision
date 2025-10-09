#ifndef LIGHT_HPP
#define LIGHT_HPP
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
struct Light
{
    float asp;
    glm::vec3 lightPos = glm::vec3(0.f, 512.0f, 0.0f);
    glm::mat4 lightSpaceMatrix, lightProjection, lightView;
    void update();
};

#endif // LIGHT_HPP