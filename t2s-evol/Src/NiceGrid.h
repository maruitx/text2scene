#ifndef NICEGRID_H
#define NICEGRID_H

#include "Headers.h"
#include "Mesh.h"

class Shader;
class Texture;
class VertexBufferObject;

class NiceGrid
{
public:
   NiceGrid(GLfloat size, GLfloat rep);
   ~NiceGrid();

   void render(const Transform &trans, bool applyShadow = true);

   void updatePostion(vec3 newPos) { m_position = newPos; };

private:
    void buildVBO();

private:
    GLuint  m_texID;
    GLfloat m_size;
    GLfloat m_rep;

    VertexBufferObject *m_vbo;
    Material m_material;

    vec3 m_position;

    float m_backFaceAlpha;
};

#endif

