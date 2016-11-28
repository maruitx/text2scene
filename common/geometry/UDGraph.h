#pragma once
#include <vector>

#define MAX_STR_BUF_SIZE	1024

class CUDGraph
{
public:
	CUDGraph();
	CUDGraph(int iNumV) { Initialize(iNumV); }
	virtual ~CUDGraph();

	class Vertex {
	public:
		Vertex() { t = 0; w = 0.0; }
		Vertex(int tag) { t = tag; w = 0.0; }
		Vertex(int tag, double d) { t = tag; w = d; }
		~Vertex() {}
		int t;
		double w;
	};

	class Edge {
	public:
		Edge() { v1 = v2 = 0; t = 0; d = 0; w = 0.0; }
		Edge(int i1, int i2) { v1 = i1; v2 = i2; t = 0; d = 0; w = 0.0; }
		Edge(int i1, int i2, double d) { v1 = i1; v2 = i2; t = 0; d = 0; w = d; }
		Edge(int i1, int i2, int tag) { v1 = i1; v2 = i2; t = tag; d = 0; w = 0.0; }
		Edge(int i1, int i2, int tag, int dir) { v1 = i1; v2 = i2; t = tag; d = dir; w = 0.0; }
		Edge(int i1, int i2, int tag, double ww) { v1 = i1; v2 = i2; t = tag; d = 0; w = ww; }
		~Edge() {}

		int&		operator[](int i)       { return (v[i]); }
		const int&	operator[](int i) const { return (v[i]); }
		Edge&		operator=(const Edge& e) { v1 = e.v1; v2 = e.v2; w = e.w; t = e.t; fv = e.fv; return *this; }
		friend int	operator==(const Edge& e1, const Edge& e2) { return (e1.v1 == e2.v1&&e1.v2 == e2.v2) || (e1.v1 == e2.v2&&e1.v2 == e2.v1); }
		friend int	operator!=(const Edge& e1, const Edge& e2) { return !(e1 == e2); }

		union { struct { int v1, v2; }; struct { int first, second; }; struct { int v[2]; }; };
		int t;					// edge tag
		short d;				// direction flag (used only for notation)
		double w;					// edge weight
		std::vector<double> fv;	// feature vector
	};

	bool IsEmpty(void) { return (m_V.size() == 0); }
	void Clear(void);

	void Initialize(int iNumV);
	unsigned int Size(void) const { return m_V.size(); }
	unsigned int ESize(void) const { return m_E.size(); }
	void SetV(int iNumV);

	int InsertEdge(int ev1, int ev2);
	int InsertEdge(int ev1, int ev2, int tag);
	int InsertEdge(int ev1, int ev2, int tag, int dir);
	int InsertEdge(int ev1, int ev2, double ew);
	int InsertEdge(int ev1, int ev2, int tag, double ew);
	int DeleteEdge(int ev1, int ev2);
	int DeleteEdge(int eid);
	int InsertNode(int tag, double weight);
	int InsertNode(int tag);

	int ComputeComponentMap(std::vector<int> &CM, int &iNumC);

	bool IsEdge(int ev1, int ev2);
	bool HasEdge(int ev1, int tag);

	int SetEdgeTag(int ev1, int ev2, int tag);
	int SetEdgeTag(int eid, int tag);

	Edge* GetEdge(int i);
	Edge* GetEdge(int ev1, int ev2);
	int GetEdgeId(int ev1, int ev2) const;
	int GetEdgeTag(int ev1, int ev2) const;
	int GetEdgeTag(int eid) const;


	void GetAllNeigborEdgeList(int ev1, std::vector<int> &el) const;  // get all edge list origined from ev1
	void GetAllNeigborEdgeList(int ev1, int tag, std::vector<int> &el) const;
	void GetAllNeigborEdgeList(const std::vector<int> &evl, std::vector<int> &el) const;
	void GetAllNeigborEdgeList(const std::vector<int> &evl, int tag, std::vector<int> &el) const;

	void ReadEdge(std::ifstream &ifs);
	void WriteEdge(std::ofstream &ofs);
	void ReadVert(std::ifstream &ifs);
	void WriteVert(std::ofstream &ofs);

	CUDGraph& operator=(const CUDGraph& e);


protected:
	std::vector<Vertex>				m_V;
	std::vector<Edge>				m_E;
};

