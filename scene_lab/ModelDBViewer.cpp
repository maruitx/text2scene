#include "ModelDBViewer.h"
#include "../common/geometry/CModel.h"


ModelDBViewer::ModelDBViewer()
{
	m_model = NULL;
}


ModelDBViewer::~ModelDBViewer()
{
}

void ModelDBViewer::draw()
{
	if (m_model != NULL)
	{
		m_model->draw();
	}
}

void ModelDBViewer::init()
{
	// set background color
	setBackgroundColor(QColor(255, 255, 255, 255));

	// set view dir
	this->camera()->setPosition(qglviewer::Vec(0, -3, 2));
}

void ModelDBViewer::updateModel(CModel *m)
{
	m_model = m;

	// update scene bounds
	setViewBounds();
}

void ModelDBViewer::setViewBounds()
{
	if (m_model != NULL)
	{

		MathLib::Vector3 a = m_model->getMinVert();
		MathLib::Vector3 b = m_model->getMaxVert();

		qglviewer::Vec vecA(a[0], a[1], a[2]);
		qglviewer::Vec vecB(b[0], b[1], b[2]);

		this->setSceneCenter((vecA + vecB) * 0.5);
		this->setSceneRadius((vecB - vecA).norm()*0.5);
		this->setSceneBoundingBox(vecA, vecB);
		this->camera()->setPosition(qglviewer::Vec(0, -(vecB - vecA).norm(), 0)); // rest camera

		this->showEntireScene();
		this->updateGL();
	}
}
