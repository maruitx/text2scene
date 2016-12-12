#include "modeldbviewer_widget.h"
#include "modelDatabase.h"
#include "category.h"
#include "ModelDBViewer.h"
#include "../common/geometry/CModel.h"

ModelDBViewer_widget::ModelDBViewer_widget(ModelDatabase *modleDB, QWidget *parent /*= 0*/)
	:m_modelDB(modleDB)
{
	ui.setupUi(this);
	m_displayedModel = NULL;

	m_viewer = new ModelDBViewer();
	m_viewer->show();


	// init category list
	for (auto it = m_modelDB->dbCategories.begin(); it != m_modelDB->dbCategories.end(); it++)
	{
		Category* currCat = it->second;

		if (currCat->getCatgoryLevel() == 0)
		{
			ui.catSelectListWidget->addItem(currCat->getCatName());
		}
	}

	ui.catNumLabel->setText(QString("Category Num: %1").arg(m_modelDB->getParentCatNum()));
	ui.catSelectListWidget->setCurrentRow(0);
	
	// init model id list
	updateModelIdList();

	connect(ui.catSelectListWidget, SIGNAL(itemSelectionChanged()), this, SLOT(updateModelIdList()));
	connect(ui.modelIdListWidget, SIGNAL(itemSelectionChanged()), this, SLOT(update3DModel()));
}

ModelDBViewer_widget::~ModelDBViewer_widget()
{

}

void ModelDBViewer_widget::setModelIdWidgetForCat(const QString &catName)
{
	ui.modelIdListWidget->clear();

	Category* currCat = m_modelDB->dbCategories[catName];
	int instanceNum = currCat->getInstanceNum();
	
	for (int i = 0; i < instanceNum; i++)
	{
		CandidateModel* currModel = currCat->modelInstances[i];
		ui.modelIdListWidget->addItem(currModel->getIdStr());
	}

	ui.modelNumLabel->setText(QString("Model Num: %1").arg(instanceNum));
	ui.modelIdListWidget->setCurrentRow(0);

	update3DModel();
}

void ModelDBViewer_widget::updateModelIdList()
{
	QString currCatName = ui.catSelectListWidget->currentItem()->text();
	setModelIdWidgetForCat(currCatName);
}

void ModelDBViewer_widget::update3DModel()
{
	QString modelIdStr = ui.modelIdListWidget->currentItem()->text();

	if (m_displayedModel != NULL)
	{
		delete m_displayedModel;
	}

	m_displayedModel = new CModel();
	double modelScale = m_modelDB->dbCandiModels[modelIdStr]->getScale();

	// debug some model scale is 0
	if (modelScale < 1e-8 )
	{
		modelScale = 1;
	}

	QString modelFileName = m_modelDB->getDBPath() + "/models-OBJ/models/" + modelIdStr + ".obj";
	m_displayedModel->loadModel(modelFileName, 1.0);

	m_viewer->updateModel(m_displayedModel);
}
