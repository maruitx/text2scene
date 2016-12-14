#include "Object.h"
#include "Shader.h"
#include "VertexBufferObject.h"
#include "Mesh.h"

void ObjectThread::load()
{
    QFileInfo fi(m_fileName);
    QString fn = fi.fileName();
    QString baseName = m_fileName;
    baseName.replace(fn, "");

    vector<tinyobj::shape_t> shapes;
    vector<tinyobj::material_t> materials;
    
    string err;
	bool ret = tinyobj::LoadObj(shapes, materials, err, m_fileName.toStdString().c_str(), baseName.toStdString().c_str(), true);

    if (err.length() > 0)
    {
        qDebug() << err.c_str();
    }

    vector<Material> mats;
    if (materials.size() > 0)
    {
        for (size_t i = 0; i < materials.size(); i++)
        {
            materials[i].name;

            vec3 Ka = vec3(materials[i].ambient[0], materials[i].ambient[1], materials[i].ambient[2]);
            vec3 Kd = vec3(materials[i].diffuse[0], materials[i].diffuse[1], materials[i].diffuse[2]);
            vec3 Ks = vec3(materials[i].specular[0], materials[i].specular[1], materials[i].specular[2]);
            float Ns = materials[i].shininess;
            QString dTexName = baseName + QString(materials[i].diffuse_texname.c_str());
            mats.push_back(Material(Ka, Kd, Ks, Ns, dTexName));
        }
    }

    for (size_t i = 0; i < shapes.size(); i++)
    {
        vector<uint> &indices = shapes[i].mesh.indices;

        vec3 p, n;
        vec2 t;
        vector<Vertex> vertices;

        for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++)
        {
            p = vec3(shapes[i].mesh.positions[3 * v + 0], shapes[i].mesh.positions[3 * v + 1], shapes[i].mesh.positions[3 * v + 2]);

            int n1 = 3 * v + 0, n2 = 3 * v + 1, n3 = 3 * v + 2;
            int t1 = 2 * v + 0, t2 = 2 * v + 1;

            if (n1 < shapes[i].mesh.normals.size() && n2 < shapes[i].mesh.normals.size() && n3 < shapes[i].mesh.normals.size())
            {
                n = vec3(shapes[i].mesh.normals[n1], shapes[i].mesh.normals[n2], shapes[i].mesh.normals[n3]);
            }

            if (t1 < shapes[i].mesh.texcoords.size() && t2 < shapes[i].mesh.texcoords.size())
            {
                t = vec2(shapes[i].mesh.texcoords[t1], shapes[i].mesh.texcoords[t2]);
            }

            vertices.push_back(Vertex(p, n, vec4(), t));
        }

        Material defMat(vec3(0.0f), vec3(1.0, 0.2, 0.2), vec3(0.2), 10);

        if (shapes[i].mesh.material_ids.size() > 0 && shapes[i].mesh.material_ids[0] != -1)
        {
            int matId = shapes[i].mesh.material_ids[0];
            m_materials.push_back(mats[matId]);
        }
        else
        {
            m_materials.push_back(defMat);
        }

        m_allIndices.push_back(indices);
        m_allVertices.push_back(vertices);
    }

    if (m_normalize)
    {
        normalizeGeometry(m_allVertices, m_position, m_scale, m_rotation);
        normalizeGeometry(m_allVertices, vec3(0.0f), vec3(1.0f), vec4(0.0f));
    }
}

void ObjectThread::buildMeshData(vector<Vertex> &vertices, vector<uint> &indices)
{
    VertexBufferObject::DATA *data = new VertexBufferObject::DATA[vertices.size()];

    for (uint i = 0; i < vertices.size(); ++i)
    {
        vec3 v = vertices[i].position;
        vec3 n = normalize(vertices[i].normal);
        vec2 t = vertices[i].texture;

        data[i].vx = v.x;
        data[i].vy = v.y;
        data[i].vz = v.z;
        data[i].vw = 0.0f;

        data[i].cx = 1.0f;
        data[i].cy = 1.0f;
        data[i].cz = 1.0f;
        data[i].cw = 1.0f;

        data[i].nx = n.x;
        data[i].ny = n.y;
        data[i].nz = n.z;
        data[i].nw = 0.0f;

        data[i].tx = t.x;
        data[i].ty = t.y;
        data[i].tz = 0.0f;
        data[i].tw = 0.0f;
    }

    uint *idxdata = new uint[indices.size()];
    for (int i = 0; i<indices.size(); ++i)
    {
        idxdata[i] = indices[i];
    }

    ObjectMetaData meta;
    meta.data = data;
    meta.indices = idxdata;
    meta.nrData = vertices.size();
    meta.nrIndices = indices.size();
    meta.boundingBox = m_bb;

    m_meshData.push_back(meta);
}

