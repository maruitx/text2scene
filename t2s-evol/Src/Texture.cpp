#include "Texture.h"
#include <QImage>

//Default
Texture::Texture()
: m_id(0),
  m_width(0),
  m_height(0),
  m_target(GL_TEXTURE_2D),
  m_mipLevel(0),
  m_internalFormat(GL_RGBA),
  m_format(GL_RGBA),
  m_border(0),
  m_type(GL_UNSIGNED_BYTE),
  m_minFilter(GL_NEAREST),
  m_magFilter(GL_NEAREST),
  m_wrap(GL_CLAMP),
  m_envMode(GL_REPLACE),
  m_createMipMaps(GL_FALSE),
  m_maxAnisotropy(1.0f),
  m_manualMipMaps(GL_FALSE), 
  m_isReady(GL_FALSE)
{
}

//Create empty texture
Texture::Texture(GLsizei w, GLsizei h, GLint iFormat, GLint format, GLint type)
: m_id(0),
  m_width(w),
  m_height(h),
  m_target(GL_TEXTURE_2D),
  m_mipLevel(0),
  m_internalFormat(iFormat),
  m_format(format),
  m_border(0),
  m_type(type),
  m_minFilter(GL_NEAREST),
  m_magFilter(GL_NEAREST),
  m_wrap(GL_CLAMP),
  m_envMode(GL_REPLACE),
  m_createMipMaps(GL_FALSE),
  m_maxAnisotropy(1.0f),
  m_manualMipMaps(GL_FALSE), 
  m_isReady(GL_FALSE)
{
    create(nullptr);
}

//Pass with Data Pointer
Texture::Texture(GLsizei w, GLsizei h, GLint iFormat, GLint format, GLint type, GLvoid *data)
: m_id(0),
  m_width(w),
  m_height(h),
  m_target(GL_TEXTURE_2D),
  m_mipLevel(0),
  m_internalFormat(iFormat),
  m_format(format),
  m_border(0),
  m_type(type),
  m_minFilter(GL_NEAREST),
  m_magFilter(GL_NEAREST),
  m_wrap(GL_CLAMP),
  m_envMode(GL_REPLACE),
  m_createMipMaps(GL_TRUE),
  m_maxAnisotropy(0.0f),
  m_manualMipMaps(GL_FALSE), 
  m_isReady(GL_FALSE)
{
    create(data);
}

//Load from QImage
Texture::Texture(const QImage &img)
: m_id(0),
  m_width(0),
  m_height(0),
  m_target(GL_TEXTURE_2D),
  m_mipLevel(0),
  m_internalFormat(GL_RGBA),
  m_format(GL_BGRA),
  m_border(0),
  m_type(GL_UNSIGNED_BYTE),
  m_minFilter(GL_LINEAR_MIPMAP_LINEAR),
  m_magFilter(GL_LINEAR),
  m_wrap(GL_CLAMP),
  m_envMode(GL_REPLACE),
  m_createMipMaps(GL_TRUE),
  m_maxAnisotropy(16.0f),
  m_manualMipMaps(GL_FALSE), 
  m_isReady(GL_FALSE)
{
    //m_data = (GLvoid *)img.bits();
    m_width = (GLuint)img.width();
    m_height = (GLuint)img.height();

    //m_manualMipMaps = GL_TRUE;
    //createManualMipMaps(img);

    create((GLvoid *)img.bits());
}

Texture::Texture(const QImage &img, GLint magFilter, GLint minFilter, GLfloat anisotrophy, GLboolean createMipmaps)
: m_id(0),
  m_width(0),
  m_height(0),
  m_target(GL_TEXTURE_2D),
  m_mipLevel(0),
  m_internalFormat(GL_RGBA),
  m_format(GL_BGRA),
  m_border(0),
  m_type(GL_UNSIGNED_BYTE),
  m_minFilter(minFilter),
  m_magFilter(magFilter),
  m_wrap(GL_CLAMP),
  m_envMode(GL_REPLACE),
  m_createMipMaps(createMipmaps),
  m_maxAnisotropy(anisotrophy),
  m_manualMipMaps(GL_FALSE), 
  m_isReady(GL_FALSE)
{
    //m_data = (GLvoid *)img.bits();
    m_width = (GLuint)img.width();
    m_height = (GLuint)img.height();

    create((GLvoid *)img.bits());
}

