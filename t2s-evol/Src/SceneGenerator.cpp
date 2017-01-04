#include "SceneGenerator.h"
#include "SceneSemGraphManager.h"
#include "SceneSemGraph.h"
#include "SemGraphMatcher.h"

SceneGenerator::SceneGenerator(const QString &sceneDBPath, unordered_map<string, Model*> &models)
	:m_models(models)
{
	m_sceneSemGraphManager = new SceneSemGraphManager(sceneDBPath);
	m_semanticGraphMatcher = new SemGraphMatcher(m_sceneSemGraphManager);
}

SceneGenerator::~SceneGenerator()
{

}

void SceneGenerator::setCurrentTextGraph(TextSemGraph *tsg)
{
	m_semanticGraphMatcher->updateCurrentTextSemGraph(tsg);
}

std::vector<TSScene*> SceneGenerator::generateTSScenes(int num)
{
	//vector<SceneSemGraph*> matchedSSGs = m_semanticGraphMatcher->testMatchTSGWithSSGs(num);
	std::vector<SceneSemGraph*> matchedSSGs = m_semanticGraphMatcher->matchTSGWithSSGs(num);

	std::vector<TSScene*> tsscenes;

	for (int i = 0; i < matchedSSGs.size(); i++)
	{
		QString sceneName = QString("Preview %1").arg(i);
		TSScene *s = matchedSSGs[i]->covertToTSScene(m_models, sceneName);
		tsscenes.push_back(s);
	}

	return tsscenes;
}
