#ifndef MODELDBVIEWER_WIDGET_H
#define MODELDBVIEWER_WIDGET_H

#include <QWidget>
#include "ui_modeldbviewer_widget.h"

class ModelDatabase;
class ModelDBViewer;

class CModel;

class ModelDBViewer_widget : public QWidget
{
	Q_OBJECT

public:
	ModelDBViewer_widget(ModelDatabase *modleDB, QWidget *parent = 0);
	~ModelDBViewer_widget();

	void setModelIdWidgetForCat(const QString &catName);

public slots:
void updateModelIdList();
void update3DModel();

private:
	Ui::ModelDBViewer_widget ui;
	ModelDatabase *m_modelDB;

	ModelDBViewer *m_viewer;
	CModel *m_displayedModel;

};

#endif // MODELDBVIEWER_WIDGET_H
