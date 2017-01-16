#include "Model.h"
#include "Shader.h"
#include "VertexBufferObject.h"
#include "Mesh.h"
#include "Light.h"
#include "Utility.h"
#include "TriangleIntersection.h"

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

    //Currently keeping the geometry data
	//m_vertices.clear();
	//m_indices.clear();
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

void ModelMesh::render(const Transform &trans, Shader *shader, const string &textureDir)
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
			// if there is no texture dir specified, use the global one
			string td;
			if (textureDir == "")
			{
				td = params::inst()->textureDirectory;
			}
			else
			{
				td = textureDir;
			}

			string path = td + m_material.texName;
			
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
			//glCullFace(GL_BACK);
			//glEnable(GL_CULL_FACE);
		}		

    	m_vbo->render();
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

	//string modelFile = params::inst()->modelDirectory + baseName + string(".obj");
	//string materialFile = params::inst()->modelDirectory + baseName + string(".mtl");

	string modelFile = fileName;
	string materialFile = fi.path().toStdString() + "/" + baseName + string(".mtl");

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

    vec3 mi = vec3(math_maxfloat);
    vec3 ma = vec3(math_minfloat);

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

            if(mi.x > curVertex.vx)
                mi.x = curVertex.vx;
            if(mi.y > curVertex.vy)
                mi.y = curVertex.vy;
            if(mi.z > curVertex.vz)
                mi.z = curVertex.vz;

            if(ma.x < curVertex.vx)
                ma.x = curVertex.vx;
            if(ma.y < curVertex.vy)
                ma.y = curVertex.vy;
            if(ma.z < curVertex.vz)
                ma.z = curVertex.vz;    

            m_bb.setMinMax(mi, ma);
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
: m_thread(fileName, m_meshes, m_bb), 
  m_vboBB(nullptr), 
  m_bb(vec3(math_maxfloat), vec3(math_minfloat)),
  m_loadingDone(false)
  //m_collisionTrans(vec3())
{
	//Code for parallel loading
	connect(&m_thread, SIGNAL(finished()), this, SLOT(loadingDone()));
	m_thread.start();

	//Code for sequential loading
	//m_thread.run();
	//for (auto &i : m_meshes)
	//	i.buildVBO();
}

void Model::render(const Transform &trans, const mat4 &initTrans, bool applyShadow, const string &textureDir, int renderMode, int isSelected)
{
	if (params::inst()->applyCulling)
	{
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);
	}

    //mat4 matCollision = mat4::translate(m_collisionTrans);
	mat4 viewTrans = mat4::scale(params::inst()->globalSceneViewScale) * mat4::rotateX(-90);
	//mat4 m = matCollision * viewTrans * initTrans;
	mat4 m = viewTrans * initTrans;

	Shader *shader = shaders::inst()->model;
	shader->bind();
		shader->setMatrices(trans, m, true, true, true, true, true);

		shader->set3f("lightPos", params::inst()->lights[0]->position());
		shader->setTexture("shadowMap", params::inst()->lights[0]->shadowMapBlurredId());
		shader->seti("applyShadow", params::inst()->applyShadow && applyShadow ? 1 : 0);
        shader->seti("renderMode", renderMode);
        shader->seti("isSelected", isSelected);

		for (auto &i : m_meshes)
		{
			i.render(trans, shader, textureDir);
		}

	shader->release();


    if(m_vboBB && params::inst()->renderObjectBB)
    {
	    shader = shaders::inst()->default;
	    shader->bind();
		    shader->setMatrices(trans, m, true, true, true, true, true);
       
		    m_vboBB->render();

        shader->release();
    }

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Model::renderDepth(const Transform &trans, const mat4 &initTrans)
{
    //mat4 matCollision = mat4::translate(m_collisionTrans);
	mat4 viewTrans = mat4::scale(params::inst()->globalSceneViewScale) * mat4::rotateX(-90);
    //mat4 m = matCollision * viewTrans * initTrans;
	mat4 m =  viewTrans * initTrans;

	Shader *shader = shaders::inst()->modelDepth;
	shader->bind();
	    shader->setMatrices(trans, m, true, true, true, true, true);

		    for (auto &i : m_meshes)
		    {
			    i.renderDepth(trans);
		    }

	shader->release();
}