//Load from FileInfoList
Texture::Texture(const QFileInfoList &infoList)
: m_id(0),
  m_width(0),
  m_height(0),
  m_target(GL_TEXTURE_2D),
  m_mipLevel(0),
  m_internalFormat(GL_RGBA),
  m_format(GL_RGBA),
  m_border(0),
  m_type(GL_UNSIGNED_BYTE),
  m_minFilter(GL_LINEAR_MIPMAP_LINEAR),
  m_magFilter(GL_LINEAR),
  m_wrap(GL_CLAMP),
  m_envMode(GL_REPLACE),
  m_createMipMaps(GL_TRUE),
  m_maxAnisotropy(16.0f),
  m_manualMipMaps(GL_FALSE), 
  m_isReady(GL_FALSE)
{
    for(int i=0; i<infoList.size(); ++i)
    {
        QFileInfo fi = infoList[i];
        QString filePath = fi.absoluteFilePath();

        loadManualMipMaps(filePath);
    }

    m_manualMipMaps = GL_TRUE;

    create();
}

Texture::Texture(QString path)
: m_id(0),
  m_width(0),
  m_height(0),
  m_target(GL_TEXTURE_2D),
  m_mipLevel(0),
  m_internalFormat(GL_RGBA),
  m_format(GL_BGRA),
  m_border(0),
  m_type(GL_UNSIGNED_BYTE),
  m_minFilter(GL_LINEAR_MIPMAP_LINEAR),
  m_magFilter(GL_LINEAR),
  m_wrap(GL_CLAMP),
  m_envMode(GL_REPLACE),
  m_createMipMaps(GL_TRUE),
  m_maxAnisotropy(16.0f),
  m_manualMipMaps(GL_FALSE), 
  m_isReady(GL_FALSE)
{
    QImage img(path);    

    m_width = (GLuint)img.width();
    m_height = (GLuint)img.height();  

    create((GLvoid *)img.bits());    
}

Texture::Texture(const Texture &t)
{
  m_id = t.id();
  m_width = t.width();
  m_height = t.height();
  m_target = t.target();
  m_mipLevel = t.mipLevel();
  m_internalFormat = t.internalFormat();
  m_format = t.format();
  m_border = t.border();
  m_type = t.type();
  m_minFilter = t.minFilter();
  m_magFilter = t.magFilter();
  m_wrap = t.wrap();
  m_envMode = t.envMode();
  m_createMipMaps = t.createMipMaps();
  m_maxAnisotropy = t.maxAnisotropy();
  m_manualMipMaps = t.manualMipMaps();
  m_isReady = t.isReady();
}

Texture &Texture::operator = (const Texture &t)
{
  m_id = t.id();
  m_width = t.width();
  m_height = t.height();
  m_target = t.target();
  m_mipLevel = t.mipLevel();
  m_internalFormat = t.internalFormat();
  m_format = t.format();
  m_border = t.border();
  m_type = t.type();
  m_minFilter = t.minFilter();
  m_magFilter = t.magFilter();
  m_wrap = t.wrap();
  m_envMode = t.envMode();
  m_createMipMaps = t.createMipMaps();
  m_maxAnisotropy = t.maxAnisotropy();
  m_manualMipMaps = t.manualMipMaps();
  m_isReady = t.isReady();

  return *this;
}

Texture::~Texture()
{
    deleteTex();
}

void Texture::load(QString fileName)
{
    QImage img(fileName);    

    m_width = (GLuint)img.width();
    m_height = (GLuint)img.height();  

    create((GLvoid *)img.bits());    
}

void Texture::create(GLvoid *data)
{    
    glGenTextures(1, &m_id);	
    glBindTexture(m_target, m_id);    

    if(m_createMipMaps)
    {           
        if(m_manualMipMaps)
        {
            for(uint i=0; i<m_manualMM.size(); ++i)
            {
                QImage img = m_manualMM[i];            
                glTexImage2D(m_target, i, m_internalFormat, img.width(), img.height(), m_border, m_format, m_type, img.bits());            
            }

            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, m_manualMM.size() - 1);
        }
        else
        {
            glTexParameteri(m_target, GL_GENERATE_MIPMAP, GL_TRUE); 

            glTexImage2D(m_target, m_mipLevel, m_internalFormat, m_width, m_height, m_border, m_format, m_type, data);     
        }
    }
    else
    {
        glTexImage2D(m_target, 0, m_internalFormat, m_width, m_height, m_border, m_format, m_type, data);     
    }

    glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, m_minFilter);
    glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, m_magFilter); 

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, m_envMode);

    glTexParameterf(m_target, GL_TEXTURE_WRAP_S, m_wrap);
    glTexParameterf(m_target, GL_TEXTURE_WRAP_T, m_wrap);
    glTexParameterf(m_target, GL_TEXTURE_WRAP_R, m_wrap);

    glTexParameterf(m_target, GL_TEXTURE_MAX_ANISOTROPY_EXT, m_maxAnisotropy);

    if(data)
    {
        m_isReady = true;
    }
}

