#ifndef OBSTACLEDIALOG
#define OBSTACLEDIALOG

#include "Headers.h"
#include <QDialog>

class GLWidget;
class QPushButton;
class QTextEdit;

class TextDialog : public QDialog
{
	Q_OBJECT

private:
    GLWidget *m_parent;
	bool m_showState;

	QPushButton *m_buttonProcess;
	QTextEdit *m_editSentence;

public slots:
	void onButtonProcess();

public:	
	TextDialog(GLWidget *parent);
	~TextDialog();

	void init();
	void initEditSentence();

	void setupConnection();
	void toggleShow(int posX, int posY);

	void keyPressEvent(QKeyEvent *e);
};

#endif