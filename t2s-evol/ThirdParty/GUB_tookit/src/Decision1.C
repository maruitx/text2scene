/********************************************************************
              Copyright 1995 University of Bern              
     
                   All Rights Reserved
 
Permission to use this software and its documentation for any purpose
 and without fee is hereby granted, provided that the above copyright 
notice appear in all copies and that both that copyright notice and this 
permission notice appear in supporting documentation. Do not distribute 
without specific, written prior permission.  
                                                  
Author: Bruno T. Messmer	                       
Email:  messmer@iam.unibe.ch	                   
********************************************************************/

#include "Decision.h"


//extern Timer Timex;


int 
PG_class::merge1(Graph *G,int num){
    DT_node* state;
    AM=&G->AM;
    int i,x,next;
    INT_TYPE* permutation;
    
    
    //time_spent=Timex.time_stamp();
	time_spent = elapsed_time();
    
    current_start_pos=0;

    if((num>=MAX_MODELS)||(Models_in_tree[num]!=NULL)){
	cout << "Model number collision! exiting....\n";
	exit(10);
    }
    Models_in_tree[num]=G;
    NumberOfModels=num;

    if(minSizedModel>AM->numberOfVertices())
	minSizedModel=AM->numberOfVertices();
    
    if(maxSizedModel<AM->numberOfVertices())
	maxSizedModel=AM->numberOfVertices();

    if(RootNode==NULL){
	RootNode=new DT_node;
	RootNode->successors=new DT_node* [MEM_LIMIT];
	RootNode->allocated=MEM_LIMIT;
	RootNode->succ=0;
	RootNode->models=new int[1];
	RootNode->inst_count= new int[1];
	RootNode->inst_count[0]=0;
	RootNode->models_count=0;
	RootNode->models[0]=num;   
	RootNode->mark=0;
	RootNode->terminal=0;
	RootNode->tag=1;
	numberOfNodes=1;
    }
   

    state=RootNode;
    step=0;
    Dim=AM->Dimension;
   
    
    if(Dim>ENV_MAX_GRAPH_LENGTH){

	if(!ENV_PRUNING_TREE){

	    /* Call large merge function */
	    return mergeLargeGraph(G,num);
	}


    }

    largeGraphMerge=0;
    permutation=new INT_TYPE[Dim];
    vertex_sequence=new INT_TYPE[Dim];
    edges_tuple=new int[Dim];
    
    H_seq=new int[Dim];
    
    int first_permutation;
    INT_TYPE *first_perm;
    DT_node* first_state;
    
    first_perm=new INT_TYPE[Dim];
    int PATH_ID=0;
    int path_trace;
    int offset;
    int continueSequences;
    
    int pruning=0;
    vertex_checked=NULL;
    
    delInstances1.clear();
    delInstances2.clear();

    for(int s=1;s<=Dim;s++){
	/*
	 * create all subset of size s and retrieve them one
	 * by one with nextSubset
	 */
	
      //	printf("level %d of %d\n",s,Dim);

	if((s>=ENV_BUILD_LEVEL)){

	    if((ENV_FAST_BUILD)&&(s<=Dim-ENV_BYTREE_DIFF))
		first_state=buildfromSubsets_bytree1(G,num,s);


	    if(ENV_PRUNING_TREE)
		if(!first_state) break;

	    continue;
	}
	
	if(s<Dim) BUILDING_TREE=0;
	//else BUILDING_TREE=1;
	
	initSubset(s,Dim);
	
	
	continueSequences=0;  // all subsets of size s are pruned stop building
	
        while(nextSubset()){
	    /* for each subset, generate every possible 
	     *  permutation. If the first permutated sequence
	     *  of vertices cannot be recognized by the Decision Tree
	     *  it must be extended by a new NODE. 
	     *  Every subsequent permutation
	     *  can then be link to the NODE by permutating its
	     *  vertices with the inverse permutation!
	     */
	    
	    
	    first_permutation=1;
	    
	    PATH_ID++;
	    
	    initSequences(s,0);
	    
	    first_state=NULL;
	    
	    while(nextSequences(permutation)){
		for(x=0;x<s;x++){
		    vertex_sequence[x]=subset[permutation[x]];
// 		    if(s==Dim)
// 		      cout << vertex_sequence[x] << " ";
		}
// 		if(s==Dim) cout << " > ";
		
		state=RootNode;
		PASSED_BY_NODE=0;
		path_trace=1;
		
		step=0;
		while(step<s){
		    
//		    if(ENV_PRUNING_TREE){
//			if(state->sequences.count()==1){
//			    pruning=1;
//			    break;
//			}
//		    }
		    
		    for(x=0;x<step;x++)
			edges_tuple[x]=AM->isEdge(vertex_sequence[step],vertex_sequence[x]);
// 			if(AM->isEdge(vertex_sequence[step],vertex_sequence[x])>-1)
// 			    edges_tuple[x]=1;
// 			else
// 			    edges_tuple[x]=0;
// 		    }
		    
		    next=state->branch.test(edges_tuple,vertex_sequence[step],AM);
		    
		    if(next<0){
			if(first_permutation){
			    next=addNode(state,edges_tuple,num,
					 vertex_sequence[step]);
			    first_state=state->successors[next];
			    first_state->mark=PATH_ID;
			    offset=first_state->sequences.count();
			}else{
			    gmt_assert(step==s-1);
			    next=addNode_bypass(state,edges_tuple,permutation,
						s,first_state,first_perm,
						num,vertex_sequence[step]);			
			}		    
		    }else{
			if(first_permutation){
			    
			    first_state=state->successors[next];
			    first_state->mark=PATH_ID;
			    offset=first_state->sequences.count();
			  
			    if(step<s-1) first_state=NULL;
			  }
		    }
		    
		    
		    setNodeData(state->successors[next],num);
		    
		    if(state->successors[next]->mark!=PATH_ID)
		      path_trace=0;
		    
		    if(step==s-1)
		      if((first_permutation)||(path_trace))
			addInstance(state->successors[next],vertex_sequence);
		        
		    state=makeOneStepInMerge(state->successors[next],step+1,
					     permutation);
		    
		    if(step==s-1)
			if(!((first_state==state)||(first_state->allocated==-1)))
			    printf("States confusion...\n");
		    step++;
		}
		
	
	   	
		if((first_permutation)&&(pruning==0)){
		  for(x=0;x<s;x++)
		    first_perm[x]=permutation[x];
		  first_permutation=0;
		  
		  
		}

		if(pruning==0) continueSequences=1;
		pruning=0;
	
	    }
	  
 	    /*  copy all instances from the first_state into
 	     *  the gate_state if first_state itself was not a 
 	     *  gate_state! Otherwise the instance count is not
 	     *	correct!
 	     *	it is sufficient to store the
 	     *	instances in the gate_state!
 	     */
			      

	    if(first_state!=NULL){
	      if(first_state->allocated==-1){
		DT_node* dest_state;
		  INT_TYPE* inst;
		  
		  first_state->sequences.reset();
		  int j=0;
		  while(inst=(INT_TYPE*) first_state->sequences.getnext()){
		    if(j>=offset){
		      for(x=0;x<s;x++)
			vertex_sequence[x]=inst[x];
		      dest_state=makeOneStep(first_state, s);
		      addInstance(dest_state,vertex_sequence);
		      
		    }
		    j++;
		  }
	      }
	    }

	    delete [] vertex_checked;
	    delete [] AVAIL;
	}
	
	
	if(continueSequences==0) break;
    }
    
    if(first_state!=NULL){
	first_state->terminal=1;
	if(state->depth<minSizedModel)
	    minSizedModel=state->depth;
    }

    delete [] first_perm;
    delete [] permutation;
    delete [] H_seq;
    delete [] vertex_sequence;
    delete [] edges_tuple;
    
    
    
    if(ENV_CONSISTENCY_TEST){
	if(!consistencyTest(G))
	    cout << "Error....\n";
	else
	    cout << "Tree is consistent\n";
	
    }
    //time_spent=Timex.time_stamp()-time_spent;
	time_spent = elapsed_time() - time_spent;
    
    return 1;
    
}



