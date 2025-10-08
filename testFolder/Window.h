#ifndef WINDOW_HPP
#define WINDOW_HPP
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
struct Window
{
    GLFWwindow *window;
    std::string title;
    float width, height;
};

void initWindow(Window *window, int width, int height, std::string title);
void cleanupWindow(Window *window);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

#endif // WINDOW_HPP