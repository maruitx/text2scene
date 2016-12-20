#include "GLWidget.h"
#include "Shader.h"
#include "Camera.h"
#include "Renderer.h"
#include "Scene.h"
#include "GUI.h"
#include "CameraManager.h"
#include "Light.h"
#include "TextDialog.h"

GLWidget::GLWidget(QGLContext *context, int width, int height)
: QGLWidget(context),
  m_leftButton(false),
  m_rightButton(false),
  m_width(width),
  m_height(height),
  m_ctrlPressed(false),
  m_altPressed(false),
  m_shiftPressed(false),
  m_noOtherKey(true),
  m_renderOffline(false),
  m_oldTime(0),
  m_frameNr(0)
{
    this->resize(m_width, m_height);
    this->setMouseTracking(true);
}

GLWidget::~GLWidget()
{
	delete m_scene;
	delete m_cameraManager;
	delete m_gui;
	delete m_renderer;
}

void GLWidget::initializeGL() 
{
    glewInit();    

	initParams();
    initShaders();

	m_cameraManager = new CameraManager();
    m_scene = new Scene(m_cameraManager);
	m_gui = new GUI(m_cameraManager, m_scene);
	m_renderer = new Renderer(m_scene, m_cameraManager, m_gui);  

	m_textDialog = new TextDialog(this, m_scene);

    glEnable(GL_DEPTH_TEST);     
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);    

    glEnable(GL_SAMPLE_SHADING);
    glMinSampleShading(params::inst()->sampleShading);
}

void GLWidget::initParams()
{
    GlobalObjectParams *p = params::inst();

    p->camPos              = vec3(0.0f, 0.0f, 0.0f);
	p->blur                = vec2(2.0f, 2.0f);
    p->shadowMapSize       = vec2(1024, 1024);
    p->applyShadow         = true;
    p->gridRenderMode      = 2;
    p->shadowIntensity     = 0.4f;
    p->sampleShading       = 1.0f;
    p->polygonMode         = 0;
    p->activeLight         = 0;
    p->applyCulling        = false;
    
    p->windowSize          = vec2(m_width, m_height);
    p->previewSize         = vec2(240, 180);
    p->bufferSize          = vec2(0, 0);
    p->fboSamples          = 4;
    
    p->renderMesh          = true;
    p->renderObjects       = true;
    p->renderTextures      = false;
    p->renderWireframe     = false;
    p->renderNormals       = false;
    p->renderMisc          = false;
	p->sceneDistances      = false;
	p->renderObjectBB      = false;

    p->clipPlaneGround     = vec4(0.0f, -1.0f, 0.0f, 4.0f);
    p->ncp                 = 0.0f;
    p->fcp                 = 0.0f;
    p->fov                 = 0.0f;
    p->lightIntensity      = 1.0f;
    
    p->polygonOffsetUnits  = 0.0f;
    p->polygonOffsetFactor = 0.0f;
    p->depthRangeMax       = 1.0f;
    p->depthRangeMin       = 0.0f;
    
    p->nrVertices          = 0;
    p->nrActiveVertices    = 0;

	p->modelDirectory      = "";
	p->textureDirectory    = "";
	p->sceneDirectory      = "";
}

void GLWidget::initShaders()
{
    shaders::inst()->default = new Shader("Shader/Default.vert.glsl", "Shader/Default.frag.glsl");
    shaders::inst()->default->bindAttribLocations();

    shaders::inst()->defaultLight = new Shader("Shader/DefaultLight.vert.glsl", "Shader/DefaultLight.frag.glsl");
    shaders::inst()->defaultLight->bindAttribLocations();

    shaders::inst()->defaultDepth = new Shader("Shader/Default.vert.glsl", "Shader/DefaultDepth.frag.glsl");
    shaders::inst()->defaultDepth->bindAttribLocations();

    shaders::inst()->blur = new Shader("Shader/Blur.vert.glsl", "Shader/Blur.frag.glsl");
    shaders::inst()->blur->bindAttribLocations();

    shaders::inst()->grid = new Shader("Shader/NiceGrid.vert.glsl", "Shader/NiceGrid.frag.glsl");
    shaders::inst()->grid->bindAttribLocations();

    shaders::inst()->object = new Shader("Shader/Object.vert.glsl", "Shader/Object.frag.glsl");
    shaders::inst()->object->bindAttribLocations();

    shaders::inst()->objectDepth = new Shader("Shader/ObjectDepth.vert.glsl", "Shader/ObjectDepth.frag.glsl");
    shaders::inst()->objectDepth->bindAttribLocations();

    shaders::inst()->objectLines = new Shader("Shader/ObjectLines.vert.glsl", "Shader/ObjectLines.frag.glsl");
    shaders::inst()->objectLines->bindAttribLocations();

	shaders::inst()->gui = new Shader("Shader/GUI.vert.glsl", "Shader/GUI.frag.glsl");
	shaders::inst()->gui->bindAttribLocations();

	shaders::inst()->noise = new Shader("Shader/Noise.vert.glsl", "Shader/Noise.frag.glsl");
	shaders::inst()->noise->bindAttribLocations();

	shaders::inst()->cookTorrance = new Shader("Shader/CookTorrance.vert.glsl", "Shader/CookTorrance.frag.glsl");
	shaders::inst()->cookTorrance->bindAttribLocations();

	shaders::inst()->sphericalHarmonic = new Shader("Shader/SphericalHarmonic.vert.glsl", "Shader/SphericalHarmonic.frag.glsl");
	shaders::inst()->sphericalHarmonic->bindAttribLocations();

    shaders::inst()->tessellation = new Shader();
    shaders::inst()->tessellation->attachVertexShader("Shader/TessInterp.vert.glsl");
    shaders::inst()->tessellation->attachControlShader("Shader/TessInterp.cont.glsl");
    shaders::inst()->tessellation->attachEvaluationShader("Shader/TessInterp.eval.glsl");
    shaders::inst()->tessellation->attachGeometryShader("Shader/TessInterp.geom.glsl");
    shaders::inst()->tessellation->attachFragmentShader("Shader/TessInterp.frag.glsl");
    shaders::inst()->tessellation->bindAttribLocations();

	shaders::inst()->difference = new Shader("Shader/Difference.vert.glsl", "Shader/Difference.frag.glsl");
	shaders::inst()->difference->bindAttribLocations();

	shaders::inst()->model = new Shader("Shader/Model.vert.glsl", "Shader/Model.frag.glsl");
	shaders::inst()->model->bindAttribLocations();

	shaders::inst()->modelDepth = new Shader("Shader/Model.vert.glsl", "Shader/ModelDepth.frag.glsl");
	shaders::inst()->modelDepth->bindAttribLocations();

}

