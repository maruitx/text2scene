#include "category.h"
#include "modelDatabase.h"

Category::Category()
{
	m_categoryLevel = 0;
}

Category::Category(const QString &s):
m_catName(s)
{
	//m_modelBlackList.push_back("wss.937cdf200b33bdfd1aec2282dd8bc87a");
	//m_modelBlackList.push_back("wss.6475d0da1bfe0006c78df40cf2e9097a");
	//m_modelBlackList.push_back("wss.53433f3e15014d831220240e825d0985");
	//m_modelBlackList.push_back("wss.ee7ef8b40cc2c90d6b7170bfc4da1f8");
	//m_modelBlackList.push_back("wss.225d37286bed9bb9dbf53b03c847b004");

	//m_sharedModelList.push_back("wss.6c3bf99a9534c75314513156cf2b8d0d");
	//m_sharedModelList.push_back("wss.49a0a84ee5a91344b11c5cce13f76151");
	//m_sharedModelList.push_back("wss.3b8cd62e5842a4c98870030ed2672a1d");
	//m_sharedModelList.push_back("wss.af3dda1cfe61d0fc9403b0d0536a04af");
	//m_sharedModelList.push_back("wss.7b1fc86844257f8fa54fd40ef3a8dfd0");
	//m_sharedModelList.push_back("wss.6ca2149ac6d3699130612f5c0ef21eb8");
	//m_sharedModelList.push_back("wss.7446fa250adf49c5e7ef9fff09638f8e");
	//m_sharedModelList.push_back("wss.b1d75ad18d986ec760005b40a079e2d3");
	//m_sharedModelList.push_back("wss.f7800c8710eacfb9b7212d4f8780127f");

	//m_sharedModelList.push_back("6c3bf99a9534c75314513156cf2b8d0d");
	//m_sharedModelList.push_back("49a0a84ee5a91344b11c5cce13f76151");
	//m_sharedModelList.push_back("3b8cd62e5842a4c98870030ed2672a1d");
	//m_sharedModelList.push_back("af3dda1cfe61d0fc9403b0d0536a04af");
	//m_sharedModelList.push_back("7b1fc86844257f8fa54fd40ef3a8dfd0");
	//m_sharedModelList.push_back("6ca2149ac6d3699130612f5c0ef21eb8");
	//m_sharedModelList.push_back("7446fa250adf49c5e7ef9fff09638f8e");
	//m_sharedModelList.push_back("b1d75ad18d986ec760005b40a079e2d3");
	//m_sharedModelList.push_back("f7800c8710eacfb9b7212d4f8780127f");
}

Category::~Category()
{

}


MetaModel* Category::sampleInstance()
{
	while (true)
	{
		int randId = GenRandomInt(0, modelInstances.size());

		MetaModel* result = modelInstances[randId];
		//if (catName == "Table" && result.idStr!= "wss.eed2fa156d662c79b26e384cea2f274e")
		//	continue;

		//if (catName == "Table" && result.idStr == "wss.eed2fa156d662c79b26e384cea2f274e" && firstScale == 0.0f)
		//{
		//	firstScale = 1.0f;
		//	DisembodiedObject *x = (DisembodiedObject *)&result;
		//	x->scale *= 1.1f;
		//}

		//if (!result.model->blacklisted)

		if (isInModelBlackList(result->getIdStr()))
		{
			continue;
		}

		if (isSharedModel(result->getIdStr()))
		{
			continue;
		}

		return result;
	}
}

bool Category::isInModelBlackList(const QString &s)
{
	if (s == "wss.937cdf200b33bdfd1aec2282dd8bc87a" || // headphone
		s == "wss.6475d0da1bfe0006c78df40cf2e9097a" ||  // headphone
		s == "wss.53433f3e15014d831220240e825d0985" || // headphone
		s == "wss.ee7ef8b40cc2c90d6b7170bfc4da1f8" ||
		s == "wss.225d37286bed9bb9dbf53b03c847b004")  // cellphone

		return true;
	else
		return false;
}

bool Category::isSharedModel(const QString &s)
{
	//  do not sample shared model
	if (s == "6c3bf99a9534c75314513156cf2b8d0d" || 
		s == "49a0a84ee5a91344b11c5cce13f76151" || 
		s == "3b8cd62e5842a4c98870030ed2672a1d" ||
		s == "af3dda1cfe61d0fc9403b0d0536a04af" ||
		s == "7b1fc86844257f8fa54fd40ef3a8dfd0" ||
		s == "6ca2149ac6d3699130612f5c0ef21eb8" || 
		s == "7446fa250adf49c5e7ef9fff09638f8e" || 
		s == "b1d75ad18d986ec760005b40a079e2d3" ||
		s == "f7800c8710eacfb9b7212d4f8780127f" ) 

		return true;
	else
		return false;
}

