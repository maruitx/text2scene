#include "Model.h"
#include "Shader.h"
#include "VertexBufferObject.h"
#include "Mesh.h"
#include "Light.h"

#include <sstream> 

ModelMesh::ModelMesh(vector<VertexBufferObject::DATA> &vertices, vector<uint> &indices, const Material &material)
: m_vertices(vertices), 
  m_indices(indices), 
  m_material(material), 
  m_vbo(nullptr)
{
}

ModelMesh::~ModelMesh()
{
	delete m_vbo;
}

void ModelMesh::buildVBO()
{
	m_vbo = new VertexBufferObject();
	m_vbo->setData(m_vertices.data(), GL_STATIC_DRAW, m_vertices.size(), GL_TRIANGLES);
	m_vbo->setIndexData(m_indices.data(), GL_STATIC_DRAW, m_indices.size());
	m_vbo->bindDefaultAttribs();

	m_vertices.clear();
	m_indices.clear();
}

void ModelMesh::computeNormals()
{
	for (int i = 0; i < m_indices.size(); i+=3)
	{
		uint a = m_indices[i];
		uint b = m_indices[i + 1];
		uint c = m_indices[i + 2];

		VertexBufferObject::DATA &r = m_vertices[a];
		VertexBufferObject::DATA &s = m_vertices[b];
		VertexBufferObject::DATA &t = m_vertices[c];

		vec3 vr = vec3(r.vx, r.vy, r.vz);
		vec3 vs = vec3(s.vx, s.vy, s.vz);
		vec3 vt = vec3(t.vx, t.vy, t.vz);

		vec3 n = normalize(cross(vr - vs, vr - vt));

		r.nx = n.x;
		r.ny = n.y;
		r.nz = n.z;

		s.nx = n.x;
		s.ny = n.y;
		s.nz = n.z;

		t.nx = n.x;
		t.ny = n.y;
		t.nz = n.z;
	}
}

void ModelMesh::render(const Transform &trans, Shader *shader, const mat4 &model)
{
	if (m_vbo)
	{
		shader->set3f("Kd", m_material.Kd);

		auto &iter = params::inst()->textures.find(m_material.texName);
		if (iter != params::inst()->textures.end())
		{
			Texture *tex = iter->second;
			shader->setTexture("tex", tex->id());
		}
		else if (m_material.texName.length() > 0)
		{
			string path = params::inst()->textureDirectory + m_material.texName;
			
			Texture *tex = new Texture(QString(path.c_str()));
			tex->setEnvMode(GL_REPLACE);
			tex->setWrapMode(GL_REPEAT);
			tex->setFilter(GL_LINEAR, GL_LINEAR);

			params::inst()->textures.insert(make_pair(m_material.texName, tex));
			cout << "\nLoading Texture: " << m_material.texName;
		}

		m_vbo->render();		
	}

}

void ModelMesh::renderDepth(const Transform &trans, const mat4 &model)
{
	if (m_vbo)
	{
		if (params::inst()->applyCulling)
		{
			glCullFace(GL_BACK);
			glEnable(GL_CULL_FACE);
		}

		mat4 m = mat4::scale(vec3(0.1)) * mat4::rotateX(-90) * model;

		Shader *shader = shaders::inst()->modelDepth;
		shader->bind();
		shader->setMatrices(trans, m);
			m_vbo->render();
		shader->release();
	}
}


ModelThread::ModelThread(const string &fileName, vector<ModelMesh> &meshes, BoundingBox &bb)
: m_fileName(fileName), 
  m_meshes(meshes), 
  m_bb(bb)
{

}

void ModelThread::run()
{
	load(m_fileName);
}

