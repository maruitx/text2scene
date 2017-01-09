#include "TextDialog.h"
#include "GLWidget.h"
#include "Scene.h"
#include <qtextedit.h>
#include <qpushbutton.h>
#include <qboxlayout.h>
#include <QProcess>

TextDialog::TextDialog(GLWidget *parent, Scene *s)
 : m_parent(parent), 
   m_scene(s),
   m_showState(false), 
   m_captureSpeech(false)
{
	init();
	toggleShow(m_parent->pos().x(), m_parent->pos().y());

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(checkSpeech()));    
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

	m_editSentence->setPlainText("There is a desk and a chair");
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

	QString inputSentence = m_editSentence->toPlainText();

	if (inputSentence.contains("LM "))
	{
		QStringList stringList = inputSentence.split(" ");
		QString modelName = stringList[1];

		MetaModel m;
		m.name = modelName.toStdString();
		m.path = params::inst()->modelDirectory + modelName.toStdString() + ".obj";
		mat4 transMat;
		transMat.setToIdentity();
		m.transformation = transMat;
		m.isInited = true;

		m_scene->roomModel = m;

		int varNum = m_scene->m_variations.size();
		for (int i = 0; i < varNum; i++)
		{
			m_scene->m_variations[i]->updateRoomModel(m);
		}

		return;
	}

	if (inputSentence.contains("LS "))
	{
		inputSentence.remove("LS ");
		QStringList sceneNameList = inputSentence.split(" ");
		
		int sceneNum = sceneNameList.size();

		for (int i = 0; i < sceneNum; i++)
		{
			if (sceneNum < m_scene->m_variations.size())
			{
				QString sceneFileName = QString(params::inst()->sceneDirectory.c_str()) + sceneNameList[i] + ".txt";
				m_scene->m_variations[i]->loadSceneFile(sceneFileName);
			}
		}

		return;
	}


	QString filename = "in.txt";

	QFile outFile(filename);
	QTextStream ofs(&outFile);

	if (!outFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
	{
		cout << "\TextDialog: cannot save text to in.txt\n";
		return;
	}
	else
	{
		ofs << inputSentence;
		outFile.close();	}

	// To-do: call command line text parser to generate out.txt
	QString parserProgramName = "matt-SEL-playground.exe";
	
	QProcess::execute(parserProgramName);
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

void TextDialog::toggleSpeech()
{    
    m_captureSpeech = !m_captureSpeech;

    if(m_captureSpeech)
    {
        if(!m_timer.isActive())
        {
            m_timer.start(10);
            qDebug() << "Speeach capturing enabled.";

            QString workingDir = "../speech2text/";
            QString cmd = QString("start python " + workingDir + "transcribeStreamingMatt.py");

            system(cmd.toStdString().c_str());
        }        
    }
    else
    {
        m_timer.stop();
        qDebug() << "Speeach capturing disabled.";
    }
}

void TextDialog::checkSpeech()
{
    //QString dir = "../speech2text/";
    QString dir = "";

    QFile stateFile(dir + "readySignal.txt");
    if (stateFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {      
        QTextStream stateIn(&stateFile);
        QString line = stateIn.readLine();
        
        if(line == "ready")
        {
            QFile transFile(dir + "transcript.txt");
            if (transFile.open(QIODevice::ReadOnly | QIODevice::Text))
            {      
                QTextStream transIn(&transFile);
                QString line = transIn.readLine();  

                params::inst()->currentText = line;
                params::inst()->textCoolDown = 250;

                m_editSentence->setText(line);               
            }

            transFile.close();
        }
    }
    stateFile.remove();
}