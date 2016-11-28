#include "CMesh.h"
#include "../utilities/utility.h"
#include <qgl.h>
#include <QFile>

CMesh::CMesh(QString path, QString name)
{
	m_name = name;
}

CMesh::~CMesh()
{
	m_vertices.clear();
	m_faces.clear();
	m_faceNormals.clear();
}

bool CMesh::readObjFile(const std::string &filename, const double metric /*= 1.0*/, QString sceneDbType)
{	
	MathLib::Vector3 currMetric(metric, metric, metric);

	char   s[200];
	float  x, y, z;
	//std::vector<Surface_mesh::Vertex>  vertices;
	std::vector<int> vertices;


	// open file (in ASCII mode)
	FILE* in = fopen(filename.c_str(), "r");
	if (!in) return false;


	// if mesh is not empty we need an offset for vertex indices
	// also take into accout that OBJ indices start at 1 (not 0)
	const int voffset = m_vertices.size() - 1;


	// clear line once
	memset(&s, 0, 200);


	// parse line by line (currently only supports vertex positions & faces
	while (in && !feof(in) && fgets(s, 200, in))
	{
		// comment
		if (s[0] == '#' || isspace(s[0])) continue;

		// vertex
		else if (strncmp(s, "v ", 2) == 0)
		{
			if (sscanf(s, "v %f %f %f", &x, &y, &z))
			{
				//this->add_vertex(MathLib::Vector3(x*m_metric, y*m_metric, z*m_metric));
				//this->add_vertex(Surface_mesh::Point(x, y, z));

				m_vertices.push_back(MathLib::Vector3(x*currMetric[0], y*currMetric[1], z*currMetric[2]));
			}
		}

		// face
		else if (strncmp(s, "f ", 2) == 0)
		{
			int component(0), nV(0);
			bool endOfVertex(false);
			char *p0, *p1(s + 1);

			vertices.clear();

			// skip white-spaces
			while (*p1 == ' ') ++p1;

			while (p1)
			{
				p0 = p1;

				// overwrite next separator

				// skip '/', '\n', ' ', '\0', '\r' <-- don't forget Windows
				while (*p1 != '/' && *p1 != '\r' && *p1 != '\n' && *p1 != ' ' && *p1 != '\0') ++p1;

				// detect end of vertex
				if (*p1 != '/')
				{
					endOfVertex = true;
				}

				// replace separator by '\0'
				if (*p1 != '\0')
				{
					*p1 = '\0';
					p1++; // point to next token
				}

				// detect end of line and break
				if (*p1 == '\0' || *p1 == '\n')
				{
					p1 = 0;
				}

				// read next vertex component
				if (*p0 != '\0')
				{
					switch (component)
					{
					case 0: // vertex
						vertices.push_back(atoi(p0) + voffset);
						break;

					case 1: // texture coord
						break;

					case 2: // normal
						break;
					}
				}

				++component;

				if (endOfVertex)
				{
					component = 0;
					nV++;
					endOfVertex = false;
				}
			}

			int v0 = vertices[0];
			int v1 = vertices[1];
			int v2 = vertices[2];

			if (!(v0 == v1 | v1 == v2 | v0 == v2))
				//this->add_face(vertices);
				m_faces.push_back(vertices);
		}


		// clear line
		memset(&s, 0, 200);
	}


	fclose(in);

	computeMinMaxVerts();
	computeFaceNormal();

	return true;
}

