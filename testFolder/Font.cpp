#include "Font.h"

void createTextureFont(CharacterV *characterv,std::string path,int size)

{
    if (FT_Init_FreeType(&characterv->ft))
    {
        std::cerr << "Could not init FreeType Library" << std::endl;
        // обработка ошибки
    }

    if (FT_New_Face(characterv->ft, path.c_str(), 0, &characterv->face))
    {
        std::cerr << "Failed to load font" << std::endl;
        // обработка ошибки
    }

    FT_Set_Pixel_Sizes(characterv->face, 0, size); // size of font
    // disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (unsigned char c = 0; c < 128; c++)
    {
        if (FT_Load_Char(characterv->face, c, FT_LOAD_RENDER))
        {
            std::cerr << "Failed to load glyph" << std::endl;
            continue;
        }
        // OpenGL texture from face->glyph->bitmap
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            characterv->face->glyph->bitmap.width,
            characterv->face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            characterv->face->glyph->bitmap.buffer);
        // configure options texture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // fill charcter structure
        Character character = {
            texture,
            glm::ivec2(characterv->face->glyph->bitmap.width, characterv->face->glyph->bitmap.rows),
            glm::ivec2(characterv->face->glyph->bitmap_left, characterv->face->glyph->bitmap_top),
            (GLuint)characterv->face->glyph->advance.x};
        characterv->Characters.insert(std::pair<char, Character>(c, character));
    }
}

void configTextbufs(TextV *text)
{
    // configure VAO/VBO for texture quads
    // -----------------------------------
    glGenVertexArrays(1, &text->VAOtext);
    glGenBuffers(1, &text->VBOtext);
    glBindVertexArray(text->VAOtext);
    glBindBuffer(GL_ARRAY_BUFFER, text->VBOtext);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void RenderText(Shader *shader, TextV *textV, CharacterV *characterv, std::string text, float x, float y, float scale, glm::vec3 color, glm::mat4 &proj)
{
    shader->use();
    shader->setMat4("projection", proj);
    shader->setVec3("textColor", color);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textV->VAOtext); // VAO for text

    for (char c : text)
    {
        Character ch = characterv->Characters[c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        // update vbo for every symbol
        float vertices[6][4] = {
            {xpos, ypos + h, 0.0f, 0.0f},
            {xpos, ypos, 0.0f, 1.0f},
            {xpos + w, ypos, 1.0f, 1.0f},

            {xpos, ypos + h, 0.0f, 0.0f},
            {xpos + w, ypos, 1.0f, 1.0f},
            {xpos + w, ypos + h, 1.0f, 0.0f}};

        // update VBO
        glBindBuffer(GL_ARRAY_BUFFER, textV->VBOtext);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        // draw glyph
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // update pos for next glyph
        x += (ch.Advance >> 6) * scale; // offset to next glyph
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}