#ifndef SCENE_LAB_WIDGET_H
#define SCENE_LAB_WIDGET_H

#include <QObject>
#include <QWidget>
#include <QString>
#include "ui_scene_lab_widget.h"

class scene_lab;

class scene_lab_widget : public QWidget
{
	Q_OBJECT

public:
	scene_lab_widget(scene_lab *s_lab, QWidget *parent=0);
	~scene_lab_widget();

	QString loadSceneName();

	Ui::scene_lab_widget *ui;

private:

	scene_lab *m_scene_lab;

	QString m_lastUsedDirectory;


};

#endif // SCENE_LAB_WIDGET_H
