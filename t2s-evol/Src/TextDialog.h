#ifndef OBSTACLEDIALOG
#define OBSTACLEDIALOG

#include "Headers.h"
#include <QDialog>
#include <QProcess>

class GLWidget;
class QPushButton;
class QTextEdit;
class Scene;

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

    bool m_captureSpeech;

public slots:
	void onButtonProcess();
    void checkSpeech();

public:	
	TextDialog(GLWidget *parent, Scene *s);
	~TextDialog();

	void init();
	void initEditSentence();

	void setupConnection();
	void toggleShow(int posX, int posY);

	void keyPressEvent(QKeyEvent *e);

    void toggleSpeech();        
};

#endif