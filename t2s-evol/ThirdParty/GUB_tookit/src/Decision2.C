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
PG_class::merge2(Graph *G,int num){
    DT_node* state;
    AM=&G->AM;
    int i,x,next;
    INT_TYPE* permutation;
    
    //Timex.start();
    //Timex.start_log();
    //time_spent=Timex.time_stamp();

	time_spent = elapsed_time();
    
    current_start_pos=0;

    if((num>=MAX_MODELS)||(Models_in_tree[num]!=NULL)){
	cout << "Model number collision! exiting....\n";
	exit(10);
    }
    Models_in_tree[num]=G;
    NumberOfModels=num;
    if(num>1){
	printf("ERROR: This verison of DT_class4.C does not support multiple models!\n");
	exit(10);
    }

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


    HashSeq1=new MultiIndex;
    HashSeq2=new MultiIndex;

    for(int s=1;s<=Dim;s++){
      /*
       * create all subset of size s and retrieve them one
       * by one with nextSubset
       */
      
      BUILDING_TREE=0;
      
      if((s>=ENV_BUILD_LEVEL)){
	
	printf("level %d of %d\n",s,Dim);
	
	if((ENV_FAST_BUILD)&&(s<=Dim-ENV_BYTREE_DIFF)) 
	  first_state=buildfromSubsets_bytree2(G,num,s);

	
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
	  }
	    
	  state=RootNode;
	  PASSED_BY_NODE=0;
	  path_trace=1;
	  
	  step=0;
	  while(step<s){
	    
	    
	    for(x=0;x<step;x++)
	      edges_tuple[x]=AM->isEdge(vertex_sequence[step],vertex_sequence[x]);
	      
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
	  vertex_checked=NULL;
	  delete [] AVAIL;
	  AVAIL=NULL;
	}
	
	
	if(continueSequences==0) break;
      }
      
      if(first_state!=NULL){
	first_state->terminal=1;
	if(state->depth<minSizedModel)
	  minSizedModel=state->depth;
      }
      
      delete HashSeq1;
      delete HashSeq2;
      
      
      if(ENV_NOSAVEINSTANCES){

	while(delInstances2.count()){
	  state=(DT_node*) delInstances2.remove(0);
	  
	  if(state->tag==-50000){
	    
	    state->inst_count[state->models_count]=0;
	    state->mark=0;
	    while(state->sequences.count()){
	      delete (Indicator_type*) state->sequences_indicator.remove(0);
	      delete [] (INT_TYPE*) state->sequences.remove(0);
	    }
	    
	    if(state->allocated==-1)
	      delete[] state->permutation;
	    delete [] state->models;
	    delete [] state->inst_count;
	    delete [] state->successors;
	    delete state;
	    
	  }	
	}
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
PG_class::run2(DT_node* state,int st,int level){
// uses the global vertex_sequence and the edge_tuple
    int x,next,hold;
    DT_node* direct_state,*previous_state;

    step=st;
    direct_state=state;

    int lstep;

    while(step<level){
	lstep=step-1;
	
	do{
	    lstep++;
	    if(lstep>=level) break;

	    for(x=0;x<step;x++)
		edges_tuple[x]=AM->isEdge(vertex_sequence[lstep],vertex_sequence[x]);
	
	    checks++;
	    next=state->branch.test(edges_tuple,vertex_sequence[lstep],AM);
	    
	    if(next>=0){
		if(state->successors[next]==NULL) next=-1;
		else if(state->successors[next]->tag<=0) next=-1;
	    }


	}while((next<0));

	if(next<0){
	    if(debug){
		cout << "\n Failed at:\n";
		for(x=0;x<step+1;x++)
		cout << vertex_sequence[x] << " ";
	    }
	    Result_State=NULL;
	    return NULL;
	}else{
	    hold=vertex_sequence[step];
	    vertex_sequence[step]=vertex_sequence[lstep];
	    vertex_sequence[lstep]=hold;
	    if((draw_func)&&(!ENV_ONLY_TRUE_STATES))
		draw_func(state->mark,state->successors[next]->mark);

	}
	
	previous_state=state;
	direct_state=state->successors[next];
	
	if(step+1<level){
	    state=makeOneStep(state->successors[next],step+1);	
	    
	    if((draw_func)&&(ENV_ONLY_TRUE_STATES))
		draw_func(previous_state->mark,state->mark);
	}else{
	    if((draw_func)&&(ENV_ONLY_TRUE_STATES)){
		DT_node* hstate=state->successors[next];
		while(hstate->allocated==-1) hstate=hstate->successors[next];
		draw_func(previous_state->mark,hstate->mark);
	    }
	}

	if(debug){
	    cout << "\n";
	    for(x=0;x<step+1;x++)
		cout << vertex_sequence[x] << " ";
	
	}

	step++;
    }

    
    return direct_state;
}



int
PG_class::prunedIsomorphism2(Graph *G){
// uses the global vertex_sequence and the edge_tuple
 
    DT_node* state;
    AM=&G->AM;
    int i,next;
  
    cout << "function PG_class::prunedIsomorph(Graph *G) called\n";

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


    if(!testing_consistency){
	vertex_sequence=new INT_TYPE[Dim];
	edges_tuple=new int[Dim];
	H_seq=new int[Dim];
    }

    int* start_pos=new int[Dim];
    int* end_pos=new int[Dim];
    DT_node** the_state=new DT_node*[Dim];
    int parts=0;

    current_start_pos=0;

    Token *t=AM->orderCoherent();

    clear();
    
    int x,hold;
    int level=Dim;
    if(!testing_consistency){
	for(x=0;x<Dim;x++)
	    vertex_sequence[x]=(*t)[x];
    }
    
    delete t;

    int lstep=0;

   
    DT_node* direct_state;

    step=0;
    direct_state=state;


    while(step<level){
	lstep=step-1;
	
	do{
	    lstep++;
	    if(lstep>=level) break;

	    for(x=0;x<step;x++)
		edges_tuple[x]=AM->isEdge(vertex_sequence[lstep],vertex_sequence[x]);
	
	    checks++;
	    next=state->branch.test(edges_tuple,vertex_sequence[lstep],AM);
	    
	    if(next>=0){
		if(state->successors[next]==NULL) next=-1;
		else if(state->successors[next]->tag<=0) next=-1;
	    }


	}while((next<0));

	if(next<0){
	    if(debug){
		cout << "\n Failed at:\n";
		for(x=0;x<step+1;x++)
		cout << vertex_sequence[x] << " ";
	    }
	    Result_State=NULL;
	    return 0;
	}else{
	    hold=vertex_sequence[step];
	    vertex_sequence[step]=vertex_sequence[lstep];
	    vertex_sequence[lstep]=hold;
	}
	
	direct_state=state->successors[next];
	
	if(step+1<level)
	    state=makeOneStep(state->successors[next],step+1);	
	    
	step++;

	if((state->succ==0)||(step==Dim)){
	    start_pos[parts]=current_start_pos;
	    end_pos[parts]=lstep;
	    the_state[parts]=state;
	    parts++;
	    step=0;
	    state=RootNode;
	    current_start_pos=lstep;
	    break;
	}


	if(debug){
	    cout << "\n";
	    for(x=0;x<step+1;x++)
		cout << vertex_sequence[x] << " ";
	
	}

    }
     // collect and check matches

    for(x=0;x<parts;x++)
	cout << x << ".part: " << the_state[x]->inst_count[0] << "\n";


    collectPrunedInstances(start_pos,end_pos,the_state,parts,vertex_sequence);

    if(!testing_consistency){
	delete [] vertex_sequence;
	delete [] edges_tuple;
	delete [] H_seq;
    }

    delete [] start_pos;
    delete [] end_pos;
    delete [] the_state;

    //time_spent=Timex.time_stamp()-time_spent;
	time_spent = elapsed_time() - time_spent;

    return 1;
}




DT_node*
PG_class::buildfromSubsets_bytree2(Graph* G,int num, int level){
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
    INT_TYPE* hold_sequence,*vt;
    hold_sequence=new INT_TYPE[level];
    vt=new INT_TYPE[level];
    int xx;

    original_sequence=new INT_TYPE[level+1];
    
    for(x=0;x<AM->numberOfVertices();x++){
	v=new int;
	*v=x;
	adj_list[0].insert(v);
    }


    DT_node* some_first_state=NULL;
  
    bstep=0;
   
    initSubset(level,Dim);

    
    HashSeq1->init(level);

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

	    if(level<ENV_SEARCHING_LINEAR){
	      if(level>1) {
		memcpy(hold_sequence,vertex_sequence,(level-1)*sizeof(INT_TYPE));
		
		default_valid=1;
		initSequences(level-1,0);
		
		while(nextSequences(vt)){
		  
		  for(xx=0;xx<level-1;xx++) vertex_sequence[xx]=hold_sequence[vt[xx]];
		  
		  state=run2(RootNode,0,level-1);
		  if(state) break;
	    
		}
		default_valid=0;
	      }else {
		state=run2(RootNode,0,level-1);
	      }

	      if(!state){
		continue;
	      }

	      state=makeOneStep(state,level-1);
	    }else{
		
		if(ENV_NOFASTSEARCH)
		    state=searchLinearInStates(vertex_sequence,level-1);
		else{
		    List* L=(List*) HashSeq2->get(vertex_sequence,level-1);
		    state=NULL;
		    if(L){
			L->reset();
			while(state=(DT_node*) L->getnext()){
			    if((state->tag>0)&&(state->allocated!=-1))
				break;
			}
		    }
		}
		    
		if(!state) continue;
	    }
	    
	    
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
		
		return_last_always=1;
		new_state=run2(state,level-1,level);
		return_last_always=0;

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
			    if(state->tag>0){
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

		    DT_node* hs=new_state;
		    DT_node* hs2=first_state;

		    if((first_state!=NULL)&&(ENV_NOREDIRECT==0)){
		      int fl1=0;
		      do {
			do {
			  if(hs==hs2) fl1=1;
			  if(hs2->allocated==-1)
			    hs2=hs2->successors[0];
			  else
			    break;
			}while(1);
			hs2=first_state;
			if(hs->allocated==-1)
			  hs=hs->successors[0];
			else
			  break;
		      }while(1);

		      if(fl1==0){

			redirectNode(first_state,edges_tuple,original_sequence,
						level,new_state,vertex_sequence,
						num,vertex_sequence[level-1]);
			if(first_state->tag>0){
			  for(int ii=0;ii<first_state->tag;ii++)
			    tag_op(new_state,1);
			}

			memcpy(original_sequence,vertex_sequence,sizeof(INT_TYPE)*level);
			first_state=new_state;
			some_first_state=new_state;
			
		      }
		    }

		    
		    //tag necessray nodes for breadth-pruning
		    if(level<=1){
			new_state->tag+=FIRST_TAG;
			FIRST_TAG=GLOBAL_TAG;
		    }else{

			if(indicator->node==NULL){
			    indicator->node=new_state;
			    if(state->tag>0)
				tag_op(new_state,1);
			}else{
			    if(state->tag>0){
				if(new_state->tag>=indicator->node->tag>0){
				    tag_op(new_state,1);
				    tag_op(indicator->node,0);
				    indicator->node=new_state;
				}
			    }
			}
		    }
		}

		if(new_state==NULL) continue;

		setNodeData(new_state,num);
		if(new_state->allocated!=-1)
		    addInstance(new_state,vertex_sequence);
		HashSeq1->insert(new_state,vertex_sequence,level);


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
    delete [] hold_sequence;
    delete [] vt;

    HashSeq2->clear();
    MultiIndex* H3=HashSeq2;
    HashSeq2=HashSeq1;
    HashSeq1=H3;

   
    if(ENV_NOSAVEINSTANCES){

	while(delInstances2.count()){
	    state=(DT_node*) delInstances2.remove(0);
	    state->inst_count[state->models_count]=0;
	    state->mark=0;
	    while(state->sequences.count()){
		delete (Indicator_type*) state->sequences_indicator.remove(0);
		delete [] (INT_TYPE*) state->sequences.remove(0);
	    }

	    // also set  all successor states with tag==0 to tag=-50000
	    // and delete them in the next round

	    if(state->allocated!=-1){
		for(x=0;x<state->succ;x++){
		    if(state->successors[x]->tag<=0){
			state->successors[x]->tag=-50000;
			state->successors[x]=NULL;
		    }
		}
	    }

	    if(state->tag==-50000){
		if(state->allocated==-1)
		    delete[] state->permutation;
		delete [] state->models;
		delete [] state->inst_count;
		delete [] state->successors;
		delete state;
	    }	
	}
	delInstances1.mv(delInstances2);
    }
    
    delete [] adj_list;
    already_adj.clear2();

    return some_first_state;
}




int 
PG_class::isomorph2(Graph *G){
    
    DT_node* state;
    AM=&G->AM;
    int i,x,next;
  
 
    if(ENV_CONSISTENCY_TEST){
	if(!consistencyTest(G))
	    cout << "Error....\n";
	else
	    cout << "Tree is consistent for all possible sequences\n";
	
    }

    //    cout << "function PG_class::isomorph(Graph *G) (n^3) called\n";

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

    state=run2(RootNode,0,Dim);
    
    if(state==NULL) {
	delete [] vertex_sequence;
	delete [] edges_tuple;
	delete [] H_seq;
	return 0;
    }

  Result_State=makeOneStep(state,step);

  collectMatches(vertex_sequence,Result_State);

    //checks+=BRANCH_class::checks;

    delete [] vertex_sequence;
    delete [] edges_tuple;
    delete [] H_seq;

    //time_spent=Timex.time_stamp()-time_spent;
	time_spent = elapsed_time() - time_spent;

    return 1;
    
}









