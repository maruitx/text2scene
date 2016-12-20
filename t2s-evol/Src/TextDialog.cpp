#include "TextDialog.h"
#include "GLWidget.h"
#include "Scene.h"
#include <qtextedit.h>
#include <qpushbutton.h>
#include <qboxlayout.h>

TextDialog::TextDialog(GLWidget *parent, Scene *s)
	: m_parent(parent), 
	m_scene(s),
  m_showState(false)
{
	init();
	toggleShow(m_parent->pos().x(), m_parent->pos().y());
}

TextDialog::~TextDialog()
{
	delete m_buttonProcess;
	delete m_editSentence;
}

void TextDialog::init()
{
	initEditSentence();
	m_buttonProcess = new QPushButton("Process", this);

	QBoxLayout* boxLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	boxLayout->addWidget(m_editSentence);
	boxLayout->addWidget(m_buttonProcess);

	this->setLayout(boxLayout);

	setupConnection();

	this->setParent(m_parent);
	this->setFocusPolicy(Qt::NoFocus);
	this->setVisible(m_showState);
	this->setWindowTitle("Text Dialog");
	this->setMaximumSize(200, 300);
	this->setWindowFlags(Qt::WindowStaysOnTopHint);
	this->setWindowFlags(Qt::Tool);
}

void TextDialog::initEditSentence()
{
	m_editSentence = new QTextEdit(this);

	//m_editSentence->setPlainText("Below the TV is a low-profile media cabinet which contains some audiovisual equipment."
	//	                         "Each unit contains a mixture of books and decorations. Inside the basket, "
	//							 "there is a basketball and other sports equipment.");

	m_editSentence->setPlainText("Debug: directly parsing result from out.txt");
}

void TextDialog::setupConnection()
{
	connect(m_buttonProcess, SIGNAL(pressed()), this, SLOT(onButtonProcess()));
}
 
void TextDialog::toggleShow(int posX, int posY)
{
    m_showState = !m_showState;   

    this->setGeometry(posX+25, posY+65, this->size().width(), this->size().height());
    this->setVisible(m_showState);     
}

void TextDialog::onButtonProcess()
{
	//qDebug() << m_editSentence->toPlainText();

	// To-do: save current text to in.txt

	// To-do: call command line parser to generate out.txt

	m_scene->runOneEvolutionStep();
}

void TextDialog::keyPressEvent(QKeyEvent *e)
{
	switch (e->key())
	{
	case Qt::Key_Space:
		break;
	}
}