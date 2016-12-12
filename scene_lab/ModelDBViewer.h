#pragma once

#include <QGLViewer/qglviewer.h>

class ModelDatabase;
class ModelDBViewer_widget;

class CModel;

class ModelDBViewer : public QGLViewer
{
public:
	ModelDBViewer();
	~ModelDBViewer();
	
	void draw();
	void init();

	void updateModel(CModel *m);
	void setViewBounds();

private:
	CModel* m_model;

};

