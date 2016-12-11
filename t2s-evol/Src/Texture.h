#ifndef TEXTURE_H
#define TEXTURE_H

#include "Headers.h"

class Texture
{

public:
   Texture();
   Texture(const Texture &);   
   Texture(GLsizei w, GLsizei h, GLint iFormat, GLint format, GLint type);
   Texture(GLsizei w, GLsizei h, GLint iFormat, GLint format, GLint type, GLvoid *data);
   Texture(const QImage &img);
   Texture(const QImage &img, GLint magFilter, GLint minFilter, GLfloat anisotrophy, GLboolean createMipmaps);
   Texture(QString path);
   Texture(const QFileInfoList &infoList);
   ~Texture();

   Texture &operator = (const Texture &t);

   void bind();
   void release();
   void create(GLvoid *data = nullptr);

   void setWrapMode(GLint wrap = GL_REPEAT);
   void setEnvMode(GLint envMode = GL_REPLACE);
   void setFilter(GLint minFilter = GL_NEAREST, GLint magFilter = GL_NEAREST);
   void setMaxIsotropy(GLfloat anisotropy = 1.0f);

   void deleteTex();
   void load(QString fileName);

   void render(GLuint posX, GLuint posY, GLfloat width, GLfloat height);

   GLuint id() const;
   GLsizei width() const;
   GLsizei height() const;
   GLenum target() const;
   GLint mipLevel() const;
   GLint internalFormat() const;
   GLenum format() const;
   GLint border() const;
   GLenum type() const;
   GLint minFilter() const;
   GLint magFilter() const;
   GLint wrap() const;
   GLint envMode() const;
   GLfloat maxAnisotropy() const;
   GLboolean createMipMaps() const;
   GLboolean manualMipMaps() const;
   GLboolean isReady() const;

   void createManualMipMaps(QImage img);
   void loadManualMipMaps(QString fileName);

private:
    //GLuint  m_id;
    GLsizei m_width;
    GLsizei m_height;
    GLenum  m_target;
    GLint   m_mipLevel;
    GLint   m_internalFormat;
    GLenum  m_format;
    GLint   m_border;
    GLenum  m_type;
    GLint   m_minFilter;
    GLint   m_magFilter;
    GLint   m_wrap;
    GLint   m_envMode;
    GLboolean m_createMipMaps;
    GLfloat m_maxAnisotropy;
    GLboolean m_manualMipMaps;
    GLboolean m_isReady;
    GLuint m_id;

    std::vector<QImage> m_manualMM;
};

#endif

