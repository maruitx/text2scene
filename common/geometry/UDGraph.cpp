#include "UDGraph.h"
#include <fstream>

//#include <boost/graph/adjacency_list.hpp>
//#include <boost/graph/connected_components.hpp>

CUDGraph::CUDGraph()
{
}


CUDGraph::~CUDGraph()
{
}

void CUDGraph::Clear(void)
{
	m_V.clear();
	m_E.clear();
}

void CUDGraph::Initialize(int iNumV)
{
	Clear();
	SetV(iNumV);
}

void CUDGraph::SetV(int iNumV)
{
	m_V.resize(iNumV);
}

int CUDGraph::InsertEdge(int ev1, int ev2)
{
	if (ev1 == ev2) {
		return -1;
	}
	else if (ev1 > ev2) {
		std::swap(ev1, ev2);
	}
	Edge e(ev1, ev2);
	if (std::find(m_E.begin(), m_E.end(), e) == m_E.end()) {
		m_E.push_back(e);
		// 		m_dMaxW = std::max(m_dMaxW, e.w);
		// 		m_dMinW = std::min(m_dMinW, e.w);
		return 0;
	}
	else {
		return -1;
	}
}

int CUDGraph::InsertEdge(int ev1, int ev2, double ew)
{
	if (ev1 == ev2) {
		return -1;
	}
	else if (ev1 > ev2) {
		std::swap(ev1, ev2);
	}
	Edge e(ev1, ev2, ew);
	std::vector<Edge>::iterator it;
	if ((it = std::find(m_E.begin(), m_E.end(), e)) == m_E.end()) { // if not found
		m_E.push_back(e);
		return 0;
	}
	else { // if already exist
		return -1;
	}
}

int CUDGraph::InsertEdge(int ev1, int ev2, int tag, int dir)
{
	if (ev1 == ev2) {
		return -1;
	}
	else if (ev1 > ev2) {
		std::swap(ev1, ev2);
	}
	Edge e(ev1, ev2, tag, dir);
	std::vector<Edge>::iterator it;
	if ((it = std::find(m_E.begin(), m_E.end(), e)) == m_E.end()) { // if not found
		m_E.push_back(e);
		return 0;
	}
	else { // if already exist
		return -1;
	}
}

int CUDGraph::InsertEdge(int ev1, int ev2, int tag)
{
	if (ev1 == ev2) {
		return -1;
	}
	else if (ev1 > ev2) {
		std::swap(ev1, ev2);
	}
	Edge e(ev1, ev2, tag);
	std::vector<Edge>::iterator it;
	if ((it = std::find(m_E.begin(), m_E.end(), e)) == m_E.end()) { // if not found
		m_E.push_back(e);
		return 0;
	}
	else { // if already exist
		return -1;
	}
}

int CUDGraph::InsertEdge(int ev1, int ev2, int tag, double ew)
{
	if (ev1 == ev2) {
		return -1;
	}
	else if (ev1 > ev2) {
		std::swap(ev1, ev2);
	}
	Edge e(ev1, ev2, tag, ew);
	std::vector<Edge>::iterator it;
	if ((it = std::find(m_E.begin(), m_E.end(), e)) == m_E.end()) { // if not found
		m_E.push_back(e);
		return 0;
	}
	else { // if already exist
		return -1;
	}
}

int CUDGraph::DeleteEdge(int eid)
{
	if (eid < 0 || eid >= m_E.size()) {
		return -1;
	}
	m_E.erase(m_E.begin() + eid);
	return 0;
}

int CUDGraph::DeleteEdge(int ev1, int ev2)
{
	Edge e(ev1, ev2);
	std::vector<Edge>::iterator it;
	if ((it = std::find(m_E.begin(), m_E.end(), e)) != m_E.end()) { // if found
		m_E.erase(it);
		return 0;
	}
	return 1;
}

int CUDGraph::InsertNode(int tag, double weight)
{
	m_V.push_back(Vertex(tag, weight));
	return 0;
}

int CUDGraph::InsertNode(int tag)
{
	m_V.push_back(Vertex(tag));
	return 0;
}

bool CUDGraph::IsEdge(int ev1, int ev2)
{
	return (std::find(m_E.begin(), m_E.end(), Edge(ev1, ev2)) != m_E.end());
}

bool CUDGraph::HasEdge(int ev1, int tag)
{
	for (unsigned int i = 0; i < m_E.size(); i++) {
		if (m_E[i].t == tag && (m_E[i][0] == ev1 || m_E[i][1] == ev1)) {
			return true;
		}
	}
	return false;
}

