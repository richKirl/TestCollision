#ifndef UTILS_HPP
#define UTILS_HPP

// #include <GL/glew.h>
// #define STB_IMAGE_IMPLEMENTATION
// #include "stb_image.h"
// #include <iostream>
#include <glm/glm.hpp>
#include "Terrain.h"
glm::vec3 computeNormal(Heightmap *heightmap,int x, int z) {
  float heightL = getHeightAt(heightmap,x - 1, z);
  float heightR = getHeightAt(heightmap,x + 1, z);
  float heightD = getHeightAt(heightmap,x, z - 1);
  float heightU = getHeightAt(heightmap,x, z + 1);
  glm::vec3 normal = glm::normalize(glm::vec3(heightL - heightR, 2.0f, heightD - heightU));
  return normal;
}
// GLuint loadTexture(const char *filename)
// {
//     int width, height, nrChannels;
//     unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 4); // force RGBA
//     if (!data)
//     {
//         std::cerr << "Failed to load texture: " << filename << std::endl;
//         return 0;
//     }

//     GLuint textureID;
//     glGenTextures(1, &textureID);
//     glBindTexture(GL_TEXTURE_2D, textureID);

//     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
//     glGenerateMipmap(GL_TEXTURE_2D);

//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

//     stbi_image_free(data);
//     return textureID;
// }

#endif // UTILS_HPP