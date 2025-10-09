#include "Window.h"
#include <iostream>

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width
    // and height will be significantly larger than specified on retina displays.
    Window* data = static_cast<Window*>(glfwGetWindowUserPointer(window));
    data->width=static_cast<float>(width) ;
    data->height = static_cast<float>(height);
    glViewport(0, 0, width, height);
    //std::cout<< "glViewport " << data->width << " "<< data->height<<std::endl;
}

void initWindow(Window *window, int width, int height,const std::string title)
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        // Handle error, e.g., throw an exception or exit
        return;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_SAMPLES, 4); 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // Create the window
    window->width = width;
    window->height = height;
    window->title = title;
    window->window = glfwCreateWindow(window->width, window->height, window->title.c_str(), NULL, NULL);
    if (!window->window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate(); // Terminate GLFW if window creation fails
        // Handle error
        return;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window->window);
    glfwSetFramebufferSizeCallback(window->window, framebuffer_size_callback);
    // Set the user pointer for this window
    //for mechanism send/recieve data from pointer in callback instance
    glfwSetWindowUserPointer(window->window, &window->window);

    
    // glfwSetCursorPosCallback(window, mouse_callback);
    // glfwSetMouseButtonCallback(window, mouse_button_callback);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        // Handle error, e.g., throw an exception or exit
        return;
    }
    // configure OpenGL
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void cleanupWindow(Window *window)
{

    glfwDestroyWindow(window->window);
}