DT_node*
PG_class::run1(DT_node* state,int st,int level){
// uses the global vertex_sequence and the edge_tuple
    int x,next;
    DT_node* direct_state;

    step=st;
    direct_state=state;
    while(step<level){
	
	for(x=0;x<step;x++)
	    edges_tuple[x]=AM->isEdge(vertex_sequence[step],vertex_sequence[x]);
	
	checks++;
	next=state->branch.test(edges_tuple,vertex_sequence[step],AM);
	
	//	if((next<0)||((return_last_always==0)&&(state->successors[next]->tag<=0))) {
	
	if(next<0){
	  Result_State=NULL;
	    return NULL;
	}
	
	direct_state=state->successors[next];
	state=makeOneStep(state->successors[next],step+1);	
	step++;
    }

    
    return direct_state;
}


/* 
 *    BuildfromSubset  sets the <level> nodes in the DT by
 *    choosing each subset (n \choose level) of G vertices -> v1..vk
 *    Then for each (v1..vk) -vi (for all i once)
 *    the final state is determine by calling run().
 *    if there is a state -> do notiung
 *    if no state is found, then a new branch must be added to
 *    the previous state. For each instance in the previous state
 *    (found by collectPermutations) we must test if a new branch must
 *    be created.
 *    Finally, all states created for (v1...vi) are redirected onto
 *    one single state. 
 */ 