void ObjectThread::buildLinesData(vector<Vertex> &vertices, vector<uint> &indices)
{
    vector<Vertex> tmp;
    for (uint i = 0; i<indices.size() - 3; i += 3)
    {
        Vertex &a = vertices[indices[i]];
        Vertex &b = vertices[indices[i + 1]];
        Vertex &c = vertices[indices[i + 2]];

        tmp.push_back(a);
        tmp.push_back(b);

        tmp.push_back(a);
        tmp.push_back(c);

        tmp.push_back(b);
        tmp.push_back(c);
    }

    VertexBufferObject::DATA *data = new VertexBufferObject::DATA[tmp.size()];

    for (uint i = 0; i<tmp.size(); ++i)
    {
        vec3 v = tmp[i].position;
        vec3 n = tmp[i].normal;
        vec2 t = tmp[i].texture;

        data[i].vx = v.x;
        data[i].vy = v.y;
        data[i].vz = v.z;
        data[i].vw = 0.0f;

        data[i].cx = 0.0f;
        data[i].cy = 0.0f;
        data[i].cz = 0.0f;
        data[i].cw = 1.0f;

        data[i].nx = n.x;
        data[i].ny = n.y;
        data[i].nz = n.z;
        data[i].nw = 0.0f;

        data[i].tx = t.x;
        data[i].ty = t.y;
        data[i].tz = 0.0f;
        data[i].tw = 0.0f;
    }

    ObjectMetaData meta;
    meta.data = data;
    meta.indices = nullptr;
    meta.nrData = tmp.size();
    meta.nrIndices = 0;
    meta.boundingBox = BoundingBox();

    m_lineData.push_back(meta);
}

void ObjectThread::buildNormalsData(vector<Vertex> &vertices, vector<uint> &indices)
{
    float s = 0.1;
    vector<vec3> positions;
    for (uint i = 0; i<indices.size() - 3; i += 3)
    {
        Vertex &a = vertices[indices[i]];
        Vertex &b = vertices[indices[i + 1]];
        Vertex &c = vertices[indices[i + 2]];

        positions.push_back(a.position);
        positions.push_back(a.position + a.normal * s);

        positions.push_back(b.position);
        positions.push_back(b.position + b.normal * s);

        positions.push_back(c.position);
        positions.push_back(c.position + c.normal * s);
    }

    VertexBufferObject::DATA *data = new VertexBufferObject::DATA[positions.size()];

    for (uint i = 0; i<positions.size(); ++i)
    {
        vec3 v = positions[i];

        data[i].vx = v.x;
        data[i].vy = v.y;
        data[i].vz = v.z;
        data[i].vw = 0.0f;

        data[i].cx = v.x;
        data[i].cy = v.y;
        data[i].cz = v.z;
        data[i].cw = 1.0f;

        data[i].nx = 0.0f;
        data[i].ny = 0.0f;
        data[i].nz = 0.0f;
        data[i].nw = 0.0f;

        data[i].tx = 0.0f;
        data[i].ty = 0.0f;
        data[i].tz = 0.0f;
        data[i].tw = 0.0f;
    }

    ObjectMetaData meta;
    meta.data = data;
    meta.indices = nullptr;
    meta.nrData = positions.size();
    meta.nrIndices = 0;
    meta.boundingBox = BoundingBox();

    m_normalData.push_back(meta);
}

