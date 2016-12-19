#ifndef RENDERER_H
#define RENDERER_H

#include "Headers.h"

class GUI;
class CameraManager;
class FrameBufferObjectMultisample;
class FrameBufferObject;
class Shader;
class Scene;
class VertexBufferObject;
class Texture;

class Preview
{
public:
    int x;
    int y;
    int w;
    int h;

    FrameBufferObjectMultisample *fbo;

	float distToCurrent;

public:
	Preview() : x(0), y(0), w(0), h(0), fbo(nullptr), distToCurrent(0.0f) {}

    bool clicked(int mx, int my, int offset = 0) 
    {
        if(mx > x && mx < x + w)
        {
            if(my > y+offset && my < y + h + offset)
            {
                return true;
            }
        }

        return false;
    }
};

class Renderer
{
public:
    Renderer(Scene *scene, CameraManager *camManager, GUI *gui);
    ~Renderer();

    void init();
    void render(Transform &trans);
    
    void resize(int width, int height);
    void toggleBGColor();
    void togglePolygonMode();
    void initPreviewFBOs();
    void onMouseClick(int mx, int my);
    void onMouseWheel(int delta);

private:
    void renderScene(const Transform &trans);
    void renderIntoPreviewFBOs(Transform &trans);
    void renderPreviews(Transform &trans);
    void renderIntoMainFBO(Transform &trans);

	float computeFBODifference(GLuint texA, GLuint texB);
	void renderSceneDifference();

private:
    GUI *m_gui;
    CameraManager *m_cameraManager;
	Scene *m_scene;

    int m_width;
    int m_height;

    vec4 m_bgColor; 

    const GLuint m_samples;
    GLuint m_bgMode;

    FrameBufferObjectMultisample *m_fbo;
	FrameBufferObject *m_fboDifference;
	VertexBufferObject *m_vboQuad;

    vector<Preview> m_previewFBOs;
    int m_activePreview;

    int m_frameCount;

    int m_prevYOffset;
};

#endif

 