void Model::loadingDone()
{
	for (auto &i : m_meshes)
	{
		i.buildVBO();
	}

    buildBBVBO();
	
	m_loadingDone = true;
}

void Model::buildBBVBO()
{
    vec3 mi = m_bb.mi();
    vec3 ma = m_bb.ma();

    vec3 a = vec3(mi.x, mi.y, mi.z);
    vec3 b = vec3(ma.x, mi.y, mi.z);
    vec3 c = vec3(ma.x, mi.y, ma.z);
    vec3 d = vec3(mi.x, mi.y, ma.z);

    vec3 e = vec3(mi.x, ma.y, mi.z);
    vec3 f = vec3(ma.x, ma.y, mi.z);
    vec3 g = vec3(ma.x, ma.y, ma.z);
    vec3 h = vec3(mi.x, ma.y, ma.z);

    vector<vec3> vertices;
    
    vertices.push_back(a);
    vertices.push_back(b);
    vertices.push_back(b);
    vertices.push_back(c);
    vertices.push_back(c);
    vertices.push_back(d);
    vertices.push_back(d);
    vertices.push_back(a);

    vertices.push_back(e);
    vertices.push_back(f);
    vertices.push_back(f);
    vertices.push_back(g);
    vertices.push_back(g);
    vertices.push_back(h);
    vertices.push_back(h);
    vertices.push_back(e);

    vertices.push_back(a);
    vertices.push_back(e);
    vertices.push_back(b);
    vertices.push_back(f);
    vertices.push_back(c);
    vertices.push_back(g);
    vertices.push_back(d);
    vertices.push_back(h);

   uint nrVertices = vertices.size();
   VertexBufferObject::DATA *data = new VertexBufferObject::DATA[nrVertices];

    for(uint i=0; i<nrVertices; ++i)
    {    
        vec3 v = vertices[i];

        data[i].vx = v.x;
        data[i].vy = v.y;
        data[i].vz = v.z;
        data[i].vw = 1.0f;
        
        data[i].nx = 0.0f;
        data[i].ny = 0.0f;
        data[i].nz = 0.0f;
        data[i].nw = 0.0f;
        
        data[i].cx = 0.0f;
        data[i].cy = 0.0f;
        data[i].cz = 0.0f;
        data[i].cw = 1.0f;
        
        data[i].tx = 0.0f;
        data[i].ty = 0.0f;
        data[i].tz = 0.0f;
		data[i].tw = 0.0f;
    }    

	m_vboBB = new VertexBufferObject();
	m_vboBB->setData(data, GL_STATIC_DRAW, nrVertices, GL_LINES);
	m_vboBB->bindDefaultAttribs();

    delete data;
}

//used to map from vec3 to [3]
bool triangleTriangleWrapper(const vec3 &a1, const vec3 &a2, const vec3 &a3, const vec3 &b1, const vec3 &b2, const vec3 &b3)
{    
    float p1[3], q1[3], r1[3];
    float p2[3], q2[3], r2[3];
    
    p1[0] = a1.x;
    p1[1] = a1.y;
    p1[2] = a1.z;

    q1[0] = a2.x;
    q1[1] = a2.y;
    q1[2] = a2.z;

    r1[0] = a3.x;
    r1[1] = a3.y;
    r1[2] = a3.z;

    p2[0] = b1.x;
    p2[1] = b1.y;
    p2[2] = b1.z;

    q2[0] = b2.x;
    q2[1] = b2.y;
    q2[2] = b2.z;

    r2[0] = b3.x;
    r2[1] = b3.y;
    r2[2] = b3.z;
    
    return tri_tri_intersect(p1, q1, r1, p2, q2, r2);
}