void ObjectThread::normalizeGeometry(vector<vector<Vertex>> &vertices, const vec3 &translate, const vec3 &scale, const vec4 &rotate)
{
    vec3 mi = vec3(math_maxfloat, math_maxfloat, math_maxfloat);
    vec3 ma = vec3(math_minfloat, math_minfloat, math_minfloat);

    for (int i = 0; i<vertices.size(); ++i)
    {
        for (int j = 0; j<vertices[i].size(); ++j)
        {
            vec3 &a = vertices[i][j].position;

            if (a.x > ma.x) ma.x = a.x;
            if (a.y > ma.y) ma.y = a.y;
            if (a.z > ma.z) ma.z = a.z;

            if (a.x < mi.x) mi.x = a.x;
            if (a.y < mi.y) mi.y = a.y;
            if (a.z < mi.z) mi.z = a.z;
        }
    }

    vec3 d = ma - mi;
    float s = max(d.x, max(d.y, d.z));

    vec3 shift = d / s / 2;
    for (int i = 0; i<vertices.size(); ++i)
    {
        for (int j = 0; j<vertices[i].size(); ++j)
        {
            vec3 &a = vertices[i][j].position;

            a -= mi;
            a /= s;
            a -= vec3(shift.x, 0.0f, shift.z);

            mat4 m = mat4::identitiy();
            m *= mat4::translate(translate);
            m *= mat4::rotate(rotate.w, vec3(rotate.x, rotate.y, rotate.z));
            m *= mat4::scale(scale);

            vec4 ta = m * vec4(a);
            vertices[i][j].position = vec3(ta.x, ta.y, ta.z);
        }
    }

    mi = vec3(math_maxfloat, math_maxfloat, math_maxfloat);
    ma = vec3(math_minfloat, math_minfloat, math_minfloat);

    for (int i = 0; i<vertices.size(); ++i)
    {
        for (int j = 0; j<vertices[i].size(); ++j)
        {
            vec3 &a = vertices[i][j].position;

            if (a.x > ma.x) ma.x = a.x;
            if (a.y > ma.y) ma.y = a.y;
            if (a.z > ma.z) ma.z = a.z;

            if (a.x < mi.x) mi.x = a.x;
            if (a.y < mi.y) mi.y = a.y;
            if (a.z < mi.z) mi.z = a.z;
        }
    }

    m_bb = BoundingBox(mi, ma);
}

void ObjectThread::run()
{
    load();

    for (int i = 0; i < m_allVertices.size(); ++i)
    {
        buildMeshData(m_allVertices[i], m_allIndices[i]);
        buildLinesData(m_allVertices[i], m_allIndices[i]);
        buildNormalsData(m_allVertices[i], m_allIndices[i]);
    }
}

Object::Object(const QString &fileName, bool normalize, bool buildLineVBO, bool buildNormalVBO, const vec3 &pos, const vec3 &scale, const vec4 &rot, const vec4 &color)
: m_fileName(fileName),
  m_position(pos),
  m_scale(scale),
  m_rotation(rot),
  m_color(color),
  m_isSelected(false),
  m_nrTriangles(0),
  m_nrVertices(0), 
  m_normalize(normalize), 
  m_lines(buildLineVBO), 
  m_normals(buildNormalVBO), 
  m_objectThread(fileName, pos, scale, rot, normalize, m_meshData, m_lineData, m_normalData)
{    
    connect(&m_objectThread, SIGNAL(finished()), this, SLOT(loadingDone()));    

    //prepareData(m_fileName);
    //buildBuffers();
}

void Object::start()
{
    m_objectThread.start();
}

Object::~Object()
{
    for(int i=m_vbosTriangles.size()-1; i>=0; --i)
    {
        VertexBufferObject *vbo = m_vbosTriangles[i];
        delete vbo;
    }

    for(int i=m_vbosLines.size()-1; i>=0; --i)
    {
        VertexBufferObject *vbo = m_vbosLines[i];
        delete vbo;
    }

    for (int i = 0; i < m_meshData.size(); ++i)
    {
        ObjectMetaData &md = m_meshData[i];

        delete[] md.indices;
        delete[] md.data;
    }
}

