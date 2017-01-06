// Looks like C but is in fact C++
//      Author: 	Bruno Messmer <messmer@iam.unibe.ch>
//     Created:	Thu Sep 30 14:18:37 1993
//    Filename:	/home/siam/messmer/NFPproject/Code/include/LineDrawing.h

#ifndef _linedrawing
#define _linedrawing

#include "Tools.h"
#include "Stroke.h"
#include "Graph.h"

#define EDGE_TO_EDGE_LABEL 11
#define VERTEX_TO_EDGE_LABEL 12
#define EDGE_AS_VERTEX_LABEL 13
#define ARTIFICIAL_EDGE 14
#define EDGE_ANGLE_LABEL 15
#define END_POINT_LABEL 16
#define PARALLEL_EDGE 17
#define REAL_EDGE 18
#define LABEL0 0
#define SPATIAL_LABEL 17

Graph* extract_parallel_line_graph(Graph *new_G);
extern Graph* extract_line_graph_from_eepic(char *name,double threshold, double , double);

extern Graph* build_complex_vertices(Graph* G, double radius_threshold,double angle_threshold);

extern void split_lines_at_intersections(List *LINES,double threshold);

extern void delete_tiny_lines(List* LINES,double line_threshold);

extern Graph* make_line_graph(List* LINES);


extern Graph* extract_vertex_edge_angle_graph(Graph* new_G);

extern void create_additional_edges(Graph* G,double angle_threshold);

#endif
