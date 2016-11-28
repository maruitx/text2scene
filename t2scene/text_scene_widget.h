#ifndef TEXT_SCENE_WIDGET_H
#define TEXT_SCENE_WIDGET_H

#include <QWidget>
#include "ui_text_scene_widget.h"

class text2scene_mode;

class text_scene_widget : public QWidget
{
	Q_OBJECT

public:
	text_scene_widget(text2scene_mode *m = 0);
	~text_scene_widget();

	Ui::text_scene_widget *ui;

private:
	text2scene_mode *m_mode;
};

#endif // TEXT_SCENE_WIDGET_H
