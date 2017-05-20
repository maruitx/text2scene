#ifndef OBSTACLEDIALOG
#define OBSTACLEDIALOG

#include "Headers.h"
#include <QDialog>
#include <QProcess>

class GLWidget;
class QPushButton;
class QTextEdit;
class Scene;
class Renderer;

class TextDialog : public QDialog
{
	Q_OBJECT

private:
    GLWidget *m_parent;
	bool m_showState;

	QPushButton *m_buttonProcess;
	QTextEdit *m_editSentence;

	Scene *m_scene;
    QTimer m_timer;

	Renderer *m_render;

    bool m_captureSpeech;

public slots:
	void onButtonProcess();
    void checkSpeech();

public:	
	TextDialog(GLWidget *parent, Renderer *render, Scene *s);
	~TextDialog();

	void init();
	void initEditSentence();

	void setupConnection();
	void toggleShow(int posX, int posY);

	void keyPressEvent(QKeyEvent *e);

    void toggleSpeech();        
};

#endif