void CMesh::draw(QColor c)
{
	Eigen::Matrix4d displayTransMat = Eigen::Matrix4d::Identity();

	glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_HINT_BIT | GL_LINE_BIT | GL_CURRENT_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);

	glColor4d(c.redF(), c.greenF(), c.blueF(), c.alphaF());

	if (displayTransMat == Eigen::Matrix4d::Identity())
	{
		// draw front face
		glCullFace(GL_BACK);
		glBegin(GL_TRIANGLES);

		for (unsigned int f_id = 0; f_id < m_faces.size(); f_id++)
		{
			glNormal3dv(m_faceNormals[f_id].v);

			std::vector<int> vert_ids = m_faces[f_id];			
			for (int i = 0; i < 3; i++)
			{
				int v_id = vert_ids[i];
				glVertex3dv(m_vertices[v_id].v);
			}			
		}

		glEnd();


		// draw back face
		glCullFace(GL_FRONT);
		glBegin(GL_TRIANGLES);
		for (unsigned int f_id = 0; f_id < m_faces.size(); f_id++)
		{
			double* faceNormal = m_faceNormals[f_id].v;
			glNormal3d(-faceNormal[0], -faceNormal[1], -faceNormal[2]);

			std::vector<int> vert_ids = m_faces[f_id];
			for (int i = 0; i < 3; i++)
			{
				int v_id = vert_ids[i];
				glVertex3dv(m_vertices[v_id].v);
			}
		}
		glEnd();

	}
	else
	{

	}

	glPopAttrib();
}

void CMesh::draw(const std::vector<int> &faceIndicators)
{
	Eigen::Matrix4d displayTransMat = Eigen::Matrix4d::Identity();

	glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_HINT_BIT | GL_LINE_BIT | GL_CURRENT_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);

	if (displayTransMat == Eigen::Matrix4d::Identity())
	{
		// draw front face
		glCullFace(GL_BACK);
		glBegin(GL_TRIANGLES);

		for (unsigned int f_id = 0; f_id < m_faces.size(); f_id++)
		{
			QColor c;
			if (faceIndicators[f_id] < 0)
			{ 
				c = QColor(180, 180, 180, 255);
			}
			else
				c = GetColorFromSet(faceIndicators[f_id]);					
			
			glColor4d(c.redF(), c.greenF(), c.blueF(), c.alphaF());

			glNormal3dv(m_faceNormals[f_id].v);

			std::vector<int> vert_ids = m_faces[f_id];
			for (int i = 0; i < 3; i++)
			{
				int v_id = vert_ids[i];
				glVertex3dv(m_vertices[v_id].v);
			}
		}

		glEnd();


		// draw back face
		glCullFace(GL_FRONT);
		glBegin(GL_TRIANGLES);
		for (unsigned int f_id = 0; f_id < m_faces.size(); f_id++)
		{
			QColor c;
			if (faceIndicators[f_id] < 0)
			{
				c = QColor(180, 180, 180, 255);
			}
			else
				c = GetColorFromSet(faceIndicators[f_id]);

			glColor4d(c.redF(), c.greenF(), c.blueF(), c.alphaF());

			double* faceNormal = m_faceNormals[f_id].v;
			glNormal3d(-faceNormal[0], -faceNormal[1], -faceNormal[2]);

			std::vector<int> vert_ids = m_faces[f_id];
			for (int i = 0; i < 3; i++)
			{
				int v_id = vert_ids[i];
				glVertex3dv(m_vertices[v_id].v);
			}
		}
		glEnd();

	}
	else
	{

	}

	glPopAttrib();
}

void CMesh::computeMinMaxVerts()
{
	MathLib::Vector3 minVert(MAX_VALUE, MAX_VALUE, MAX_VALUE);
	MathLib::Vector3 maxVert(-MAX_VALUE, -MAX_VALUE, -MAX_VALUE);

	for (unsigned int v_id = 0; v_id < m_vertices.size(); v_id++)
	{
		for (int i = 0; i < 3; i++)
		{
			if (m_vertices[v_id][i] < minVert[i])
			{
				minVert[i] = m_vertices[v_id][i];
			}

			if (m_vertices[v_id][i] > maxVert[i])
			{
				maxVert[i] = m_vertices[v_id][i];
			}
		}
	}

	m_minVert = minVert;
	m_maxVert = maxVert;
}

void CMesh::computeFaceNormal()
{
	m_faceNormals.resize(m_faces.size());

	for (unsigned int f_id = 0; f_id < m_faces.size(); f_id++)
	{
		std::vector<int> vertIds = m_faces[f_id];

		MathLib::Vector3 OA = m_vertices[vertIds[1]] - m_vertices[vertIds[0]];
		MathLib::Vector3 OB = m_vertices[vertIds[2]] - m_vertices[vertIds[0]];

		MathLib::Vector3 fNormal = OA.cross(OB);
		//m_faceNormals[f_id] = fNormal.magnitude();
		fNormal.normalize();
		m_faceNormals[f_id] = fNormal;
	}
}