void Object::prepareData(const QString &fileName)
{ 
    QFileInfo fi(fileName);
    QString fn = fi.fileName();
    QString baseName = fileName;
    baseName.replace(fn, "");

    vector<tinyobj::shape_t> shapes;
    vector<tinyobj::material_t> materials;

    string err;
    bool ret = tinyobj::LoadObj(shapes, materials, err, m_fileName.toStdString().c_str(), baseName.toStdString().c_str(), true);
    
    if(err.length() > 0)
    {
        qDebug() << "Error: " << err.c_str();
    }

    vector<Material> mats;
    if(materials.size() > 0)
    {
        for (size_t i = 0; i < materials.size(); i++) 
        {
            materials[i].name;

            vec3 Ka = vec3(materials[i].ambient[0], materials[i].ambient[1], materials[i].ambient[2]);
            vec3 Kd = vec3(materials[i].diffuse[0], materials[i].diffuse[1], materials[i].diffuse[2]);
            vec3 Ks = vec3(materials[i].specular[0], materials[i].specular[1], materials[i].specular[2]);
            float Ns = materials[i].shininess;
            QString dTexName = baseName + QString(materials[i].diffuse_texname.c_str());
            mats.push_back(Material(Ka, Kd, Ks, Ns, dTexName));
        }
    }

    for (size_t i = 0; i < shapes.size(); i++) 
    {
        vector<uint> &indices = shapes[i].mesh.indices;

        vec3 p, n;
        vec2 t;
        vector<Vertex> vertices;

        for(size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) 
        {
            p = vec3(shapes[i].mesh.positions[3*v+0], shapes[i].mesh.positions[3*v+1], shapes[i].mesh.positions[3*v+2]);

            int n1 = 3*v+0, n2 = 3*v+1, n3 = 3*v+2;
            int t1 = 2*v+0, t2 = 2*v+1;

            if(n1 < shapes[i].mesh.normals.size() && n2 < shapes[i].mesh.normals.size() && n3 < shapes[i].mesh.normals.size())
            {
                n = vec3(shapes[i].mesh.normals[n1], shapes[i].mesh.normals[n2], shapes[i].mesh.normals[n3]);
            }

            if(t1 < shapes[i].mesh.texcoords.size() && t2 < shapes[i].mesh.texcoords.size())
            {
                t = vec2(shapes[i].mesh.texcoords[t1], shapes[i].mesh.texcoords[t2]);
            }

            vertices.push_back(Vertex(p, n, vec4(), t));            
        }

        Material defMat(vec3(0.0f), vec3(1.0, 0.2, 0.2), vec3(0.2), 10);

        if(shapes[i].mesh.material_ids.size() > 0 && shapes[i].mesh.material_ids[0] != -1)
        {
            int matId = shapes[i].mesh.material_ids[0];
            m_materials.push_back(mats[matId]);
        }
        else
        {
            m_materials.push_back(defMat);
        }

        m_allIndices.push_back(indices);
        m_allVertices.push_back(vertices);
    }

    if(m_normalize)
    {
        normalizeGeometry(m_allVertices, m_position, m_scale, m_rotation);
        //normalizeGeometry(allVertices, vec3(0.0f), vec3(1.0f), vec4(0.0f));
    }


}

void Object::buildBuffers()
{
    for (int i = 0; i<m_allVertices.size(); ++i)
    {
        buildVBOMesh(m_allVertices[i], m_allIndices[i]);

        if (m_lines)
        {
            buildVBOLines(m_allVertices[i], m_allIndices[i]);
        }

        if (m_normals)
        {
            buildVBONormals(m_allVertices[i], m_allIndices[i]);
        }
    }
}

void Object::buildBuffersFromData()
{
    for (int i = 0; i < m_meshData.size(); ++i)
    {
        buildVBOMeshFromData(m_meshData[i]);

        if (m_lines && m_lineData.size() > 0)
        {
            buildVBOLinesFromData(m_lineData[i]);
        }

        if (m_normals && m_normalData.size() > 0)
        {
            buildVBONormalsFromData(m_normalData[i]);
        }
    }
}

void Object::buildVBOMeshFromData(ObjectMetaData &meta)
{
    VertexBufferObject* vbo = new VertexBufferObject();
    vbo->setData(meta.data, GL_STATIC_DRAW, meta.nrData, GL_TRIANGLES);
    vbo->setIndexData(meta.indices, GL_STATIC_DRAW, meta.nrIndices);
    vbo->bindDefaultAttribs();

    m_vbosTriangles.push_back(vbo);
}