void CUDGraph::ReadEdge(std::ifstream &ifs)
{
	int num(0);
	char buf[MAX_STR_BUF_SIZE];
	ifs >> num;
	for (int i = 0; i < num; i++) {
		ifs.getline(buf, MAX_STR_BUF_SIZE, '\n');
		int ev1, ev2, tag;
		double weight(0);
		ifs >> ev1 >> ev2 >> tag >> weight;
		InsertEdge(ev1, ev2, tag, weight);
	}
}

void CUDGraph::WriteEdge(std::ofstream &ofs)
{
	ofs << "E " << m_E.size() << std::endl;
	for (unsigned int i = 0; i < m_E.size(); i++) {
		ofs << m_E[i].v1 << " " << m_E[i].v2 << " " << m_E[i].t << " " << m_E[i].w << std::endl;
	}
}

void CUDGraph::ReadVert(std::ifstream &ifs)
{
	int num(0);
	char buf[MAX_STR_BUF_SIZE];
	ifs >> num;
	for (int i = 0; i < num; i++) {
		ifs.getline(buf, MAX_STR_BUF_SIZE, '\n');
		int tag(0); double weight(0);
		ifs >> tag >> weight;
		InsertNode(tag, weight);
	}
}

void CUDGraph::WriteVert(std::ofstream &ofs)
{
	ofs << "N " << m_V.size() << std::endl;
	for (unsigned int i = 0; i < m_V.size(); i++) {
		ofs << m_V[i].t << " " << m_V[i].w << std::endl;
	}
}

int CUDGraph::SetEdgeTag(int ev1, int ev2, int tag)
{
	Edge e(ev1, ev2);
	std::vector<Edge>::iterator it;
	if ((it = std::find(m_E.begin(), m_E.end(), e)) != m_E.end()) { // if found
		it->t = tag;
		return 0;
	}
	return -1;
}

int CUDGraph::SetEdgeTag(int eid, int tag)
{
	if (eid < 0 || eid >= (int)m_E.size()) {
		return -1;
	}
	m_E[eid].t = tag;
	return 0;
}

CUDGraph::Edge* CUDGraph::GetEdge(int i)
{
	if (i >= 0 && i < (int)m_E.size()) {
		return &m_E[i];
	}
	return NULL;
}

CUDGraph::Edge* CUDGraph::GetEdge(int ev1, int ev2)
{
	Edge e(ev1, ev2);
	std::vector<Edge>::iterator it;
	if ((it = std::find(m_E.begin(), m_E.end(), e)) != m_E.end()) {
		return &(*it);
	}
	return NULL;
}

int CUDGraph::GetEdgeId(int ev1, int ev2) const
{
	for (unsigned int i = 0; i < m_E.size(); i++) {
		if ((m_E[i].v1 == ev1&&m_E[i].v2 == ev2) || (m_E[i].v1 == ev2&&m_E[i].v2 == ev1)) {
			return i;
		}
	}
	return -1;
}

int CUDGraph::GetEdgeTag(int ev1, int ev2) const
{
	Edge e(ev1, ev2);
	std::vector<Edge>::const_iterator it;
	if ((it = std::find(m_E.begin(), m_E.end(), e)) != m_E.end()) { // if found
		return it->t;
	}
	return -1;
}

int CUDGraph::GetEdgeTag(int eid) const
{
	if (eid < 0 || eid >= (int)m_E.size()) {
		return -1;
	}
	return m_E[eid].t;
}

void CUDGraph::GetAllNeigborEdgeList(int ev1, std::vector<int> &el) const
{
	el.clear();
	for (unsigned int i = 0; i < m_E.size(); i++) {
		if (m_E[i][0] == ev1) {
			el.push_back(i);
		}
		else if (m_E[i][1] == ev1) {
			el.push_back(i);
		}
	}
}

void CUDGraph::GetAllNeigborEdgeList(int ev1, int tag, std::vector<int> &el) const
{
	el.clear();
	for (unsigned int i = 0; i < m_E.size(); i++) {
		if (m_E[i].t != tag) { continue; }
		if (m_E[i][0] == ev1) {
			el.push_back(i);
		}
		else if (m_E[i][1] == ev1) {
			el.push_back(i);
		}
	}
}


void CUDGraph::GetAllNeigborEdgeList(const std::vector<int> &evl, std::vector<int> &el) const
{
	el.clear();
	std::vector<short> vmap(m_V.size(), 0);
	for (unsigned int ev = 0; ev < evl.size(); ev++) {
		vmap[evl[ev]] = 1;
	}
	for (unsigned int ev = 0; ev < evl.size(); ev++) {
		for (unsigned int i = 0; i < m_E.size(); i++) {
			if (m_E[i][0] == evl[ev] && vmap[m_E[i][1]] == 0) { // find an outgoing edge
				el.push_back(i);
			}
			else if (m_E[i][1] == evl[ev] && vmap[m_E[i][0]] == 0) { // find an outgoing edge
				el.push_back(i);
			}
		}
	}
}