void CMesh::transformMesh(const MathLib::Matrix4d &transMat)
{
	// transform verts
	for (unsigned int v_id = 0; v_id < m_vertices.size(); v_id++)
	{
		m_vertices[v_id] = transformVert(m_vertices[v_id], transMat);
	}

	// transform min max verts for aabb
	computeMinMaxVerts();
}

MathLib::Vector3 CMesh::transformVert(const MathLib::Vector3 &vert, const MathLib::Matrix4d &transMat)
{
	return transMat.transform(vert);
}

bool CMesh::isSegIntersect(const MathLib::Vector3 &startPt, const MathLib::Vector3 &endPt, const double radius, MathLib::Vector3 &intersectPoint /*= MathLib::Vector3(0, 0, 0)*/)
{
	// if test with Ray
	if (radius == 0)
	{
		MathLib::Vector3 segDir = endPt - startPt;
		double segLength = segDir.magnitude();
		segDir.normalize();

		IceMaths::Ray segRay(IceMaths::Point(startPt[0], startPt[1], startPt[2]),
			IceMaths::Point(segDir[0], segDir[1], segDir[2]));

		Opcode::RayCollider rayCollider;
		rayCollider.SetMaxDist(segLength);
		Opcode::CollisionFace closest_contact;
		Opcode::SetupClosestHit(rayCollider, closest_contact);

		bool testStatus = rayCollider.Collide(segRay, *m_OpcodeModel);

		if (testStatus)
		{
			if (rayCollider.GetContactStatus())
			{
				double d = closest_contact.mDistance;
				intersectPoint = startPt + segDir*d;
				return true;
			}
		}

		return false;
	}

	// test with Capsule
	else
	{
		IceMaths::LSS capsule;
		capsule.mP0 = IceMaths::Point(startPt[0], startPt[1], startPt[2]);
		capsule.mP1 = IceMaths::Point(endPt[0], endPt[1], endPt[2]);
		capsule.mRadius = radius;

		Opcode::LSSCollider capsuleCollider;
		Opcode::LSSCache capsuleCache;

		bool testStatus = capsuleCollider.Collide(capsuleCache, capsule, *m_OpcodeModel, null, null);

		if (testStatus)
		{
			if (capsuleCollider.GetContactStatus())
			{
				return true;
			}
		}

		return false;
	}

}

void CMesh::buildOpcodeModel()
{
	// Opcode mesh interface
	m_opFaces = new unsigned int[m_faces.size()*3];
	int k = 0;
	for (int i = 0; i < m_faces.size(); i++)
	{
		m_opFaces[k++] = m_faces[i][0];
		m_opFaces[k++] = m_faces[i][1];
		m_opFaces[k++] = m_faces[i][2];
	}

	IceMaths::Point *opVerts = new IceMaths::Point[m_vertices.size()];
	for (int i = 0; i < m_vertices.size(); i++)
	{
		opVerts[i] = IceMaths::Point(m_vertices[i][0], m_vertices[i][1], m_vertices[i][2]);
	}

	m_meshIterface = new Opcode::MeshInterface();
	m_meshIterface->SetNbTriangles(m_faces.size());
	m_meshIterface->SetNbVertices(m_vertices.size());
	m_meshIterface->SetPointers((const IndexedTriangle*)m_opFaces, opVerts);

	Opcode::OPCODECREATE opCreate;
	opCreate.mIMesh = m_meshIterface;
	opCreate.mSettings.mLimit = 1;
	opCreate.mSettings.mRules = Opcode::SPLIT_SPLATTER_POINTS | Opcode::SPLIT_GEOM_CENTER;
	opCreate.mNoLeaf = true;
	opCreate.mQuantized = true;
	opCreate.mKeepOriginal = false;
	opCreate.mCanRemap = false;

	m_OpcodeModel = new Opcode::Model();
	m_OpcodeModel->Build(opCreate);
}