void Texture::loadManualMipMaps(QString fileName)
{
    QImage img(fileName);
    img = img.rgbSwapped();
    
    int w = img.width();
    int h = img.height();

    img = img.scaled(w, h, Qt::IgnoreAspectRatio, Qt::FastTransformation);

    //for(int x = 0; x<w; ++x)
    //{
    //    for(int y=0; y<h; ++y)
    //    {
    //        QRgb pixel = img.pixel(x, y);
    //        int alpha = qAlpha(pixel);
    //        float falpha = (float)alpha / 255.0f;

    //        if(falpha < 1.0f)
    //            falpha = 0.0f;


    //        alpha = int(falpha * 255.0f);

    //        img.setPixel(x, y, qRgba(qRed(pixel), qGreen(pixel), qBlue(pixel), alpha));
    //    }
    //}

    m_manualMM.push_back(img);
}

void Texture::createManualMipMaps(QImage img)
{
    float nrLevels = 8;
    QImage prev = img;

    for(int i=0; i<nrLevels; ++i)
    {
        int w = prev.width();
        int h = prev.height();

        QImage a = prev.scaled(w/2, h/2, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        if(a.width() > 1 && a.height() > 1)
            m_manualMM.push_back(a);
        else
            std::cout << "TEXTURE::createManualMipMaps: MipMap too small." << std::endl;

        prev = a;
    }
    
}

void Texture::bind()
{
    glBindTexture(m_target, m_id);
}

void Texture::release()
{
    glBindTexture(m_target, 0);
}

void Texture::deleteTex()
{
	if (m_id != 0)
	{
		glDeleteTextures(1, &m_id);	
	}
}

GLuint Texture::id() const
{        
    return m_id;
}

void Texture::setWrapMode(GLint wrap)
{
    m_wrap = wrap;

    bind();

    glTexParameterf(m_target, GL_TEXTURE_WRAP_S, m_wrap);
    glTexParameterf(m_target, GL_TEXTURE_WRAP_T, m_wrap);
    glTexParameterf(m_target, GL_TEXTURE_WRAP_R, m_wrap);

    release();
}

void Texture::setEnvMode(GLint envMode)
{
    m_envMode = envMode;

    bind();

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, m_envMode);

    release();
}

void Texture::setFilter(GLint minFilter, GLint magFilter)
{
    m_minFilter = minFilter;
    m_magFilter = magFilter; 

    bind();

    glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, m_minFilter);
    glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, m_magFilter); 

    release();
}

void Texture::setMaxIsotropy(GLfloat anisotropy)
{
    m_maxAnisotropy = anisotropy;

    bind();

    glTexParameterf(m_target, GL_TEXTURE_MAX_ANISOTROPY_EXT, m_maxAnisotropy);  

    release();
}

void Texture::render(GLuint posX, GLuint posY, GLfloat width, GLfloat height)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glColor4f(0.0, 0.0, 0.0, 1.0f);  
    
    glEnable(GL_TEXTURE_2D);     
    glActiveTexture(GL_TEXTURE0);
    
    bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    glDisable(GL_DEPTH_TEST);      
    glEnable2D();
    glPushMatrix();            
        glTranslatef(posX, posY, 0.0f);       
        glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(0.0f, 0.0f, 0.0f);

            glTexCoord2f(1.0f, 1.0f);
            glVertex3f(width, 0.0f, 0.0f);
            
            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(width, height, 0.0f);

            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(0.0, height, 0.0f);
        glEnd();
    glPopMatrix();
    glDisable2D();

    release();

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);  

    glPopClientAttrib();
    glPopAttrib();
}

GLsizei Texture::width() const
{
    return m_width;
}

GLsizei Texture::height() const
{
    return m_height;
}

GLenum Texture::target() const
{
    return m_target;
}

GLint Texture::mipLevel() const
{
    return m_mipLevel;
}

GLint Texture::internalFormat() const
{
    return m_internalFormat;
}

GLenum Texture::format() const
{
    return m_format;
}

GLint Texture::border() const
{
    return m_border;
}

GLenum Texture::type() const
{
    return m_type;
}

GLint Texture::minFilter() const
{
    return m_minFilter;
}

GLint Texture::magFilter() const
{
    return m_magFilter;
}

GLint Texture::wrap() const
{
    return m_wrap;
}

GLint Texture::envMode() const

{
    return m_envMode;
}

GLfloat Texture::maxAnisotropy() const
{
    return m_maxAnisotropy;
}

GLboolean Texture::createMipMaps() const
{
    return m_createMipMaps;
}

GLboolean Texture::manualMipMaps() const
{
    return m_manualMipMaps;
}

GLboolean Texture::isReady() const
{
    return m_isReady;
}