bool Model::checkCollisionBBTriangles(const BoundingBox &testBB, const mat4 &testModelTransMat, const mat4 &refModelTransMat, double delta /*= 0*/)
{	
    //box triangle collision as triangle triangle collision

    //vec3 b1 = vec3(testBB.mi().x, testBB.mi().y, testBB.mi().z);
    //vec3 b2 = vec3(testBB.ma().x, testBB.mi().y, testBB.mi().z);
    //vec3 b3 = vec3(testBB.ma().x, testBB.mi().y, testBB.ma().z);
    //vec3 b4 = vec3(testBB.mi().x, testBB.mi().y, testBB.ma().z);

    //vec3 b5 = vec3(testBB.mi().x, testBB.ma().y, testBB.mi().z);
    //vec3 b6 = vec3(testBB.ma().x, testBB.ma().y, testBB.mi().z);
    //vec3 b7 = vec3(testBB.ma().x, testBB.ma().y, testBB.ma().z);
    //vec3 b8 = vec3(testBB.mi().x, testBB.ma().y, testBB.ma().z);

	vec3 b1 = vec3(testBB.mi().x, testBB.mi().y, testBB.mi().z);
	vec3 b2 = vec3(testBB.ma().x, testBB.mi().y, testBB.mi().z);
	vec3 b3 = vec3(testBB.ma().x, testBB.ma().y, testBB.mi().z);
	vec3 b4 = vec3(testBB.mi().x, testBB.ma().y, testBB.mi().z);

	vec3 b5 = vec3(testBB.mi().x, testBB.mi().y, testBB.ma().z);
	vec3 b6 = vec3(testBB.ma().x, testBB.mi().y, testBB.ma().z);
	vec3 b7 = vec3(testBB.ma().x, testBB.ma().y, testBB.ma().z);
	vec3 b8 = vec3(testBB.mi().x, testBB.ma().y, testBB.ma().z);

	b1 = TransformPoint(testModelTransMat, b1);
	b2 = TransformPoint(testModelTransMat, b2);
	b3 = TransformPoint(testModelTransMat, b3);
	b4 = TransformPoint(testModelTransMat, b4);
	b5 = TransformPoint(testModelTransMat, b5);
	b6 = TransformPoint(testModelTransMat, b6);
	b7 = TransformPoint(testModelTransMat, b7);
	b8 = TransformPoint(testModelTransMat, b8);

    vector<tuple<vec3, vec3, vec3>> triangles;
    
    //BB Triangles

    //bottom
    triangles.push_back(make_tuple(b1, b2, b4));
    triangles.push_back(make_tuple(b2, b3, b4));

    //top
    triangles.push_back(make_tuple(b5, b6, b8));
    triangles.push_back(make_tuple(b6, b7, b8));

    //right
    triangles.push_back(make_tuple(b2, b3, b6));
    triangles.push_back(make_tuple(b3, b7, b6));

    //left
    triangles.push_back(make_tuple(b4, b1, b5));
    triangles.push_back(make_tuple(b4, b5, b8));

    //front
    triangles.push_back(make_tuple(b1, b2, b6));
    triangles.push_back(make_tuple(b6, b5, b1));

    //back
    triangles.push_back(make_tuple(b3, b4, b8));
    triangles.push_back(make_tuple(b8, b7, b3));
    
    bool iscollide = false;

	for (int i = 0; i < m_meshes.size(); ++i)
	{
		vector<VertexBufferObject::DATA> &vertices = m_meshes[i].m_vertices;
        vector<uint> &indices = m_meshes[i].m_indices;

        for(int j=0; j<indices.size()-3; j+=3)
        {
            VertexBufferObject::DATA &d1 = vertices[indices[j]];
            VertexBufferObject::DATA &d2 = vertices[indices[j+1]];
            VertexBufferObject::DATA &d3 = vertices[indices[j+2]];

            vec3 v1 = TransformPoint(refModelTransMat, vec3(d1.vx, d1.vy, d1.vz));
            vec3 v2 = TransformPoint(refModelTransMat, vec3(d2.vx, d2.vy, d2.vz));
            vec3 v3 = TransformPoint(refModelTransMat, vec3(d3.vx, d3.vy, d3.vz));

            for(int k=0; k<triangles.size(); ++k)
            {
                auto &t = triangles[k];

                vec3 w1 = get<0>(t);
                vec3 w2 = get<1>(t);
                vec3 w3 = get<2>(t);

                bool iscollide = triangleTriangleWrapper(v1, v2, v3, w1, w2, w3);
                
                if(iscollide)
                    return true;
            }
        }                
	}

	return false;
}

