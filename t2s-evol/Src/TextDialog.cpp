#include "TextDialog.h"
#include "GLWidget.h"
#include "Scene.h"
#include "Utility.h"
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

	// load room model
	if (inputSentence.contains("lrm "))
	{
		QStringList stringList = inputSentence.split(" ");
		QString modelName = stringList[1];

		if (!modelName.contains("room"))
		{
			return;
		}

		MetaModel m;
		m.name = modelName.toStdString();
		m.path = params::inst()->modelDirectory + modelName.toStdString() + ".obj";
		mat4 transMat;
		transMat.setToIdentity();
		m.transformation = transMat;
		m.isInitLoaded = true;

		m_scene->roomModel = m;

		int varNum = m_scene->m_variations.size();
		for (int i = 0; i < varNum; i++)
		{
			m_scene->m_variations[i]->updateRoomModel(m);
		}

		return;
	}

	// load a list of scenes with filename extension
	if (inputSentence.contains("ls "))
	{
		bool isRenderRoom = true;

		inputSentence.remove("ls ");

		// option for not loading room
		if (inputSentence.contains("-nr"))
		{
			inputSentence.remove("-nr ");
			isRenderRoom = false;
		}

		QStringList sceneNameList = inputSentence.split(" ");
		
		int sceneNum = sceneNameList.size();

		if (sceneNum < 0) return;

		// load local sceneDB path
		vector<string> localSceneDBPaths = getFileLines("./SceneDB/LocalSceneDBPath.txt", 3);
		string stanfordDBPath, shapeNetSemDBPath, scenennDBPath, changScenePath, resultPath;

		for (int i = 0; i < localSceneDBPaths.size(); i++)
		{
			if (localSceneDBPaths[i][0] != '#')
			{
				if (localSceneDBPaths[i].find("StanfordDB=") != string::npos)
				{
					stanfordDBPath = PartitionString(localSceneDBPaths[i], "StanfordDB=")[0];
					continue;
				}

				if (localSceneDBPaths[i].find("ShapeNetSemDB=") != string::npos)
				{
					shapeNetSemDBPath = PartitionString(localSceneDBPaths[i], "ShapeNetSemDB=")[0];
					params::inst()->shapeNetSemDirectory = shapeNetSemDBPath;
					continue;
				}
				
				if (localSceneDBPaths[i].find("SceneNNDB=") != string::npos)
				{
					scenennDBPath = PartitionString(localSceneDBPaths[i], "SceneNNDB=")[0];
					continue;
				}

				if (localSceneDBPaths[i].find("ChangScenePath=") != string::npos)
				{
					changScenePath = PartitionString(localSceneDBPaths[i], "ChangScenePath=")[0];
					continue;
				}
				
				if (localSceneDBPaths[i].find("ResultPath=") != string::npos)
				{
					resultPath = PartitionString(localSceneDBPaths[i], "ResultPath=")[0];
					continue;
				}							
			}
		}

		if (!dirExists(stanfordDBPath))
		{
			cout << "Please set your local StanfordSceneDB in SceneDB/LocalSceneDBPath.txt\n";
			return;
		}
		{
			// set default DB path as stanfordDB path
			params::inst()->sceneDirectory = stanfordDBPath + "scenes/";
			params::inst()->modelDirectory = stanfordDBPath + "models/";
			params::inst()->textureDirectory = stanfordDBPath + "textures/";
		}

		// check scene extension and set proper DB path
		// input scenes number as the variation number each time for now, otherwise will not be shown
		for (int i = 0; i < sceneNum; i++)
		{
			if (sceneNum < m_scene->m_variations.size())
			{
				QString sceneName = sceneNameList[i];

				if (sceneName.contains(".txt"))
				{
					// still need to update since may first load scenes from other DB
					params::inst()->sceneDirectory = stanfordDBPath + "scenes/";
					params::inst()->modelDirectory = stanfordDBPath + "models/";
					params::inst()->textureDirectory = stanfordDBPath + "textures/";
				}
				else if (sceneName.contains(".snn"))
				{
					// scenenn use stanfordDB models and textures
					params::inst()->sceneDirectory = scenennDBPath + "scenes/";
					params::inst()->modelDirectory = stanfordDBPath + "models/";
					params::inst()->textureDirectory = stanfordDBPath + "textures/";
				}
				else if (sceneName.contains(".user"))
				{
					// user scenes saved in resultPath and use the stanfordDB models and textures
					params::inst()->sceneDirectory = resultPath; 
					params::inst()->modelDirectory = stanfordDBPath + "models/";
					params::inst()->textureDirectory = stanfordDBPath + "textures/";
					
					double s = 0.1 / 0.0254;
					params::inst()->globalSceneViewScale = vec3(s, s,s) ;
				}
				else if (sceneName.contains(".chang"))
				{
					// chang scenes also saved in resultPath and 
					// if shapenetsem DB missing, use stanfordDB models

					params::inst()->sceneDirectory = changScenePath;

					// use the stanfordDB models and textures initially
					// if not in it, use ShapeNetSem models (tested when loading model in TSScene)
					params::inst()->modelDirectory = stanfordDBPath + "models/";
					params::inst()->textureDirectory = stanfordDBPath + "textures/";

					//if (shapeNetSemDBPath.empty() && !dirExists(shapeNetSemDBPath))
					//{
					//	cout << "ShapeNetSem DB not exist, use StanfordDB models instead; some models may be missing.\n";
					//	params::inst()->modelDirectory = stanfordDBPath + "models/";
					//	params::inst()->textureDirectory = stanfordDBPath + "textures/";
					//}
					//else
					//{
					//	params::inst()->modelDirectory = shapeNetSemDBPath + "models-OBJ/models/";
					//	params::inst()->textureDirectory = shapeNetSemDBPath + "models-textures/textures/";
					//}
				}
				else if (sceneName.contains(".result"))
				{
					// result use  stanfordDB models and textures
					params::inst()->sceneDirectory = resultPath;
					params::inst()->modelDirectory = stanfordDBPath + "models/";
					params::inst()->textureDirectory = stanfordDBPath + "textures/";
				}
				else{
					cout << "Please specify the extension of the scene file\n";
					return;
				}

				QString sceneFileName = QString(params::inst()->sceneDirectory.c_str()) + sceneNameList[i];

				if (fileExists(sceneFileName.toStdString()))
				{
					m_scene->m_variations[i]->loadSceneFile(sceneFileName);
					m_scene->m_variations[i]->m_isRenderRoom = isRenderRoom;
				}
				else
				{
					cout << "Scene file " << sceneFileName.toStdString() << " does not exist\n";
				}
			}
		}

		return;
	}

	if (inputSentence.contains("ss "))
	{

		//save scene name with no extention, default extention of ".result" will be added

		inputSentence.remove("ss ");

		// load local sceneDB path
		vector<string> localSceneDBPaths = getFileLines("./SceneDB/LocalSceneDBPath.txt", 3);
		string resultPath;

		for (int i = 0; i < localSceneDBPaths.size(); i++)
		{
			if (localSceneDBPaths[i][0] != '#')
			{
				if (localSceneDBPaths[i].find("ResultPath=") != string::npos)
				{
					resultPath = PartitionString(localSceneDBPaths[i], "ResultPath=")[0];
					continue;
				}
			}
		}

		if (!dirExists(resultPath))
		{
			cout << "Please set your result path in SceneDB/LocalSceneDBPath.txt\n";
			return;
		}

		QString  sceneFileName = QString(resultPath.c_str()) + inputSentence + ".result";
		QFile outFile(sceneFileName);
		QTextStream ofs(&outFile);

		if (!outFile.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate))
		{
			cout << "Cannot open file " << sceneFileName.toStdString() << " to save the scene.\n";
		}
		else{
			TSScene *tsscene = m_scene->m_variations[m_scene->m_activeVarationId];

			ofs << "StanfordSceneDatabase\n";
			MetaScene& metaScene = tsscene->getMetaScene();
			int modelNum = metaScene.m_metaModellList.size();
			ofs << "modelCount " << modelNum << "\n";
			for (int i = 0; i < modelNum; i++)
			{
				MetaModel& md = metaScene.m_metaModellList[i];
				ofs << "newModel " << i << " " << QString(md.name.c_str()) << "\n";
				ofs << "transform " << QString("%1").arg(md.transformation.a11, 0, 'f') << " " << QString("%1").arg(md.transformation.a21, 0, 'f') << " " << QString("%1").arg(md.transformation.a31, 0, 'f') << " " << QString("%1").arg(md.transformation.a41, 0, 'f') << " "
					<< QString("%1").arg(md.transformation.a12, 0, 'f') << " " << QString("%1").arg(md.transformation.a22, 0, 'f') << " " << QString("%1").arg(md.transformation.a32, 0, 'f') << " " << QString("%1").arg(md.transformation.a42, 0, 'f') << " "
					<< QString("%1").arg(md.transformation.a13, 0, 'f') << " " << QString("%1").arg(md.transformation.a23, 0, 'f') << " " << QString("%1").arg(md.transformation.a33, 0, 'f') << " " << QString("%1").arg(md.transformation.a43, 0, 'f') << " "
					<< QString("%1").arg(md.transformation.a14, 0, 'f') << " " << QString("%1").arg(md.transformation.a24, 0, 'f') << " " << QString("%1").arg(md.transformation.a34, 0, 'f') << " " << QString("%1").arg(md.transformation.a44, 0, 'f') <<"\n";
			}

			cout << "Save current scene to file " << sceneFileName.toStdString() << "\n";
			outFile.close();
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