void Object::buildVBOLinesFromData(ObjectMetaData &meta)
{
    VertexBufferObject* vbo = new VertexBufferObject();
    vbo->setData(meta.data, GL_STATIC_DRAW, meta.nrData, GL_LINES);
    vbo->bindDefaultAttribs();

    m_vbosLines.push_back(vbo);
}

void Object::buildVBONormalsFromData(ObjectMetaData &meta)
{
    VertexBufferObject* vbo = new VertexBufferObject();
    vbo->setData(meta.data, GL_STATIC_DRAW, meta.nrData, GL_LINES);
    vbo->bindDefaultAttribs();

    m_vbosNormals.push_back(vbo);
}

void Object::render(const Transform &trans, const mat4 &model, const Material &material, bool applyShadow)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

    //if(!m_normalize)
    //{
    //    model = mat4::translate(m_position) * mat4::rotateY(m_rotation.y) * mat4::scale(m_scale); 
    //}

    if (params::inst()->applyCulling)
    {
        glCullFace(GL_BACK);
        glEnable(GL_CULL_FACE);
    }
    glEnable(GL_CLIP_DISTANCE0);    

    //glEnable(GL_POLYGON_OFFSET_FILL);
    //glPolygonOffset(params::inst()->polygonOffsetFactor, params::inst()->polygonOffsetUnits);

    //glDepthFunc(GL_LEQUAL);
    //glDepthRange(params::inst()->depthRangeMin, params::inst()->depthRangeMax);   

    if (params::inst()->renderMesh)
    {
        Shader *shader = shaders::inst()->object;
        shader->bind();

            shader->setMatrices(trans, model, true, true, true, true);
            shader->set3f("camPos", params::inst()->camPos);
            shader->seti("applyShadow", params::inst()->applyShadow && applyShadow);
            shader->setf("shadowIntensity", params::inst()->shadowIntensity);
            shader->seti("isSelected", m_isSelected);
            shader->set4f("clipPlane", params::inst()->clipPlaneGround);

            shader->setLights(params::inst()->lights);

            for (uint i = 0; i < m_vbosTriangles.size(); ++i)
            {
                shader->setMaterial(m_materials[i]);
                m_vbosTriangles[i]->render();
            }

        shader->release();
    }

    if(m_lines && params::inst()->renderWireframe)
    {
        Shader *shader = shaders::inst()->objectLines;
	    shader->bind();  

            shader->setMatrices(trans, model, true, true, true, false);

            for(uint i=0; i<m_vbosLines.size(); ++i)
            {
                m_vbosLines[i]->render();
            }

	    shader->release();
    }

    if(m_normals && params::inst()->renderNormals)
    {
        Shader *shader = shaders::inst()->default;
	    shader->bind();  

            shader->setMatrices(trans, model, true, true, true, false);

            for(uint i=0; i<m_vbosNormals.size(); ++i)
            {
                m_vbosNormals[i]->render();
            }

	    shader->release();
    }

	glDisable(GL_CULL_FACE);    

	glPopClientAttrib();
	glPopAttrib();
}

void Object::renderDepth(const Transform &trans, const mat4 &model)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

    if (params::inst()->renderMesh)
    {

        glEnable(GL_CLIP_DISTANCE0);

        //if(!m_normalize)
        //{
        //    model = mat4::translate(m_position) * mat4::rotateY(m_rotation.y) * mat4::scale(m_scale); 
        //}

        glDisable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        //glClearDepth(1.0);
        glEnable(GL_POLYGON_OFFSET_FILL);
        //glDepthFunc(GL_LEQUAL);

        //glDepthRange(params::inst()->depthRangeMin, params::inst()->depthRangeMax);
        glPolygonOffset(params::inst()->polygonOffsetFactor, params::inst()->polygonOffsetUnits);

        Shader *shader = shaders::inst()->objectDepth;
        shader->bind();

            shader->setMatrices(trans, model, true, true, true, false);
            shader->set4f("clipPlane", params::inst()->clipPlaneGround);

            for (uint i = 0; i < m_vbosTriangles.size(); ++i)
            {
                m_vbosTriangles[i]->render();
            }

        shader->release();

    }

	glPopClientAttrib();
	glPopAttrib();
}