DT_node*
PG_class::buildfromSubsets_bytree1(Graph* G,int num, int level){
 
  DT_node* state,*new_state;
    AM=&G->AM;
    int i,x,next,n,offset,s;
    int* first_perm,first_set;
    INT_TYPE* permutation,*seq,*original_sequence;
    List Perms;
    DT_node* first_state;
   
    
    AM=&G->AM;
    gmt_assert(RootNode!=NULL);

    Hash USED(10);
    List* adj_list;
    int bstep,out,in,y;

    
    Hash already_adj(10);
    int *v;
    AttId at;

    Indicator_type* indicator;
    List IND;

    int FIRST_TAG=1;

/*

  If ENV_PRUNING_TREE is set, we prune the tree below each node, which
  has less than ENV_PRUNE_INSTANCE_LIMIT instances or if the depth
  exceeds ENV_DECISION_TREE_LIMIT. These heuristic rules will make 
  the algorithm workable for large graphs also.

*/

    if(level>ENV_DECISION_TREE_LIMIT) return NULL;

    adj_list=new List[level];


    original_sequence=new INT_TYPE[level+1];
    
    for(x=0;x<AM->numberOfVertices();x++){
	v=new int;
	*v=x;
	adj_list[0].insert(v);
    }


    DT_node* some_first_state=NULL;
  
    bstep=0;
   
    initSubset(level,Dim);

    

    while(bstep>=0){

// special spanning of connected tree

	v=(int*) adj_list[bstep].getnext();

	if(bstep+Dim-USED.count()<level-1)
	    v=NULL;
	
	if(v==NULL){// retract to previous level, restore vertex_sequence!
	    adj_list[bstep].reset();
	    while(adj_list[bstep].count()){
		v=(int*) adj_list[bstep].remove(0);
		USED.remove(*v);
		delete v;
	    }
	    
	    bstep--;
	    continue;
	}

	gmt_assert(!USED.in(*v));
	USED.insert(NULL,*v);
	subset[bstep]=*v;

	if(bstep<level-1){
	    
	    // expand this state
	    // collect all adjacent vertices that are not in use

	    
	    bstep++;

	    already_adj.clear2();

	    for(x=0;x<bstep;x++){
		AM->initNext(subset[x]);
		while(AM->getnext(&out,&in,&at)!=-1){
		    if(out==subset[x])
			y=in;
		    else
			y=out;
		    if((!USED.in(y))&&(!already_adj.in(y))){
			v=new int;
			*v=y;
			adj_list[bstep].insert(v);
			already_adj.insert(NULL,*v);
		    }
		}
	    }
	    subset[bstep]=-1;
	    adj_list[bstep].reset();
	    continue;
	}
	
// end special spanning


	first_state=NULL;
	first_set=1;
	
	for(n=0;n<level;n++){
	    
	    i=0;
	    for(x=0;x<level;x++){
		if(x!=n){
		   
		    vertex_sequence[i]=subset[x];
		    i++;
		}
	    }
	  
	    vertex_sequence[level-1]=subset[n];

	    if(!orderInLine(vertex_sequence,level-1,level,level)) continue;

	    state=run1(RootNode,0,level-1);
	    
	    if(!state){
		// sequence is not coherent!
		if(orderInLine(vertex_sequence,1,level-1,level-1))
		    state=run1(RootNode,0,level-1);

		if(!state)
		    continue;
	    }


	    state=makeOneStep(state,level-1);

	    
	    int model_was_in=collectPermutations(state,level-1,Perms,IND,num); // of current vertex_sequence

	    int do_once=0;
	    if(level==1) do_once=1;
	    else{
		if(ENV_PRUNING_TREE){
		    if(level>2){
			if(state->inst_count[state->models_count]<=ENV_PRUNE_INSTANCE_LIMIT)
			    Perms.clear();
		    }
		}
	    }

	    while(Perms.count()||(do_once)){
		do_once=0;

		if(level>1){
		    seq=(INT_TYPE*) Perms.remove(0);
		    memcpy(vertex_sequence,seq,sizeof(INT_TYPE)*(level-1));
		    if(!model_was_in)
			delete [] seq;

		    indicator=(Indicator_type*) IND.remove(0);
		}

	
		vertex_sequence[level-1]=subset[n];
		
		new_state=run1(state,level-1,level);
		
		if(new_state==NULL){
		   

		    // create new state
		    if(first_set){
			    next=addNode(state,edges_tuple,num,
					 vertex_sequence[level-1]);
			    new_state=state->successors[next];
			    offset=new_state->sequences.count();
		    }else{
			// cal permutation and first_perm or rewrite addNode
			
			
			
			next=addNode_bypass(state,edges_tuple,vertex_sequence,
						level,first_state,original_sequence,
						num,vertex_sequence[level-1]);			
			new_state=state->successors[next];
		    }	

		    //  tag the necessary nodes for breadth pruning
		    if(level<=1){
			new_state->tag+=FIRST_TAG;
			FIRST_TAG=GLOBAL_TAG;
		    }else{

			if(indicator->node==NULL){
			    indicator->node=new_state;
			    if(state->tag>=0){
				tag_op(new_state,1);			      
			    }
			}else{
	     
			    tag_op(new_state,0);
			}	
		    }
		    // end tag
		}else{
		    if(first_set)
			offset=new_state->sequences.count();
		    if(G->numberOfVertices()==level){
			new_state->terminal=1;
			if(state->depth<minSizedModel)
			    minSizedModel=state->depth;
		    }
		    
		    //tag necessray nodes for breadth-pruning
		    if(level<=1){
			new_state->tag+=FIRST_TAG;
			FIRST_TAG=GLOBAL_TAG;
		    }else{

			if(indicator->node==NULL){
			    indicator->node=new_state;
			    if(state->tag>=0)
				tag_op(new_state,1);
			}else{
			    if(state->tag>=0){
				if(new_state->tag>=indicator->node->tag>0){
				    tag_op(new_state,1);
				    tag_op(indicator->node,0);
				    indicator->node=new_state;
				}
			    }
			}
		    }
		}

		
		setNodeData(new_state,num);
		if(new_state->allocated!=-1)
		    addInstance(new_state,vertex_sequence);

		if(ENV_NOSAVEINSTANCES){
		    // collect all states that ar created in this level k
		    // and delete them later on, when level k+1 has finished building!

		    if(new_state->mark!=-2){
			delInstances1.insert(new_state);
			new_state->mark=-2;
		    }
		}


		if(first_set){
		    memcpy(original_sequence,vertex_sequence,sizeof(INT_TYPE)*level);
		    first_state=new_state;
		    some_first_state=new_state;
		    first_set=0;
		  
		}
	    }
	}
    }

    delete [] original_sequence;

    if(ENV_NOSAVEINSTANCES){

	while(delInstances2.count()){
	    state=(DT_node*) delInstances2.remove(0);
	    state->inst_count[state->models_count]=0;
	    state->mark=0;
	    while(state->sequences.count()){
		delete (Indicator_type*) state->sequences_indicator.remove(0);
		delete [] (INT_TYPE*) state->sequences.remove(0);
	    }
	}
	delInstances1.mv(delInstances2);
    }
    
    delete [] adj_list;
    already_adj.clear2();

    return some_first_state;

}




