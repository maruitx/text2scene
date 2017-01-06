#pragma once

// Wrapper of Graph Matcher from Messmer
/********************************************************************
Copyright 1993 University of Bern

All Rights Reserved

Permission to use this software and its documentation for any purpose
and without fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright notice and this
permission notice appear in supporting documentation. Do not distribute
without specific, written prior permission.

Author: Bruno T. Messmer
Email:  messmer@iam.unibe.ch
********************************************************************/


#include <iostream>
#include <stdlib.h>
#include <stdio.h>

#include <time.h>
#include <sys/types.h>
#include "Graph.h"
#include "GMClass.h"


const int FL = 0;

/* Define your own deletion cost function for edges */

static double edge_deletion(int label1,
	double* values1,
	int n1,
	int f1,
	int f2,
	double del_error,
	Graph *G){

	if (FL == 0){
		return del_error;
	}
	else{
		/* Impossible deletion */
		return INFTY_COST;
	}
}


/* Define your own substitution cost function                             */
/* Default calculates the weighted sum of the attribute value differences */

static double substitution_cost(int label1,
	double* values1, int n1,
	double *w1, double *t1,
	int label2,
	double* values2, int n2,
	double *w2, double *t2,
	double lw1,
	double lw2){
	int x, d;
	double delta, Err;
	Err = 0;

	if (label1 == label2){
		for (x = 0; x < n1; x++){
			delta = fabs(values1[x] - values2[x]);
			if (delta > t1[x])
				Err = Err + delta*w1[x];
		}
		return Err;
	}
	else{
		if (lw1 < 0) return INFTY_COST;
		else return lw1;
	}
}

class GMTMatcher
{
public:
	GMTMatcher();
	~GMTMatcher();

	void test();

public:
	KB *m_graphDatabase;
	Graph* m_queryGraph;
};