void GLWidget::paintGL()
{   
    params::inst()->lights = m_scene->m_lights;
    params::inst()->nrActiveVertices = 0;

    m_renderer->render(m_trans);

    if (m_renderOffline)
    {
        saveFrameBuffer(this, m_frameNr);
        m_frameNr++;
    }

    DWORD time = GetTickCount();
    float delta = time - m_oldTime;
    if(delta > 25)
    {
        m_scene->update(1.0f/delta);
        m_oldTime = time;

        m_cameraManager->currentPerspective(m_trans);
        m_cameraManager->currentCamParams();
    }

    this->update();
}

void GLWidget::resizeGL(int w, int h)
{
	m_width = w;
	m_height = h;

    params::inst()->windowSize = vec2(this->width(), this->height());
    
    float px = params::inst()->previewSize.x;
    float bx = w - px - 10;
    float by = h;

    float a = bx / h;
    float py = px / a;

    params::inst()->previewSize = vec2(px, py);
    params::inst()->bufferSize = vec2(bx, by);

	m_renderer->resize(w, h);
	m_cameraManager->resize(bx, by);
    m_gui->resize(w, h);
}

void GLWidget::wheelEvent(QWheelEvent *event)
{
    if (!m_altPressed && !m_ctrlPressed && !m_rightButton && !m_shiftPressed && event->x() < m_width - (params::inst()->previewSize.x+10))
    {
	    m_cameraManager->onMouseWheel(event->delta());
    }

    if(m_altPressed)
    {
        vec4 lPos = m_scene->m_lights[params::inst()->activeLight]->position();
	    float delta = lPos.y * 0.1;

        if (event->delta() > 0) 
			m_scene->m_lights[params::inst()->activeLight]->setPosition(vec3(lPos.x, lPos.y+delta, lPos.z));
        else 
            m_scene->m_lights[params::inst()->activeLight]->setPosition(vec3(lPos.x, lPos.y-delta, lPos.z));
    }

	if (m_rightButton) 
	{
		if (event->delta() > 0)
			m_cameraManager->changeRotHeight(-1.0f);
		else
			m_cameraManager->changeRotHeight(1.0f);
	}

    if (event->x() > m_width - (params::inst()->previewSize.x + 10) )
        m_renderer->onMouseWheel(event->delta() > 0 ? 40 : -40);

    event->accept();    
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    QPoint newMouse(event->x(), event->y());
	QPoint diff = newMouse - m_mouse;

    m_gui->onMouseMove(event->x(), event->x());

    if(!m_altPressed && !m_ctrlPressed)
    {
	    if(m_leftButton)
	    {            		
		    m_cameraManager->onMouseMove(diff.x(), diff.y(), 0);
	    }
	     else if(m_rightButton)
	    {
		    m_cameraManager->onMouseMove(diff.x(), diff.y(), 1);
	    }
    }

    if(m_leftButton && m_altPressed)
    {
        if(m_ctrlPressed)
            m_scene->m_lights[params::inst()->activeLight]->recordPath(true);
        else
            m_scene->m_lights[params::inst()->activeLight]->recordPath(false);

		m_scene->m_lights[params::inst()->activeLight]->move(m_cameraManager, diff.x()*0.1, diff.y()*0.1);
    }

    if(m_leftButton && m_ctrlPressed)
    {
        m_scene->move(m_trans, diff.x(), diff.y());
    }

    m_mouse.setX(event->x());
    m_mouse.setY(event->y());    
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    if(!m_gui->onMouseClick(event->x(), event->y()))
    {
        if(event->button() == Qt::LeftButton)
            m_leftButton = true; 

        if(event->button() == Qt::RightButton)
            m_rightButton = true;
    }

    m_renderer->onMouseClick(event->x(), event->y());

    if(m_ctrlPressed)
    {
        m_scene->resetSelection();
        m_scene->select(m_trans, this->width(), this->height(), event->x(), event->y());
    }
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_gui->onMouseRelease();

    if(event->button() == Qt::LeftButton)
        m_leftButton = false;   

    if(event->button() == Qt::RightButton)
        m_rightButton = false;
}

