#ifndef FONT_HPP
#define FONT_HPP

#include <map>
#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Shader.h"
#include <ft2build.h>
#include FT_FREETYPE_H

struct Character
{
    GLuint TextureID;   // ID texture glyph
    glm::ivec2 Size;    // size glyph
    glm::ivec2 Bearing; // offset
    GLuint Advance;     // step

};

struct CharacterV
{
    FT_Library ft;
    FT_Face face;
    std::map<char, Character> Characters;
    ~CharacterV(){
        for(auto& a : Characters){
            glDeleteTextures(1,&a.second.TextureID);
        }
    }
};

struct TextV
{
    GLuint VAOtext, VBOtext;
    ~TextV()
    {
        glDeleteBuffers(1, &VAOtext);
        glDeleteVertexArrays(1, &VBOtext);
    }
};

void createTextureFont(CharacterV *characterv,std::string path,int size);

void configTextbufs(TextV *text);

void RenderText(Shader *shader, TextV *textV, CharacterV *characterv, std::string text, float x, float y, float scale, glm::vec3 color, glm::mat4 &proj);


#endif // FONT_HPP