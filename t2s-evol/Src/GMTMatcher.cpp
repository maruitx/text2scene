#include "GMTMatcher.h"


GMTMatcher::GMTMatcher()
{
	m_graphDatabase = new KB();
}


GMTMatcher::~GMTMatcher()
{
}

void GMTMatcher::test()
{
	KB Database;
	Graph *G1, *G2, *G3;
	double values[1];


	cout << "*************************************\n";
	cout << "*        GRAPH MATCHER DEMO         *\n";
	cout << "*        University of Bern         *\n";
	cout << "*************************************\n\n";



	/* Register your own comparison functions */
	/* Possible values for 1. arg are:        */
	/*    SUB_FUNCTION,                       */
	/*	DEL_OF_EDGES,INS_OF_EDGES           */
	/*	INS_OF_VERTICES,DEL_OF_VERTICES     */


	registerFunction(SUB_FUNCTION, (void*(*)()) substitution_cost);
	registerFunction(DEL_OF_EDGES, (void*(*)()) edge_deletion);





	/************************************************************/
	/* Instead of the following lines, write your own functions */
	/* for reading and writing Graphs to a file.                */
	/* These functions may be simple C or C++ functions         */
	/************************************************************/

	/* set Graph 1 */
	/* set(Number,Label,attribute array, dimension of array) */


	G1 = new Graph;

	values[0] = 2;
	G1->set(0, 1, values, 1);
	values[0] = 3;
	G1->set(1, 3, values, 1);
	values[0] = 1;
	G1->set(2, 1, values, 1);

	/* set edges */

	/* setEdge(out-node,in-node,number,label,attributes-array,dim of array) */
	G1->setEdge(0, 1, 1, 4, NULL, 0);
	G1->setEdge(1, 2, 2, 4, NULL, 0);

	G1->setName("G1");
	/* mark end of graph G1 definition (1) directed, (0) undirected */
	G1->done(1);


	/* set Graph 2 */

	G2 = new Graph;
	values[0] = 2;
	G2->set(0, 1, values, 1);
	values[0] = 3;
	G2->set(1, 3, values, 1);
	values[0] = 1;
	G2->set(2, 1, values, 1);
	values[0] = 2;
	G2->set(3, 3, values, 1);

	/* set edges */

	/* setEdge(out-node,in-node,number,label,attributes-array,dim of array) */
	G2->setEdge(0, 3, 1, 4, NULL, 0);
	G2->setEdge(0, 1, 2, 4, NULL, 0);
	G2->setEdge(1, 2, 3, 4, NULL, 0);
	G2->setEdge(2, 3, 4, 4, NULL, 0);

	G2->setName("G2");
	G2->done(1);

	/*set Graph 3*/
	G3 = new Graph;
	values[0] = 1.5;
	G3->set(0, 1, values, 1);
	values[0] = 4;
	G3->set(1, 3, values, 1);
	values[0] = 1;
	G3->set(2, 1, values, 1);
	values[0] = 2;
	G3->set(3, 3, values, 1);

	/* set edges */

	/* setEdge(out-node,in-node,number,label,attributes-array,dim of array) */
	G3->setEdge(0, 2, 2, 4, NULL, 0);
	G3->setEdge(0, 1, 2, 4, NULL, 0);
	G3->setEdge(1, 2, 3, 4, NULL, 0);
	G3->setEdge(2, 3, 4, 4, NULL, 0);

	G3->setName("G3");
	G3->done(1);


	/* dump the graphs:  G1, G2 */

	cout << "\n\nGraph G1:\n";
	G1->dump();

	cout << "\n\nGraph G2:\n";
	G2->dump();


	/* insert G1 into Network */
	/* addModel(Graph*,int key) */
	cout << "\n\nAdding Graph G1 to Database \n";
	Database.addModel(G1, 0);

	cout << "\n\nAdding Graph G2 to Database \n";
	Database.addModel(G2, 0);

	cout << "\n\nAdding Graph G2 to Database \n";
	Database.addModel(G3, 0);


	/* Interpret Graph 2 */

	cout << "\n\nInterpreting input graph G2 ...\n";
	double match_threshold = -1;

	/* recognition(Graph*, error threshold=-1 (no limit) =0 (exact match only)  */
	/*                                    >0 (find first inexact match)         */
	Database.recognition(G3, match_threshold);

	/* Display found instances */

	cout << "Instances:\n";
	Database.displayInterpretation();


	/* retrieving the found instances */

	int x, y;
	InstanceData* IS;

	x = Database.NumberOfMatches();

	for (y = 0; y < x; y++){
		IS = Database.getInstance(y);

		/* look in KBClass.h for the definition of InstanceData */

		/* use the matching information in IS                   */
		/* DO NOT DELETE IS !                                   */
	}

}
