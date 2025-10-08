#include "CubeV.h"

void createVAOVBObufs(CubeV *cube)
{
  glGenVertexArrays(1,&cube->cubeVAO);
  glGenBuffers(1,&cube->cubeVBO);
  glBindVertexArray(cube->cubeVAO);
  glBindBuffer(GL_ARRAY_BUFFER,cube->cubeVBO);
  glBufferData(GL_ARRAY_BUFFER,cube->vertices.size()*sizeof(float),cube->vertices.data(),GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),(void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
  glBindVertexArray(0);
}