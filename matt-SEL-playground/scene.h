
struct SceneObject
{
	int index;
	string modelName;
	int parentIndex;
	mat4f transform;
};

struct Scene
{
	void loadJSON(const string &filename);
	void saveSSG(const string &filename);

	vector<SceneObject> objects;
};