void ModelThread::load(const string &fileName)
{
	QFileInfo fi(QString(fileName.c_str()));
	string baseName = fi.baseName().toStdString();
	cout << "\nLoading Model: " << baseName;

	string modelFile = params::inst()->modelDirectory + baseName + string(".obj");
	string materialFile = params::inst()->modelDirectory + baseName + string(".mtl");

	std::vector<std::string> objLines = getFileLines(modelFile, 3);
	std::vector<std::string> mtlLines = getFileLines(materialFile, 3);

	//Load Material
	std::map<std::string, Material> materials;
	Material *activeMaterial = nullptr;

	for (unsigned int lineIndex = 0; lineIndex < mtlLines.size(); lineIndex++)
	{
		const std::string &curLine = mtlLines[lineIndex];
		if (curLine.find("newmtl ") == 0)
		{
			const std::string materialName = curLine.substr(7);
			materials[materialName] = Material();
			activeMaterial = &materials[materialName];
		}
		if (curLine[0] == 'K' && curLine[1] == 'd')
		{
			std::stringstream stream(std::stringstream::in | std::stringstream::out);
			stream << curLine.substr(3);
			stream >> activeMaterial->Kd.x >> activeMaterial->Kd.y >> activeMaterial->Kd.z;
		}
		if (curLine[0] == 'd' && curLine[1] == ' ')
		{
			std::stringstream stream(std::stringstream::in | std::stringstream::out);
			stream << curLine.substr(3);
			stream >> activeMaterial->transparency;
		}
		if (curLine.find("map_Kd ") == 0)
		{
			const std::string textureName = curLine.substr(7);
			QString tmpStr = QString(textureName.c_str()).replace("../textures/", "");
			tmpStr.chop(4);
			activeMaterial->texName = tmpStr.toStdString();
		}
	}

	// Load meshes
	std::vector<VertexBufferObject::DATA> vertices;
	std::vector<unsigned int> indices;
	activeMaterial = nullptr;

	for (unsigned int lineIndex = 0; lineIndex < objLines.size(); lineIndex++)
	{
		const std::string &curLine = objLines[lineIndex];
		if (curLine[0] == 'v' && curLine[1] == ' ')
		{
			VertexBufferObject::DATA vert;
			initVertexBufferData(vert);

			vert.cx = 1.0f, vert.cy = 1.0f, vert.cz = 1.0f, vert.cw = 1.0f;
			vertices.push_back(vert);
			VertexBufferObject::DATA &curVertex = vertices[vertices.size() - 1];

			std::stringstream stream(std::stringstream::in | std::stringstream::out);
			stream << curLine.substr(2);
			stream >> curVertex.vx >> curVertex.vy >> curVertex.vz;

			if (vertices.size() == 1)
			{
				m_bb.setMin(vec3(curVertex.vx, curVertex.vy, curVertex.vz));
				m_bb.setMax(vec3(curVertex.vx, curVertex.vy, curVertex.vz));
			}
			else
			{
				m_bb.setMin(min(m_bb.mi(), vec3(curVertex.vx, curVertex.vy, curVertex.vz)));
				m_bb.setMax(max(m_bb.ma(), vec3(curVertex.vx, curVertex.vy, curVertex.vz)));
			}
		}
		if (curLine[0] == 'v' && curLine[1] == 't')
		{
			VertexBufferObject::DATA &curVertex = vertices[vertices.size() - 1];

			std::stringstream stream(std::stringstream::in | std::stringstream::out);
			stream << curLine.substr(3);
			stream >> curVertex.tx >> curVertex.ty;
		}
		if (curLine[0] == 'f' && curLine[1] == ' ')
		{
			std::stringstream stream(std::stringstream::in | std::stringstream::out);

			unsigned int index0, index1, index2;
			if (curLine.find('/') == std::string::npos)
			{
				stream << curLine.substr(2);
				stream >> index0 >> index1 >> index2;
			}
			else
			{
				std::string curLineCopy = curLine.substr(2);
				for (unsigned int charIndex = 0; charIndex < curLineCopy.size(); charIndex++)
				{
					if (curLineCopy[charIndex] == '/')
					{
						curLineCopy[charIndex] = ' ';
					}
				}
				stream << curLineCopy;
				unsigned int temp0, temp1, temp2;
				stream >> index0 >> temp0 >> index1 >> temp1 >> index2 >> temp2;
			}
			indices.push_back(index0 - 1);
			indices.push_back(index1 - 1);
			indices.push_back(index2 - 1);
		}
		if (curLine.find("usemtl ") == 0)
		{
			if (indices.size() > 0)
			{
				std::vector<VertexBufferObject::DATA> tmpVerts;
				std::vector<uint> tmpIndices;
				uint startIdx = indices[0];

				for (int i = 0; i < indices.size(); ++i)
				{
					tmpVerts.push_back(vertices[indices[i]]);
					tmpIndices.push_back(indices[i] - startIdx);
				}

				ModelMesh m = ModelMesh(tmpVerts, tmpIndices, *activeMaterial);
				m.computeNormals();
				m_meshes.push_back(m);
				indices.clear();
			}
			const std::string materialName = curLine.substr(7);
			activeMaterial = &materials[materialName];
		}
	}

	if (indices.size() > 0)
	{
		std::vector<VertexBufferObject::DATA> tmpVerts;
		std::vector<uint> tmpIndices;
		uint startIdx = indices[0];

		for (int i = 0; i < indices.size(); ++i)
		{
			tmpVerts.push_back(vertices[indices[i]]);
			tmpIndices.push_back(indices[i] - startIdx);
		}

		ModelMesh m = ModelMesh(tmpVerts, tmpIndices, *activeMaterial);

		m.computeNormals();
		m_meshes.push_back(m);		
		indices.clear();
	}
}


Model::Model(const string &fileName)
: m_thread(fileName, m_meshes, m_bb)
{
	//Code for parallel loading
	connect(&m_thread, SIGNAL(finished()), this, SLOT(loadingDone()));
	m_thread.start();

	//Code for sequential loading
	//m_thread.run();
	//for (auto &i : m_meshes)
	//	i.buildVBO();
}

void Model::render(const Transform &trans, const mat4 &initTrans, bool applyShadow)
{
	if (params::inst()->applyCulling)
	{
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);
	}

	mat4 m = mat4::scale(vec3(0.1)) * mat4::rotateX(-90) * initTrans;

	Shader *shader = shaders::inst()->model;
	shader->bind();
		shader->setMatrices(trans, m, true, true, true, true, true);

		shader->set3f("lightPos", params::inst()->lights[0]->position());
		shader->setTexture("shadowMap", params::inst()->lights[0]->shadowMapBlurredId());
		shader->seti("applyShadow", params::inst()->applyShadow && applyShadow ? 1 : 0);

		for (auto &i : m_meshes)
		{
			i.render(trans, shader, mat4::identitiy());
		}

	shader->release();

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Model::renderDepth(const Transform &trans, const mat4 &initTrans)
{
	mat4 model = mat4::identitiy();
	model = initTrans;

	for (auto &i : m_meshes)
	{
		i.renderDepth(trans, model);
	}
}

void Model::loadingDone()
{
	for (auto &i : m_meshes)
	{
		i.buildVBO();
	}
}