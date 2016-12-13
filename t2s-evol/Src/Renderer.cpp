#include "Renderer.h"
#include "GUI.h"
#include "CameraManager.h"
#include "FrameBufferObject.h"
#include "Shader.h"
#include "VertexBufferObject.h"
#include "Scene.h"
#include "Mesh.h"
#include "Light.h"
#include "Texture.h"

Renderer::Renderer(Scene *scene, CameraManager *camManager, GUI *gui)
: m_scene(scene),
  m_gui(gui),
  m_cameraManager(camManager),
  m_bgColor(0.1f, 0.1f, 0.1f, 1.0f),
  m_width(0),
  m_height(0),
  m_samples(16),
  m_bgMode(0), 
  m_activePreview(0), 
  m_frameCount(0), 
  m_fbo(nullptr), 
  m_prevYOffset(0)
{
    init();
}

Renderer::~Renderer()
{
}

void Renderer::init()
{   
    initPreviewFBOs();
}

void Renderer::render(Transform &trans)
{
    if(params::inst()->applyShadow)
    {
        trans.lightViews.clear();
        trans.lightViews.resize(m_scene->m_lights.size());

        for(int i=0; i<m_scene->m_lights.size(); ++i)
        {
            m_scene->m_lights[i]->setIntensity(params::inst()->lightIntensity);
            m_scene->m_lights[i]->setDirection(m_scene->m_lights[i]->position());
            m_scene->m_lights[i]->renderLightView(trans.lightViews[i], m_activePreview);
        }
    }    
    
    renderIntoMainFBO(trans);
    renderIntoPreviewFBOs(trans);

    glViewport(0, 0, m_width, m_height);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     

    renderTexture(m_fbo->colorTex(), 0, 0, params::inst()->bufferSize.x, params::inst()->bufferSize.y);
    renderPreviews(trans);

    if (params::inst()->renderTextures)
    {
        for (int i = 0; i<m_scene->m_lights.size(); ++i)
        {
            renderTexture(m_scene->m_lights[i]->shadowMapId(), 220 + i * 210, m_height - 200, 200, 200);
            renderTexture(m_scene->m_lights[i]->shadowMapBlurredId(), 220 + 210 + i * 210, m_height - 200, 200, 200);
        }
    }

    m_gui->render();
}

void Renderer::renderScene(const Transform &trans)
{
    glViewport(0, 0, m_width, m_height);
    glClearColor(m_bgColor.x, m_bgColor.y, m_bgColor.z, m_bgColor.w);    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     

    glEnable(GL_MULTISAMPLE);        

    if (params::inst()->polygonMode == 1)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //glEnable(GL_POLYGON_OFFSET_FILL);
    //glEnable(GL_POLYGON_OFFSET_POINT);
    //glEnable(GL_POLYGON_OFFSET_LINE);
    //glDepthFunc(GL_LEQUAL);
    //glDepthRange(0.1, 1.0);
    //glPolygonOffset(-1.0, -10.5);
		    
    m_scene->renderObjects(trans);
    m_scene->renderWorld(trans);    

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Renderer::renderIntoMainFBO(Transform &trans)
{
    m_fbo->bind();

        glViewport(0, 0, params::inst()->bufferSize.x, params::inst()->bufferSize.y);
        glClearColor(m_bgColor.x, m_bgColor.y, m_bgColor.z, m_bgColor.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_scene->renderWorld(trans);
        m_scene->renderVariation(trans, m_activePreview);

    m_fbo->release();
    m_fbo->blit();
}

void Renderer::renderIntoPreviewFBOs(Transform &trans)
{
    for(int i=0; i<m_previewFBOs.size(); ++i)
    {
        if (m_frameCount % (i+1) == 0)
        {
            FrameBufferObjectMultisample *fbo = m_previewFBOs[i].fbo;

            fbo->bind();

                glViewport(0, 0, params::inst()->previewSize.x, params::inst()->previewSize.y);
                glClearColor(m_bgColor.x, m_bgColor.y, m_bgColor.z, m_bgColor.w);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                m_scene->renderWorld(trans, false);
                m_scene->renderVariation(trans, i, false);

            fbo->release();
            fbo->blitColor();
        }
    }

    m_frameCount++;
}

void Renderer::renderPreviews(Transform &trans)
{
    int w = params::inst()->previewSize.x;
    int h = params::inst()->previewSize.y;
    
    for(int i=0; i<m_previewFBOs.size(); ++i)
    {
        Preview &p = m_previewFBOs[i];
        renderTexture(p.fbo->colorTex(), p.x, p.y + m_prevYOffset, p.w, p.h, i == m_activePreview ? true : false);
    }
}

void Renderer::resize(int width, int height)
{
    m_width = width;
    m_height = height;    

    delete m_fbo;
    m_fbo = new FrameBufferObjectMultisample(params::inst()->bufferSize.x, params::inst()->bufferSize.y, params::inst()->fboSamples);

    initPreviewFBOs();
}

void Renderer::toggleBGColor()
{
    m_bgMode ++;
    if(m_bgMode > 2)
        m_bgMode = 0;

    if(m_bgMode == 0)
	{
        m_bgColor = vec4(0.1f, 0.1f, 0.1f, 1.0f);
		m_gui->setFontColor(vec4(0.9f, 0.9f, 0.9f, 1.0f));
	}
   
    if(m_bgMode == 1)
	{
        m_bgColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
		m_gui->setFontColor(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	}

    if(m_bgMode == 2)
	{
        m_bgColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
		m_gui->setFontColor(vec4(0.0f, 0.0f, 0.0f, 1.0f));
	}
}

void Renderer::initPreviewFBOs()
{
    int sx = 5;
    int sy = 5;

    int nrVariations = m_scene->m_variations.size();

    //Delete FBOs
    for(int i=m_previewFBOs.size()-1; i >= 0; --i)
    {
        FrameBufferObjectMultisample *fbo = m_previewFBOs[i].fbo;
        delete fbo;
    }
    m_previewFBOs.clear();

    //Make new FBOs
    int w = params::inst()->previewSize.x;
    int h = params::inst()->previewSize.y;
    
    int startX = m_width - w - sx;
    int startY = sy;

    for(int i=0; i<nrVariations; ++i)
    {
        FrameBufferObjectMultisample *fbo = new FrameBufferObjectMultisample(params::inst()->previewSize.x, params::inst()->previewSize.y, params::inst()->fboSamples);
        
        int x = m_width-w-sx;
        int y = i*(sy+h);

        Preview p;

        p.fbo = fbo;
        p.x = x;
        p.y = y;
        p.w = w;
        p.h = h;
        
        m_previewFBOs.push_back(p);
    }
}

void Renderer::onMouseClick(int mx, int my)
{
    for(int i=0; i<m_previewFBOs.size(); ++i)
    {
        if (m_previewFBOs[i].clicked(mx, my, m_prevYOffset))
        {
            m_activePreview = i;
        }
    }
}

void Renderer::onMouseWheel(int delta)
{
    int m = params::inst()->windowSize.y / m_previewFBOs.size();
    int offY = abs(m_prevYOffset);
   
    if (offY < m || delta > 0)
    {
        m_prevYOffset += delta;
    }    

    m_prevYOffset = min(0, m_prevYOffset);
}