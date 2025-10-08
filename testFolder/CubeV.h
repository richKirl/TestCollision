#ifndef CUBEV_HPP
#define CUBEV_HPP

#include <vector>
#include <GL/glew.h>

struct CubeV
{
    GLuint cubeVAO, cubeVBO;
    std::vector<float> vertices;
    ~CubeV()
    {
        glDeleteBuffers(1, &cubeVBO);
        glDeleteVertexArrays(1, &cubeVAO);
    }
};

void createVAOVBObufs(CubeV *cube);

#endif // CUBEV_HPP