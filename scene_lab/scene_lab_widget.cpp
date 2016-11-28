#include "scene_lab_widget.h"
#include "scene_lab.h"

#include <QFileDialog>
#include <QTextStream>

scene_lab_widget::scene_lab_widget(scene_lab *s_lab, QWidget *parent/*=0*/)
	: m_scene_lab(s_lab), ui(new Ui::scene_lab_widget)
{
	ui->setupUi(this);

	m_lastUsedDirectory = QString();

	connect(ui->loadSceneButton, SIGNAL(clicked()), m_scene_lab, SLOT(loadScene()));
	connect(ui->showOBBCheckBox, SIGNAL(stateChanged(int)), m_scene_lab, SLOT(updateSceneRenderingOptions()));
	connect(ui->showGraphCheckBox, SIGNAL(stateChanged(int)), m_scene_lab, SLOT(updateSceneRenderingOptions()));

}

scene_lab_widget::~scene_lab_widget()
{
	delete ui;
}

QString scene_lab_widget::loadSceneName()
{
	QString lastDirFileName = QDir::currentPath() + "/lastSceneDir.txt";
	QFile lastDirFile(lastDirFileName);

	if (lastDirFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream ifs(&lastDirFile);

		m_lastUsedDirectory = ifs.readLine();
		lastDirFile.close();
	}

	QString filename = QFileDialog::getOpenFileName(0, tr("Load scene"),
		m_lastUsedDirectory,
		tr("Scene File (*.txt)"));

	if (filename.isEmpty()) return "";

	QFileInfo fileInfo(filename);
	m_lastUsedDirectory = fileInfo.absolutePath();

	// save last open dir
	if (lastDirFile.open(QIODevice::ReadWrite | QIODevice::Text))
	{
		QTextStream ofs(&lastDirFile);
		ofs << m_lastUsedDirectory;

		lastDirFile.close();
	}

	return filename;
}
