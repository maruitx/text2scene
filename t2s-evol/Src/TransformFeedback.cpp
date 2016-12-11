#include "TransformFeedback.h"
#include "VertexBufferObject.h"
#include "Shader.h"

TransformFeedback::TransformFeedback()
{	
	initShader();
	initBuffer();
}

TransformFeedback::~TransformFeedback()
{

}

void TransformFeedback::initBuffer()
{
	uint nrVertices = 400000;

	VertexBufferObject::DATA *data = new VertexBufferObject::DATA[nrVertices];
	memset(data, 0, sizeof(VertexBufferObject::DATA)*nrVertices);

	m_vboFeedBack = new VertexBufferObject();
	m_vboFeedBack->setData(data, GL_STATIC_DRAW, nrVertices, GL_POINTS);

	delete[] data;
}

void TransformFeedback::initShader()
{
	GLchar const * Strings[] = { "GeomPosition", "GeomNormal", "GeomColor", "GeomTexture" };

	m_shaderFeedback = new Shader();
	m_shaderFeedback->attachVertexShader("Shader/Feedback.vert.glsl");
	m_shaderFeedback->attachGeometryShader("Shader/Feedback.geom.glsl");
	m_shaderFeedback->setUpdateStatus(Shader::AUTO_LOAD);

	glTransformFeedbackVaryings(m_shaderFeedback->id(), 4, Strings, GL_INTERLEAVED_ATTRIBS);
	m_shaderFeedback->link();

	glBindAttribLocation(m_shaderFeedback->id(), VERTEX_POSITION, "Position");
	glBindAttribLocation(m_shaderFeedback->id(), VERTEX_NORMAL, "Normal");
	glBindAttribLocation(m_shaderFeedback->id(), VERTEX_COLOR, "Color");
	glBindAttribLocation(m_shaderFeedback->id(), VERTEX_TEXTURE, "Texture");
}

int TransformFeedback::renderToBuffer(VertexBufferObject *vbo, const Transform &trans)
{
	uint query;
	glGenQueries(1, &query);

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

	glEnable(GL_RASTERIZER_DISCARD);
	m_shaderFeedback->bind();

		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_vboFeedBack->id());
		m_vboFeedBack->bind();

		mat4 model = mat4::identitiy();
		m_shaderFeedback->setMatrix("matModel", model, GL_TRUE);

		glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query);
		glBeginTransformFeedback(GL_POINTS);

		vbo->render();

		glEndTransformFeedback();
		glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

	m_shaderFeedback->release();
	glDisable(GL_RASTERIZER_DISCARD);


	GLuint primitivesWritten = 0;
	glGetQueryObjectuiv(query, GL_QUERY_RESULT, &primitivesWritten);

	glPopClientAttrib();
	glPopAttrib();

	return primitivesWritten;
}

void TransformFeedback::saveAsObj(QString path, int nr)
{
	vector<vec3> positions;
	vector<vec3> normals;
	vector<vec3> texCoords;
	vector<vec3> colors;

	glBindBuffer(GL_ARRAY_BUFFER, m_vboFeedBack->id());
	VertexBufferObject::DATA* attrData = (VertexBufferObject::DATA*)glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);

	if (attrData)
	{
		for (uint i = 0; i<nr; ++i)
		{
			vec3 p = vec3(attrData[i].vx, attrData[i].vy, attrData[i].vz);
			vec3 n = vec3(attrData[i].nx, attrData[i].ny, attrData[i].nz);
			vec3 c = vec3(attrData[i].cx, attrData[i].cy, attrData[i].cz);
			vec3 t = vec3(attrData[i].tx, attrData[i].ty, attrData[i].tz);

			positions.push_back(p);
			normals.push_back(n);
			texCoords.push_back(t);
			colors.push_back(c);
		}

		glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
	}

	vector<Face> faces;
	for (int i = 0; i<positions.size() - 2; i += 3)
	{
		Face f;
		f.a = i + 0 + 1;
		f.b = i + 1 + 1;
		f.c = i + 2 + 1;

		faces.push_back(f);
	}

	QFile file(path);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;

	QTextStream out(&file);

	for (int i = 0; i<positions.size(); ++i)
	{
		vec3 p = positions[i];
		out << "v " << p.x << " " << p.y << " " << p.z << endl;
	}

	for (int i = 0; i<normals.size(); ++i)
	{
		vec3 n = normals[i];
		out << "vn " << n.x << " " << n.y << " " << n.z << endl;
	}

	for (int i = 0; i<texCoords.size(); ++i)
	{
		vec3 t = normals[i];
		out << "vt " << t.x << " " << t.y << endl;
	}

	for (int i = 0; i<colors.size(); ++i)
	{
		vec3 c = colors[i];
		out << "vc " << c.x << " " << c.y << " " << c.z << endl;
	}

	for (int i = 0; i<faces.size(); ++i)
	{
		Face f = faces[i];

		out << "f " << f.a << "/" << f.a << "/" << f.a << " " << f.b << "/" << f.b << "/" << f.b
			<< " " << f.c << "/" << f.c << "/" << f.c << endl;
	}

	file.close();
	exit(0);
}

void TransformFeedback::printBuffer(int nr)
{
	glBindBuffer(GL_ARRAY_BUFFER, m_vboFeedBack->id());
	VertexBufferObject::DATA* attrData = (VertexBufferObject::DATA*)glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);

	if (attrData)
	{
		for (uint i = 0; i<nr; ++i)
		{
			qDebug() << "pos:" << attrData[i].vx << "," << attrData[i].vy << "," << attrData[i].vz << "," << attrData[i].vw;
		}

		glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
	}
}