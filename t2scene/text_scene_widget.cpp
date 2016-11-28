#include "text_scene_widget.h"
#include "text2scene_mode.h"
#include <QFileDialog>

text_scene_widget::text_scene_widget(text2scene_mode *m):
ui(new Ui::text_scene_widget)
{
	ui->setupUi(this);
	m_mode = m;

	connect(ui->showSceneLabButton, SIGNAL(toggled(bool)), m_mode, SLOT(showSceneLab(bool)));
}

text_scene_widget::~text_scene_widget()
{
	delete ui;
}