void CUDGraph::GetAllNeigborEdgeList(const std::vector<int> &evl, int tag, std::vector<int> &el) const
{
	el.clear();
	std::vector<short> vmap(m_V.size(), 0);
	for (unsigned int ev = 0; ev < evl.size(); ev++) {
		vmap[evl[ev]] = 1;
	}
	for (unsigned int ev = 0; ev < evl.size(); ev++) {
		for (unsigned int i = 0; i < m_E.size(); i++) {
			if (m_E[i].t != tag) { continue; }
			if (m_E[i][0] == evl[ev] && vmap[m_E[i][1]] == 0) { // find an outgoing edge
				el.push_back(i);
			}
			else if (m_E[i][1] == evl[ev] && vmap[m_E[i][0]] == 0) { // find an outgoing edge
				el.push_back(i);
			}
		}
	}
}
//
//int CUDGraph::ComputeComponentMap(std::vector<int> &CM, int &iNumC)
//{
//	if (m_V.empty()) {
//		CM.clear();
//		iNumC = 0;
//		return -1;
//	}
//	using namespace boost;
//	{
//		typedef adjacency_list <vecS, vecS, undirectedS> Graph;
//		Graph G(m_E.begin(), m_E.end(), m_V.size());
//		CM.resize(num_vertices(G));
//		iNumC = connected_components(G, &CM[0]);
//	}
//	return 0;
//}
//
//namespace boost
//{
//	// Define result callback
//	template <typename Graph1, typename Graph2>
//	struct iso_rslt_callback_1 {
//		iso_rslt_callback_1(const Graph1 &graph1, const Graph2 &graph2, std::vector<std::pair<int, int>> &map1to2) : graph1_(graph1), graph2_(graph2), map1to2_(map1to2) {}
//		template <typename CorrespondenceMap1To2, typename CorrespondenceMap2To1>
//		bool operator()(CorrespondenceMap1To2 f, CorrespondenceMap2To1) const {
//			// Print (sub)graph isomorphism map
//			BGL_FORALL_VERTICES_T(v, graph1_, Graph1) {
//				map1to2_.push_back(std::make_pair(get(vertex_index_t(), graph1_, v), get(vertex_index_t(), graph2_, get(f, v))));
//			}
//			return true;
//		}
//	private:
//		const Graph1 &graph1_;
//		const Graph2 &graph2_;
//		std::vector<std::pair<int, int>> &map1to2_;
//	};
//
//	template <typename Graph1, typename Graph2>
//	struct iso_rslt_callback_2 {
//		iso_rslt_callback_2(const Graph1 &graph1, const Graph2 &graph2, std::vector<int> &mapto) : graph1_(graph1), graph2_(graph2), mapto_(mapto) {}
//		template <typename CorrespondenceMap1To2, typename CorrespondenceMap2To1>
//		bool operator()(CorrespondenceMap1To2 f, CorrespondenceMap2To1) const {
//			// Print (sub)graph isomorphism map
//			BGL_FORALL_VERTICES_T(v, graph1_, Graph1) {
//				mapto_.push_back(get(vertex_index_t(), graph2_, get(f, v)));
//			}
//			return true;
//		}
//	private:
//		const Graph1 &graph1_;
//		const Graph2 &graph2_;
//		std::vector<int> &mapto_;
//	};
//
//	template <typename Graph1, typename Graph2>
//	struct iso_rslt_callback_3 {
//		iso_rslt_callback_3(const Graph1 &graph1, const Graph2 &graph2, std::vector<int> &mapto) : graph1_(graph1), graph2_(graph2), mapto_(mapto) {}
//		template <typename CorrespondenceMap1To2, typename CorrespondenceMap2To1>
//		bool operator()(CorrespondenceMap1To2 f, CorrespondenceMap2To1) const {
//			// Print (sub)graph isomorphism map
//			std::vector<int> nm;
//			BGL_FORALL_VERTICES_T(v, graph1_, Graph1) {
//				nm.push_back(get(vertex_index_t(), graph2_, get(f, v)));
//			}
//			int n = mapto_.size();
//			int m = graph1_.m_vertices.size();
//			std::set<int> s1, s2;
//			std::copy(nm.begin(), nm.end(), std::inserter(s2, s2.begin()));
//			for (unsigned int i = 0; i < n; i += m) {
//				s1.clear();
//				std::copy(mapto_.begin() + i, mapto_.begin() + (i + m), std::inserter(s1, s1.begin()));
//				if (s1 == s2) { return true; }
//			}
//			std::copy(nm.begin(), nm.end(), std::back_inserter(mapto_));
//			return true;
//		}
//	private:
//		const Graph1 &graph1_;
//		const Graph2 &graph2_;
//		std::vector<int> &mapto_;
//	};
//}