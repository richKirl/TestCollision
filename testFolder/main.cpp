#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Window.h"
#include "Font.h"
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include "Camera.h"
#include "CubeV.h"
#include "Terrain.h"
#include "Light.h"
#include "Utils.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool mousetoogle = true;
bool firstMouse = true;
struct LastMouse
{
    float lastX; //= win.width / 2.0;
    float lastY; //= win.height / 2.0;
};
LastMouse l;
Camera camera;
void processInput(GLFWwindow *window, Heightmap *heightmap, Camera *camera, float deltaTime);
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn); //, Camera *camera,LastMouse *l
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
////////////////////////////////////////////////////////////////////////////////////////////////////////////





int main()
{
    // first step init
    // init window
    Window win;
    l.lastX = win.width / 2.0;
    l.lastY = win.height / 2.0;
    // init glfw and glew
    initWindow(&win, 800, 600, "Test Scene");
    glfwSetCursorPosCallback(win.window, mouse_callback);
    glfwSetMouseButtonCallback(win.window, mouse_button_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(win.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // mouse_callback(GLFWwindow *window, double xposIn, double yposIn,&camera,&last);
    
    Shader shaderMain("asset/shaders/mainV.glsl", "asset/shaders/mainF.glsl");
    
    Shader shaderText("asset/shaders/vText.glsl", "asset/shaders/fText.glsl");

    TextV textV;
    CharacterV charV;
    createTextureFont(&charV, "asset/font/DejaVuSansMono.ttf", 48);
    configTextbufs(&textV);

    CubeV cubev;
    createVAOVBObufs(&cubev);

    glm::mat4 termodel = glm::translate(glm::mat4(1.f), glm::vec3(0.0f, -240.0f, 0.0f));
    Heightmap heightmap;
    TerrainVAO terrainVAO;
    TerrainVertices terrainVertices;
    createFromImage(&heightmap, &terrainVAO, &terrainVertices);
    

    Light light;

    // main cycle
    float deltaTime = 0;
    float lastFrame = 0;
    while (!glfwWindowShouldClose(win.window))
    {
        float terrainH = getHeightAt(&heightmap, camera.cameraPos.x, camera.cameraPos.z) - 239.0f;
        if (camera.cameraPos.y > terrainH + 1.0f)
        {
            camera.cameraPos.y -= 0.1f; // прыжок
        }
        if (camera.cameraPos.y <= terrainH + 1.0f)
        {
            camera.cameraPos.y = terrainH + 1.0f; // прыжок
        }

        glm::mat4 view = camera.getViewMatrix();
        float asp = (float)win.width / win.height;
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), asp, 0.1f, 2000.f);
        // std::cout<< "glViewportcal2 " << win.width << " "<< win.height<<std::endl;
        glm::mat4 Oproj = glm::ortho(0.0f, float(win.width), 0.0f, float(win.height), -1.f, 1.f);
        light.asp = asp;
        light.update();
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(win.window, &heightmap, &camera, deltaTime);

        glClearColor(0.1f, 0.1f, 0.15f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // glViewport(0, 0, win.width, win.height);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderMain.use();
        // Передача depth карты

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -10.0f));
        shaderMain.setVec3("cameraPos", camera.cameraPos);
        shaderMain.setVec3("viewPos", camera.cameraPos);
        shaderMain.setVec3("lightPos", light.lightPos);
        shaderMain.setVec3("objectColor", glm::vec3(0.4f, 0.8f, 0.4f));
        shaderMain.setMat4("view", view);
        shaderMain.setMat4("lightSpaceMatrix", light.lightSpaceMatrix);
        shaderMain.setMat4("projection", projection);
        shaderMain.setMat4("model", model);
        glBindVertexArray(cubev.cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        model = termodel;
        shaderMain.setVec3("objectColor", glm::vec3(0.4f, 0.8f, 0.4f));
        shaderMain.setMat4("model", model);
        glBindVertexArray(terrainVAO.terrainVAO);
        glDrawElements(GL_TRIANGLES, terrainVertices.terrainIndices.size(), GL_UNSIGNED_INT, 0);
        // glBindVertexArray(0);

        // glBindVertexArray(0);

        RenderText(&shaderText, &textV, &charV, "TESTFSDFSDFSDF", 10, 100, 0.25f, glm::vec3(1.0f, 1.0f, 1.0f), Oproj);
        glBindVertexArray(0);

        glfwSwapBuffers(win.window);
        glfwPollEvents();
    }
    cleanupWindow(&win);
    return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
bool escPressedLastFrame = false;
bool TabPressedLastFrame = false;
void processInput(GLFWwindow *window, Heightmap *heightmap, Camera *camera, float deltaTime)
{
    // Обработка ESC
    int escState = glfwGetKey(window, GLFW_KEY_ESCAPE);
    int TabState = glfwGetKey(window, GLFW_KEY_TAB);
    if (escState == GLFW_PRESS && !escPressedLastFrame)
    {
        // Первое нажатие
        escPressedLastFrame = true;
        if (mousetoogle)
        {
            //
            mousetoogle = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else
        {
            // Закрыть окно
            glfwSetWindowShouldClose(window, true);
        }
    }
    else if (escState == GLFW_RELEASE)
    {
        // Освободить флаг
        escPressedLastFrame = false;
    }
    float cameraSpeed = static_cast<float>(20.5 * deltaTime);
    float maxSlopeDegree = 46.0f;
    float maxSlopeRad = glm::radians(maxSlopeDegree);

    float angle = acos(glm::dot(computeNormal(heightmap, camera->cameraPos.x, camera->cameraPos.z), glm::vec3(0, 1, 0))); // угол между нормалью и вертикалью
    if (angle < maxSlopeRad)
    {
        // участок проходим
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera->cameraPos += cameraSpeed * camera->cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera->cameraPos -= cameraSpeed * camera->cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera->cameraPos -= glm::normalize(glm::cross(camera->cameraFront, camera->cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera->cameraPos += glm::normalize(glm::cross(camera->cameraFront, camera->cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera->cameraPos.y += cameraSpeed * 2;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            camera->cameraPos.y -= cameraSpeed;
    }
    else
    {
        camera->cameraPos -= cameraSpeed * camera->cameraFront;
    }

    //   // В функции обработки клавиш (например, в processInput):
    //   if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
    //     if(teleportationSTATUS==false)teleportationSTATUS=true;
    //   }
}

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn) //, Camera *camera,LastMouse *l
{
    if (mousetoogle)
    {
        float xpos = static_cast<float>(xposIn);
        float ypos = static_cast<float>(yposIn);

        if (firstMouse)
        {
            l.lastX = xpos;
            l.lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - l.lastX;
        float yoffset = l.lastY - ypos; // reversed since y-coordinates go from bottom to top
        l.lastX = xpos;
        l.lastY = ypos;

        float sensitivity = 0.1f; // change this value to your liking
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        camera.yaw += xoffset;
        camera.pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (camera.pitch > 89.0f)
            camera.pitch = 89.0f;
        if (camera.pitch < -89.0f)
            camera.pitch = -89.0f;

        glm::vec3 front;
        front.x = cos(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
        front.y = sin(glm::radians(camera.pitch));
        front.z = sin(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
        camera.cameraFront = glm::normalize(front);
    }
}

bool mouseButtonLastState = false;
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS && !mouseButtonLastState)
        {
            // Первое нажатие
            mouseButtonLastState = true;

            if (!mousetoogle)
            {
                //

                mousetoogle = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            else if (mousetoogle)
            {
                // double xpos, ypos;
                // glfwGetCursorPos(window, &xpos, &ypos);
                // pickObject(xpos, ypos, objects, view, projection, cameraPos);
            }
        }
        else if (action == GLFW_RELEASE)
        {
            // Освободить флаг при отпускании
            mouseButtonLastState = false;
        }
    }
}