void CMesh::updateOpcodeModel()
{
	IceMaths::Point *opVerts = new IceMaths::Point[m_vertices.size()];
	for (int i = 0; i < m_vertices.size(); i++)
	{
		opVerts[i] = IceMaths::Point(m_vertices[i][0], m_vertices[i][1], m_vertices[i][2]);
	}

	m_meshIterface->SetPointers((const IndexedTriangle*)m_opFaces, opVerts);

	if (m_OpcodeModel != NULL)
	{
		delete m_OpcodeModel;
	}

	Opcode::OPCODECREATE opCreate;
	opCreate.mIMesh = m_meshIterface;
	opCreate.mSettings.mLimit = 1;
	opCreate.mSettings.mRules = Opcode::SPLIT_SPLATTER_POINTS | Opcode::SPLIT_GEOM_CENTER;
	opCreate.mNoLeaf = true;
	opCreate.mQuantized = true;
	opCreate.mKeepOriginal = false;
	opCreate.mCanRemap = false;

	m_OpcodeModel = new Opcode::Model();
	m_OpcodeModel->Build(opCreate);
}

bool CMesh::isOBBIntersect(const COBB &testOBB)
{
	IceMaths::Matrix3x3 rotMat = IceMaths::Matrix3x3(testOBB.axis[0][0], testOBB.axis[0][1], testOBB.axis[0][2],
		testOBB.axis[1][0], testOBB.axis[1][1], testOBB.axis[1][2],
		testOBB.axis[2][0], testOBB.axis[2][1], testOBB.axis[2][2]);

	// old submission
	//IceMaths::Matrix3x3 rotMat = IceMaths::Matrix3x3(testOBB.axis[0][0], testOBB.axis[1][0], testOBB.axis[2][0],
	//	testOBB.axis[0][1], testOBB.axis[1][1], testOBB.axis[2][1],
	//	testOBB.axis[0][2], testOBB.axis[1][2], testOBB.axis[2][2]);

	IceMaths::OBB iceOBB = IceMaths::OBB(IceMaths::Point(testOBB.cent[0], testOBB.cent[1], testOBB.cent[2]),
		IceMaths::Point(testOBB.hsize[0], testOBB.hsize[1], testOBB.hsize[2]),
		rotMat);

	Opcode::OBBCollider obbCollider;
	Opcode::OBBCache obbCache;

	bool testStatus = obbCollider.Collide(obbCache, iceOBB, *m_OpcodeModel, null, null);

	if (testStatus)
	{
		if (obbCollider.GetContactStatus())
		{
			return true;
		}
	}

	return false;
}

void CMesh::saveObjFile(const std::string &filename)
{
	QFile outFile(QString(filename.c_str()));
	QTextStream ofs(&outFile);

	if (!outFile.open(QIODevice::ReadWrite | QIODevice::Text)) return;

	for (int i = 0; i < m_vertices.size(); i++)
	{
		ofs << "v " << m_vertices[i][0] << " " << m_vertices[i][1] << " " << m_vertices[i][2] << "\n";
	}

	for (int i = 0; i < m_faces.size(); i++)
	{
		ofs << "f " << m_faces[i][0] + 1 << " " << m_faces[i][1] + 1 << " " << m_faces[i][2] + 1 << "\n";
	}

	outFile.close();
}

MathLib::Vector3 CMesh::getFaceCenter(int fid)
{
	MathLib::Vector3 faceCent;
	for (int i = 0; i < 3; i++)
	{
		faceCent += m_vertices[m_faces[fid][0]];
	}
	
	faceCent = faceCent / 3;

	return faceCent;
}

std::vector<MathLib::Vector3> CMesh::getVerts(const std::vector<int> &ids)
{
	std::vector<MathLib::Vector3> verts(ids.size());

	for (int i = 0; i < ids.size(); i++)
	{
		verts[i] = m_vertices[ids[i]];
	}

	return verts;
}