bool Model::checkCollisionTrianglesTriangles(Model *testModel, const mat4 &testModelTransMat, const mat4 &refModelTransMat, double delta /*= 0*/)
{
	vector<ModelMesh>& testModelMeshes = testModel->getModelMeshs();

	for (int ti = 0; ti < testModelMeshes.size();ti++)
	{
		vector<VertexBufferObject::DATA> &testVertices = testModelMeshes[ti].m_vertices;
		vector<uint> &testIndices = testModelMeshes[ti].m_indices;

		for (int tj = 0; tj < testIndices.size()-3; tj+=3)
		{
			VertexBufferObject::DATA &d1 = testVertices[testIndices[tj]];
			VertexBufferObject::DATA &d2 = testVertices[testIndices[tj + 1]];
			VertexBufferObject::DATA &d3 = testVertices[testIndices[tj + 2]];

			vec3 v1 = TransformPoint(testModelTransMat, vec3(d1.vx, d1.vy, d1.vz));
			vec3 v2 = TransformPoint(testModelTransMat, vec3(d2.vx, d2.vy, d2.vz));
			vec3 v3 = TransformPoint(testModelTransMat, vec3(d3.vx, d3.vy, d3.vz));

			for (int ri = 0; ri < m_meshes.size(); ri++)
			{
				vector<VertexBufferObject::DATA> &vertices = m_meshes[ri].m_vertices;
				vector<uint> &indices = m_meshes[ri].m_indices;

				for (int rj = 0; rj < indices.size() - 3; rj += 3)
				{
					VertexBufferObject::DATA &d1 = vertices[indices[rj]];
					VertexBufferObject::DATA &d2 = vertices[indices[rj + 1]];
					VertexBufferObject::DATA &d3 = vertices[indices[rj + 2]];

					vec3 w1 = TransformPoint(refModelTransMat, vec3(d1.vx, d1.vy, d1.vz));
					vec3 w2 = TransformPoint(refModelTransMat, vec3(d2.vx, d2.vy, d2.vz));
					vec3 w3 = TransformPoint(refModelTransMat, vec3(d3.vx, d3.vy, d3.vz));

					bool iscollide = triangleTriangleWrapper(v1, v2, v3, w1, w2, w3);

					if (iscollide)
					{
						//qDebug() << "\nv1 ";
						//v1.print();
						//qDebug() << "v2 ";
						//v2.print();
						//qDebug() << "v3 ";
						//v3.print();

						//qDebug() << "w1 ";
						//w1.print();
						//qDebug() << "w2 ";
						//w2.print();
						//qDebug() << "w3 ";
						//w3.print();

						//qDebug() << QString("d1 %1 %2 %3\n").arg(d1.vx).arg(d1.vy).arg(d1.vz);
						//qDebug() << QString("d2 %1 %2 %3\n").arg(d2.vx).arg(d2.vy).arg(d2.vz);
						//qDebug() << QString("d3 %1 %2 %3\n").arg(d3.vx).arg(d3.vy).arg(d3.vz);

						//qDebug() << "RefModelTransMat ";
						//mat4 refM(refModelTransMat);
						//refM.print();

						//qDebug() << "TestModelTransMat ";
						//mat4 testM(testModelTransMat);
						//testM.print();

						return true;
					}
						
				}
			}
		}
	}

	return false;

}
