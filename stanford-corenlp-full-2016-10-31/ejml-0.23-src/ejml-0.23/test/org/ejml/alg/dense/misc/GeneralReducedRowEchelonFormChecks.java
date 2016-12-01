/*
 * Copyright (c) 2009-2012, Peter Abeles. All Rights Reserved.
 *
 * This file is part of Efficient Java Matrix Library (EJML).
 *
 * EJML is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3
 * of the License, or (at your option) any later version.
 *
 * EJML is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with EJML.  If not, see <http://www.gnu.org/licenses/>.
 */

package org.ejml.alg.dense.misc;

import org.ejml.data.DenseMatrix64F;
import org.ejml.factory.ReducedRowEchelonForm;
import org.ejml.ops.CommonOps;
import org.ejml.ops.RandomMatrices;
import org.junit.Test;

import java.util.Random;

import static org.junit.Assert.*;

/**
 * Generalized checks for Gauss-Jordan implementations
 *
 * @author Peter Abeles
 */
public abstract class GeneralReducedRowEchelonFormChecks {

    Random rand = new Random(234);

    ReducedRowEchelonForm alg;

    public GeneralReducedRowEchelonFormChecks(ReducedRowEchelonForm alg) {
        this.alg = alg;
    }

    /**
     * See if it is reducing systems into RREF
     */
    @Test
    public void testFormat() {
        for( int i = 1; i < 10; i++ ) {
            // test square
            checkFormatRandom(1+i,1+i);
            // test wide
            checkFormatRandom(3 + i, 4 + i * 2);
            // test tall
            checkFormatRandom(4 + i * 2,3 + i);
        }
    }

    /**
     * Solve several linear systems and check against solution
     */
    @Test
    public void testSolution() {
        checkSolutionRandom(3,4,3);
        checkSolutionRandom(3,5,3);
        // Tall won't work because with this test because the system is inconsistent
//        checkSolutionRandom(10,4,3);
    }

    @Test
    public void testSingular() {
        DenseMatrix64F A = new DenseMatrix64F(3,4,true,1,2,3,4,3,5,6,7,2,4,6,8,-3,4,9,3);

        DenseMatrix64F found = A.copy();
        alg.reduce(found,3);

        checkRref(found,3);

        DenseMatrix64F A1 = CommonOps.extract(A,0,3,0,3);
        DenseMatrix64F X = CommonOps.extract(found,0,3,3,4);
        DenseMatrix64F B = new DenseMatrix64F(3,1);

        CommonOps.mult(A1,X,B);

        for( int i = 0; i < 3; i++ )
            assertEquals(A.get(i,3),B.get(i,0),1e-8);
    }

    /**
     * Feed it specific matrices and see if it dies a horrible death
     */
    @Test
    public void spotTests() {
        DenseMatrix64F A = new DenseMatrix64F(4,6,true,
                0,0,1,-1,-1,4,
                2,4,2,4,2,4,
                2,4,3,3,3,4,
                3,6,6,3,6,6);

        alg.reduce(A,5);
        checkRref(A,5);
    }

    private void checkFormatRandom(int numRows, int numCols) {
        DenseMatrix64F A = RandomMatrices.createRandom(numRows,numCols,-1,1,rand);
        DenseMatrix64F found = A.copy();

        alg.reduce(found,numCols);

        checkRref(found, numCols);
    }

    private void checkSolutionRandom(int numRows, int numCols , int solWidth ) {
        DenseMatrix64F A = RandomMatrices.createRandom(numRows,numCols,-1,1,rand);
        DenseMatrix64F found = A.copy();

        alg.reduce(found,solWidth);

        checkRref(found,solWidth);

        DenseMatrix64F A1 = CommonOps.extract(A,0,numRows,0,solWidth);
        DenseMatrix64F X = CommonOps.extract(found,0,solWidth,solWidth,numCols);
        DenseMatrix64F B = new DenseMatrix64F(numRows,numCols-solWidth);

        CommonOps.mult(A1,X,B);

        for( int i = 0; i < numRows; i++ )
            for( int j = 0; j < numCols-solWidth; j++ )
                assertEquals(A.get(i,j+solWidth),B.get(i,j),1e-8);
    }


    /**
     * Checks to see if the provided matrix is in reduced row echelon format
     * @param A
     */
    private void checkRref( DenseMatrix64F A , int systemWidth ) {
        int prevLeading = -1;

        for( int row = 0; row < A.numRows; row++ ) {

            // find the next leading
            for( int col = 0; col < systemWidth; col++ ) {
                double val = A.get(row,col);

                if( val == 1 ) {
                    if( prevLeading > col )
                        fail("The next leading one should be at a later column than the previous");
                    prevLeading = col;

                    for( int i = 0; i < A.numRows; i++ ) {
                        if( i == row ) continue;
                        assertTrue("Column should be all zeros, except at the leading",0==A.get(i,col));
                    }

                    break;
                } else {
                    assertEquals("Should be all zeros before the leading 1", 0, val, 1e-8);
                }
            }
        }
    }
}