void Object::buildVBOMesh(vector<Vertex> &vertices, vector<uint> &indices)
{
    VertexBufferObject::DATA *data = new VertexBufferObject::DATA[vertices.size()];

	for (uint i = 0; i<vertices.size(); ++i)
	{
		vec3 v = vertices[i].position;
		vec3 n = normalize(vertices[i].normal);
		vec2 t = vertices[i].texture;

		data[i].vx = v.x;
		data[i].vy = v.y;
		data[i].vz = v.z;
		data[i].vw = 0.0f;

        data[i].cx = m_color.x;
        data[i].cy = m_color.y;
        data[i].cz = m_color.z;
		data[i].cw = m_color.w;
			
        data[i].nx = n.x;
		data[i].ny = n.y;
		data[i].nz = n.z;
		data[i].nw = 0.0f;
            
		data[i].tx = t.x;
		data[i].ty = t.y;
        data[i].tz = 0.0f;
        data[i].tw = 0.0f;
	}

	VertexBufferObject* vbo = new VertexBufferObject();
	vbo->setData(data, GL_STATIC_DRAW, vertices.size(), GL_TRIANGLES);
    vbo->setIndexData(indices.data(), GL_STATIC_DRAW, indices.size());
    vbo->bindDefaultAttribs();

    m_vbosTriangles.push_back(vbo); 

	delete[] data;    
}

void Object::buildVBOLines(vector<Vertex> &vertices, vector<uint> &indices)
{
    vector<Vertex> tmp;
	for (uint i = 0; i<indices.size()-3; i+=3)
	{
        Vertex &a = vertices[indices[i]];
        Vertex &b = vertices[indices[i+1]];
        Vertex &c = vertices[indices[i+2]];

        tmp.push_back(a);
        tmp.push_back(b);

        tmp.push_back(a);
        tmp.push_back(c);

        tmp.push_back(b);
        tmp.push_back(c);
    }    

    VertexBufferObject::DATA *data = new VertexBufferObject::DATA[tmp.size()];

	for (uint i = 0; i<tmp.size(); ++i)
	{
		vec3 v = tmp[i].position;
		vec3 n = tmp[i].normal;
		vec2 t = tmp[i].texture;

		data[i].vx = v.x;
		data[i].vy = v.y;
		data[i].vz = v.z;
		data[i].vw = 0.0f;

        data[i].cx = m_color.x;
        data[i].cy = m_color.y;
        data[i].cz = m_color.z;
		data[i].cw = m_color.w;
			
        data[i].nx = n.x;
		data[i].ny = n.y;
		data[i].nz = n.z;
		data[i].nw = 0.0f;
            
		data[i].tx = t.x;
		data[i].ty = t.y;
        data[i].tz = 0.0f;
        data[i].tw = 0.0f;
	}

	VertexBufferObject* vbo = new VertexBufferObject();
	vbo->setData(data, GL_STATIC_DRAW, tmp.size(), GL_LINES);
    vbo->bindDefaultAttribs();
    m_vbosLines.push_back(vbo);

	delete[] data;    
}

void Object::buildVBONormals(vector<Vertex> &vertices, vector<uint> &indices)
{
    float s = 0.1;
    vector<vec3> positions;
	for (uint i = 0; i<indices.size()-3; i+=3)
	{
        Vertex &a = vertices[indices[i]];
        Vertex &b = vertices[indices[i+1]];
        Vertex &c = vertices[indices[i+2]];

        positions.push_back(a.position);
        positions.push_back(a.position + a.normal * s);

        positions.push_back(b.position);
        positions.push_back(b.position + b.normal * s);

        positions.push_back(c.position);
        positions.push_back(c.position + c.normal * s);
    }    

    VertexBufferObject::DATA *data = new VertexBufferObject::DATA[positions.size()];

	for (uint i = 0; i<positions.size(); ++i)
	{
		vec3 v = positions[i];

		data[i].vx = v.x;
		data[i].vy = v.y;
		data[i].vz = v.z;
		data[i].vw = 0.0f;

        data[i].cx = v.x;
        data[i].cy = v.y;
        data[i].cz = v.z;
		data[i].cw = 1.0f;
			
        data[i].nx = 0.0f;
		data[i].ny = 0.0f;
		data[i].nz = 0.0f;
		data[i].nw = 0.0f;
            
		data[i].tx = 0.0f;
		data[i].ty = 0.0f;
        data[i].tz = 0.0f;
        data[i].tw = 0.0f;
	}

	VertexBufferObject* vbo = new VertexBufferObject();
	vbo->setData(data, GL_STATIC_DRAW, positions.size(), GL_LINES);
    vbo->bindDefaultAttribs();
    m_vbosNormals.push_back(vbo);

	delete[] data;    
}

