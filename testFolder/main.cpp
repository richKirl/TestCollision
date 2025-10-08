#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Window.h"
#include "Font.h"
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
int main()
{
    // first step init
    // init window
    Window win;

    // init glfw and glew
    initWindow(&win, 800, 600, "Test Scene");


    Shader shaderText("shaders/vText.glsl","shaders/fText.glsl");
    TextV textV;
    CharacterV charV;
    createTextureFont(&charV,"DejaVuSansMono.ttf",48);
    configTextbufs(&textV);


    // main cycle
    while (!glfwWindowShouldClose(win.window))
    {

        glm::mat4 Oproj = glm::ortho(0.0f,win.width, 0.0f,win.height,-1.f,1.f);

        glClearColor(0.1f, 0.1f, 0.15f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        RenderText(&shaderText,&textV,&charV,"TESTFSDFSDFSDF",10, 100,1, glm::vec3(1.0f,1.0f,1.0f), Oproj);


        glfwSwapBuffers(win.window);
        glfwPollEvents();

    }
    cleanupWindow(&win);
    return 0;
}
