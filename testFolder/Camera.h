#ifndef CAMERA_HPP
#define CAMERA_HPP
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


struct Camera
{
    /* data */
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 10.0f); // 50.0f-256.0f, -220.0f,440.0f-256.0f
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    float yaw = -90.0f; // yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
    float pitch = 0.0f;
    float movementSpeed = 20.5f; // скорость перемещения
    float mouseSensitivity = 0.1f; // чувствительно
    // Обновление направления камеры на основе yaw и pitch
    void updateCameraFront();
    // Обработка движения мыши для поворота камеры
    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

    // Получение матрицы вида
    glm::mat4 getViewMatrix();
};

#endif // CAMERA_HPP