int 
PG_class::isomorph1(Graph *G){
    
    DT_node* state;
    AM=&G->AM;
    int i,x,next;
  

    //    cout << "function PG_class::isomorph(Graph *G) O(n^2) called\n";

/* time measure starts here */
    //Timex.start();
    //Timex.start_log();
    //time_spent=Timex.time_stamp();
	
	time_spent = elapsed_time();
  
    checks=0;
    BRANCH_class::checks=0;
    
    state=RootNode;
    step=0;
    Dim=AM->Dimension;
    vertex_sequence=new INT_TYPE[Dim];
    edges_tuple=new int[Dim];
    H_seq=new int[Dim];
    
    Token *t=AM->orderCoherent();

    clear();
    
    for(x=0;x<Dim;x++)
	vertex_sequence[x]=(*t)[x];
    
    delete t;

    // traversing the tree	
    while(step<Dim){
	
	for(x=0;x<step;x++)
	  edges_tuple[x]=AM->isEdge(vertex_sequence[step],vertex_sequence[x]);
	    
	checks++;
	next=state->branch.test(edges_tuple,vertex_sequence[step],AM);
	
	if(next<0) {
	    Result_State=NULL;
	    delete [] vertex_sequence;
	    delete [] edges_tuple;
	    delete [] H_seq;
	    return 0;
	}
	
	state=makeOneStep(state->successors[next],step+1);	
	step++;
    }
  
  Result_State=state;
  collectMatches(vertex_sequence,Result_State);

    //checks+=BRANCH_class::checks;

    delete [] vertex_sequence;
    delete [] edges_tuple;
    delete [] H_seq;

    //time_spent=Timex.time_stamp()-time_spent;
	time_spent = elapsed_time() - time_spent;

    return 1;
    
}