void GLWidget::keyPressEvent(QKeyEvent *event)
{   
	m_cameraManager->onKeyPress(event->key());

    switch (event->key()) 
    {
		case Qt::Key_PageUp: 
			m_cameraManager->increaseSpeed();
            break;
        case Qt::Key_PageDown:
			m_cameraManager->decreaseSpeed();
            break;
        case Qt::Key_Left:
            break;
        case Qt::Key_Right:
            break;
		case Qt::Key_Space:			
			m_textDialog->toggleShow(this->pos().x(), this->pos().y());
			break;
        case Qt::Key_Plus:
            break;
        case Qt::Key_Minus:
            break;
        case Qt::Key_Enter:
        case Qt::Key_Return:
            break;


        case Qt::Key_A:
            //Camera Strafe Left
            break;
        case Qt::Key_B:
            break;
        case Qt::Key_C:
            m_cameraManager->lockCurCamera();
            break;
        case Qt::Key_D:
            //Camera Strafe Right
            break;
        case Qt::Key_E:
            break;
        case Qt::Key_F:
            break;
        case Qt::Key_G:
            break;
        case Qt::Key_H:
            break;
        case Qt::Key_I:   
            m_cameraManager->toggleInterpolation();
            break;
        case Qt::Key_J:
            break;
        case Qt::Key_K:
            loop(params::inst()->activeLight, 0, (int)params::inst()->lights.size()-1);
            break;
        case Qt::Key_L:
            m_scene->m_lights[params::inst()->activeLight]->toggleMode();
            break;
        case Qt::Key_M:
			loop(params::inst()->polygonMode, 0, 1, 1);
            break;
        case Qt::Key_N:
            stats.print();
            break;
        case Qt::Key_O:
            break;
        case Qt::Key_P:
            loop(params::inst()->gridRenderMode, 0, 4);
            break;
        case Qt::Key_Q:
            break;
        case Qt::Key_U:
            params::inst()->applyShadow = !params::inst()->applyShadow;
            break;
        case Qt::Key_R:
            break;
        case Qt::Key_S:
            //Camera Back
            break;
        case Qt::Key_T:
            break;
        case Qt::Key_V:
            break;
        case Qt::Key_W:
            //Camera Forward
            break;
        case Qt::Key_X:
            break;
        case Qt::Key_Y:
            break;
        case Qt::Key_Z:
            break;

        case Qt::Key_F1:
            this->setWindowState(this->windowState() ^ Qt::WindowFullScreen); 
            break;
        case Qt::Key_F2:
			m_cameraManager->toggleCam();
            break;        
        case Qt::Key_F3:            
            saveFrameBuffer(this);
            break;        
        case Qt::Key_F4:
			m_renderer->toggleBGColor();
            break;
        case Qt::Key_F5:
            if(m_noOtherKey)
			    m_gui->toggleMode();

            if(m_ctrlPressed)
                m_cameraManager->toggleFrameset();
            break;
       case Qt::Key_F6:			
            if(m_ctrlPressed)
                m_cameraManager->addFrame();
            break;
       case Qt::Key_F7:			
            if(m_ctrlPressed)
                m_cameraManager->clearFrameset();
            break;
       case Qt::Key_F8:			
            if(m_ctrlPressed)
                m_cameraManager->saveFrameset();
            break;
       case Qt::Key_F9:
            m_renderOffline = !m_renderOffline;
            m_frameNr = 0;
            break;
       case Qt::Key_F10:			
            break;
       case Qt::Key_F11:			
            break;
       case Qt::Key_F12:			
            break;

		case Qt::Key_Control:
			m_ctrlPressed = true;
            m_noOtherKey = false;
			break;
		case Qt::Key_Alt:
			m_altPressed = true;
            m_noOtherKey = false;
			break;
		case Qt::Key_Shift:
			m_shiftPressed = true;
            m_noOtherKey = false;
			break;
        case Qt::Key_Escape:            
            exit(0);
            break;
    }
}

void GLWidget::keyReleaseEvent(QKeyEvent *event)
{
	m_cameraManager->onKeyRelease(event->key());

    switch (event->key()) 
    {
		case Qt::Key_Control:
			m_ctrlPressed = false;
            m_noOtherKey = true;
			break;
		case Qt::Key_Alt:
			m_altPressed = false;
            m_noOtherKey = true;
			break;
		case Qt::Key_Shift:
			m_shiftPressed = false;
            m_noOtherKey = true;
			break;
		default:
            break;
    }
}
