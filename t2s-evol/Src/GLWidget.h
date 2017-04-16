#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "Headers.h"

class Shader;
class Camera;
class Scene;
class Renderer;
class GUI;
class CameraManager;
class TextDialog;

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
   GLWidget(QGLContext *context, int width, int height);
   ~GLWidget();
   
   void initializeGL();   
   void paintGL();
   void resizeGL(int width, int height);   
   void wheelEvent(QWheelEvent *event);
   void keyPressEvent(QKeyEvent *event);
   void keyReleaseEvent(QKeyEvent *event);
   void mouseMoveEvent(QMouseEvent *event);
   void mousePressEvent(QMouseEvent *event);
   void mouseReleaseEvent(QMouseEvent *event);   
   void initParams();
   void initShaders();

   void loadParams();

private:       
    QPoint m_mouse;

    bool m_leftButton;
    bool m_rightButton;    
	bool m_ctrlPressed;
	bool m_altPressed;
	bool m_shiftPressed;    
    bool m_noOtherKey;
    bool m_renderOffline;

    int m_width;
    int m_height;
    int m_frameNr;

	Scene *m_scene;
	Renderer *m_renderer;
	GUI *m_gui;
	CameraManager *m_cameraManager;

    Transform m_trans;
    double m_oldTime;

    HPTimer m_timer;

	TextDialog *m_textDialog;
};

#endif