int 
PG_class::prunedIsomorphism1(Graph *G){
    
    DT_node* state;
    AM=&G->AM;
    int i,x,next;
  
    cout << "function PG_class::prunedIsomorph(Graph *G) O(n^2) called\n";

/* time measure starts here */
    //Timex.start();
    //Timex.start_log();
    //time_spent=Timex.time_stamp();

	time_spent = elapsed_time();

  
    checks=0;
    BRANCH_class::checks=0;
    
    state=RootNode;
    step=0;
    Dim=AM->Dimension;
    vertex_sequence=new INT_TYPE[Dim];
    edges_tuple=new int[Dim];
    H_seq=new int[Dim];
    
    int* start_pos=new int[Dim];
    int* end_pos=new int[Dim];
    DT_node** the_state=new DT_node*[Dim];
    int parts=0;

    current_start_pos=0;

    Token *t=AM->orderCoherent();

    clear();
    
    for(x=0;x<Dim;x++)
	vertex_sequence[x]=(*t)[x];
    
    delete t;

    int lstep=0;
    
    // traversing the tree	
    while(lstep<Dim){
	
	for(x=0;x<step;x++)
	  edges_tuple[x]=AM->isEdge(vertex_sequence[step+current_start_pos],vertex_sequence[x+current_start_pos]);
	    
	checks++;
	next=state->branch.test(edges_tuple,vertex_sequence[step+current_start_pos],AM);
	
	if(next<0) {
	    Result_State=NULL;
	    return 0;
	}
	
	state=makeOneStep(state->successors[next],step+1);	
	step++;
	lstep++;


	if((state->succ==0)||(lstep==Dim)){
	    start_pos[parts]=current_start_pos;
	    end_pos[parts]=lstep;
	    the_state[parts]=state;
	    parts++;
	    step=0;
	    state=RootNode;
	    current_start_pos=lstep;
	    if(!orderInLine(vertex_sequence, lstep+1, Dim, lstep+1+ENV_DECISION_TREE_LIMIT))
		break;
	}
	    

    }
  

    // collect and check matches

    for(x=0;x<parts;x++)
	cout << x << ".part: " << the_state[x]->inst_count[0] << "\n";


    collectPrunedInstances(start_pos,end_pos,the_state,parts,vertex_sequence);


    delete [] vertex_sequence;
    delete [] edges_tuple;
    delete [] H_seq;

    delete [] start_pos;
    delete [] end_pos;
    delete [] the_state;

    //time_spent=Timex.time_stamp()-time_spent;
	time_spent = elapsed_time() - time_spent;

    return 1;
    
}
