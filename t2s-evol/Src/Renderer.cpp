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
  m_fboDifference(nullptr),
  m_prevYOffset(0), 
  m_vboQuad(nullptr)
{
    init();

    toggleBGColor();
    toggleBGColor();
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

	// reset to the preview after evol
	if (m_scene->m_resetPreview)
	{
		m_activePreview = m_scene->m_activeVarationId;
		Preview &p = m_previewFBOs[m_activePreview];
		m_prevYOffset = 0;
		//m_prevYOffset = p.y;
	}
    
    renderIntoMainFBO(trans);
    renderIntoPreviewFBOs(trans);

	if (params::inst()->sceneDistances)
	{
		renderSceneDifference();
	}


    glViewport(0, 0, m_width, m_height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);    
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

	//renderTexture(params::inst()->textures["88d8fa093e4c36a1"]->id(), 220, m_height - 200, 200, 200);

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
		m_scene->renderSynScene(trans, m_activePreview, true);

    m_fbo->release();
    m_fbo->blit();
}

void Renderer::renderIntoPreviewFBOs(Transform &trans)
{
	if (m_previewFBOs.size() == 0)
		return;

	int mod = m_frameCount % m_previewFBOs.size();
	FrameBufferObjectMultisample *fbo = nullptr;
	
    for(int i=0; i<m_previewFBOs.size(); ++i)
    {
       // if (mod == i)
        {
            FrameBufferObjectMultisample *fbo = m_previewFBOs[i].fbo;

            fbo->bind();

                glViewport(0, 0, params::inst()->previewSize.x, params::inst()->previewSize.y);
                glClearColor(m_bgColor.x, m_bgColor.y, m_bgColor.z, m_bgColor.w);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                m_scene->renderWorld(trans, false);
				m_scene->renderSynScene(trans, i, false);

            fbo->release();
            fbo->blitColor();
        }
    }

    m_frameCount++;
}

void Renderer::renderPreviews(Transform &trans)
{
    for(int i=0; i<m_previewFBOs.size(); ++i)
    {
        Preview &p = m_previewFBOs[i];
        renderTexturePreview(p.fbo->colorTex(), p.x, p.y + m_prevYOffset, p.w, p.h, i == m_activePreview ? true : false, p.distToCurrent);
    }
}

float Renderer::computeFBODifference(GLuint texA, GLuint texB)
{
	int w = params::inst()->previewSize.x;
	int h = params::inst()->previewSize.y;

	float sum = 0.0f;
	float *data = new float[w*h * 4];

	m_fboDifference->bind();

		glViewport(0, 0, w, h);
		glClearColor(1, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mat4 model = mat4::translate(0, 0, 0);
		mat4 view = mat4::translate(0, 0, -1);
		mat4 projection = mat4::orthographic(0, w, h, 0, -1, 1);

		Shader *shader = shaders::inst()->difference;
		shader->bind();

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texA);
			shader->seti("texA", 0);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, texB);
			shader->seti("texB", 1);

			shader->setMatrix("matModel", model, GL_TRUE);
			shader->setMatrix("matView", view, GL_TRUE);
			shader->setMatrix("matProjection", projection, GL_TRUE);

			m_vboQuad->render();

		shader->release();

		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glReadPixels(0, 0, w, h, GL_RGBA, GL_FLOAT, data);

	m_fboDifference->release();	

	int idx = 0;
	for (int y = 0; y < h; ++y)
	{
		for (int x = 0; x < w; ++x)
		{
			sum += data[idx + 0];
			idx += 4;
		}
	}

	delete[] data;

	return sum;
}

void Renderer::renderSceneDifference()
{
	Preview &p0 = m_previewFBOs[m_activePreview];

	int ma = math_minfloat;
	for (int i = 0; i < m_previewFBOs.size(); ++i)
	{
		Preview &pi = m_previewFBOs[i];
		float diff = computeFBODifference(p0.fbo->colorTex(), pi.fbo->colorTex());

		if (diff > ma)
		{
			ma = diff;
		}

		pi.distToCurrent = diff;
	}

	for (int i = 0; i < m_previewFBOs.size(); ++i)
	{
		Preview &pi = m_previewFBOs[i];

		if (ma > 0.0f)
			pi.distToCurrent /= ma;
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

	//Make difference fbo
	delete m_fboDifference;
	delete m_vboQuad;

	m_fboDifference = new FrameBufferObject(params::inst()->previewSize.x, params::inst()->previewSize.y, 1, 0, GL_FALSE);
	m_vboQuad = Mesh::quad(0, 0, params::inst()->previewSize.x, params::inst()->previewSize.y, vec4(0, 0, 1, 1));
}

void Renderer::onMouseClick(int mx, int my)
{
    for(int i=0; i<m_previewFBOs.size(); ++i)
    {
        if (m_previewFBOs[i].clicked(mx, my, m_prevYOffset))
        {
			m_scene->m_resetPreview = false;

            m_activePreview = i;
			m_scene->m_activeVarationId = m_activePreview;

            params::inst()->sceneBB = BoundingBox(vec3(math_maxfloat, math_maxfloat, math_maxfloat), vec3(math_minfloat, math_minfloat, math_minfloat));
        }
    }

    for (int i = 0; i < m_scene->m_lights.size(); ++i)
    {
        m_scene->m_lights[0]->m_moved = true;
    }
}

void Renderer::onMouseWheel(int delta)
{
    int m = params::inst()->windowSize.y / m_previewFBOs.size();
    int offY = abs(m_prevYOffset);
   
    //if (offY < m || delta > 0)
    {
        m_prevYOffset += delta;
    }    

    m_prevYOffset = min(0, m_prevYOffset);
}