void Object::normalizeGeometry(vector<vector<Vertex>> &vertices, const vec3 &translate, const vec3 &scale, const vec4 &rotate)
{
	vec3 mi = vec3(math_maxfloat, math_maxfloat, math_maxfloat);
	vec3 ma = vec3(math_minfloat, math_minfloat, math_minfloat);

	for (int i = 0; i<vertices.size(); ++i)
	{
        for(int j=0; j<vertices[i].size(); ++j)
        {
		    vec3 &a = vertices[i][j].position;

		    if (a.x > ma.x) ma.x = a.x;
		    if (a.y > ma.y) ma.y = a.y;
		    if (a.z > ma.z) ma.z = a.z;

		    if (a.x < mi.x) mi.x = a.x;
		    if (a.y < mi.y) mi.y = a.y;
		    if (a.z < mi.z) mi.z = a.z;
        }
	}    

	vec3 d = ma - mi;
	float s = max(d.x, max(d.y, d.z));

	vec3 shift = d / s /2;
	for (int i = 0; i<vertices.size(); ++i)
	{
        for(int j=0; j<vertices[i].size(); ++j)
        {
		    vec3 &a = vertices[i][j].position;

		    a -= mi;
		    a /= s;
		    a -= vec3(shift.x, 0.0f, shift.z);

		    mat4 m = mat4::identitiy();
		    m *= mat4::translate(translate);
            m *= mat4::rotate(rotate.w, vec3(rotate.x, rotate.y, rotate.z));
		    m *= mat4::scale(scale);

		    vec4 ta = m * vec4(a);
		    vertices[i][j].position = vec3(ta.x, ta.y, ta.z);
        }
	}

	mi = vec3(math_maxfloat, math_maxfloat, math_maxfloat);
	ma = vec3(math_minfloat, math_minfloat, math_minfloat);

	for (int i = 0; i<vertices.size(); ++i)
	{
        for(int j=0; j<vertices[i].size(); ++j)
        {
		    vec3 &a = vertices[i][j].position;

		    if (a.x > ma.x) ma.x = a.x;
		    if (a.y > ma.y) ma.y = a.y;
		    if (a.z > ma.z) ma.z = a.z;

		    if (a.x < mi.x) mi.x = a.x;
		    if (a.y < mi.y) mi.y = a.y;
		    if (a.z < mi.z) mi.z = a.z;
        }
	} 

    m_bb = BoundingBox(mi, ma);
}

float Object::selected(Picking &pick, const Transform &trans, int sw, int sh, int mx, int my)
{     
    mat4 model = mat4::identitiy();        

    if(!m_normalize)
    {
        model = mat4::translate(m_position) * mat4::rotateY(m_rotation.y) * mat4::scale(m_scale);
    }
        
    return pick.select(trans, model, m_bb.mi(), m_bb.ma(), sw, sh, mx, my);  
}

void Object::move(int x, int y, const vec3 &dir, const vec3 &up, const vec3 &right, const vec3 &pos)
{
	vec3 movZ, movX, movY;	
	vec3 onPlane = cross(right, vec3(0, 1, 0));
    vec3 p = m_position;

    movY = vec3(0, p.y, 0);
    movX = vec3(p.x, 0, 0) + right * x * 0.1;
    movZ = vec3(0, 0, p.z) + onPlane * y * 0.1;
        	
    vec3 result = movX + movY + movZ ;                    
    m_position = result;    
}

void Object::loadingDone()
{
    m_materials = m_objectThread.m_materials;
    for (int i = 0; i < m_materials.size(); ++i)
    {
        m_materials[i].initTexture();
    }

    buildBuffersFromData();
}