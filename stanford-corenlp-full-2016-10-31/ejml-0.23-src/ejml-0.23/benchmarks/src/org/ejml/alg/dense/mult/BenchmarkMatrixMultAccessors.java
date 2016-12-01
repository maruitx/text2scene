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

package org.ejml.alg.dense.mult;

import org.ejml.data.DenseMatrix64F;
import org.ejml.ops.RandomMatrices;

import java.util.Random;

/**
 * Compares different implementations of a reordered matrix multiplication that use different accessor functions.
 * but identical algorithms.
 *
 * @author Peter Abeles
 */
public class BenchmarkMatrixMultAccessors {

    /**
     * All reads/writes have been inline by hand
     */
    public static long inlined( DenseMatrix64F a , DenseMatrix64F b , DenseMatrix64F c )
    {
        long timeBefore = System.currentTimeMillis();

        double dataA[] = a.data;
        double dataB[] = b.data;
        double dataC[] = c.data;

        double valA;
        int indexCbase= 0;
        int endOfKLoop = b.numRows*b.numCols;

        for( int i = 0; i < a.numRows; i++ ) {
            int indexA = i*a.numCols;

            // need to assign dataC to a value initially
            int indexB = 0;
            int indexC = indexCbase;
            int end = indexB + b.numCols;

            valA = dataA[indexA++];

            while( indexB < end ) {
                dataC[indexC++] = valA*dataB[indexB++];
            }

            // now add to it
            while( indexB != endOfKLoop ) { // k loop
                indexC = indexCbase;
                end = indexB + b.numCols;

                valA = dataA[indexA++];

                while( indexB < end ) { // j loop
                    dataC[indexC++] += valA*dataB[indexB++];
                }
            }
            indexCbase += c.numCols;
        }

        return System.currentTimeMillis() - timeBefore;
    }

    /**
     * Wrapper functions with no bounds checking are used to access matrix internals
     */
    public static long wrapped( DenseMatrix64F a , DenseMatrix64F b , DenseMatrix64F c )
    {
        long timeBefore = System.currentTimeMillis();
        double valA;
        int indexCbase= 0;
        int endOfKLoop = b.numRows*b.numCols;

        for( int i = 0; i < a.numRows; i++ ) {
            int indexA = i*a.numCols;

            // need to assign dataC to a value initially
            int indexB = 0;
            int indexC = indexCbase;
            int end = indexB + b.numCols;

            valA = a.get(indexA++);

            while( indexB < end ) {
                c.set( indexC++ , valA*b.get(indexB++));
            }

            // now add to it
            while( indexB != endOfKLoop ) { // k loop
                indexC = indexCbase;
                end = indexB + b.numCols;

                valA = a.get(indexA++);

                while( indexB < end ) { // j loop
                    c.plus( indexC++ , valA*b.get(indexB++));
                }
            }
            indexCbase += c.numCols;
        }

        return System.currentTimeMillis() - timeBefore;
    }

    /**
     * Only sets and gets that are by row and column are used.
     */
    public static long access2d( DenseMatrix64F a , DenseMatrix64F b , DenseMatrix64F c )
    {
        long timeBefore = System.currentTimeMillis();

        for( int i = 0; i < a.numRows; i++ ) {

            for( int j = 0; j < b.numCols; j++ ) {
                c.set(i,j,a.get(i,0)*b.get(0,j));
            }

            for( int k = 1; k < b.numRows; k++ ) {
                for( int j = 0; j < b.numCols; j++ ) {
//                    c.set(i,j, c.get(i,j) + a.get(i,k)*b.get(k,j));
                    c.data[i*b.numCols+j] +=a.get(i,k)*b.get(k,j);
                }
            }
        }

        return System.currentTimeMillis() - timeBefore;
    }

    public static void main( String args[] ) {
        Random rand = new Random(9234243);

        int N = 1000;

        DenseMatrix64F A = RandomMatrices.createRandom(N,N,rand);
        DenseMatrix64F B = RandomMatrices.createRandom(N,N,rand);
        DenseMatrix64F C = new DenseMatrix64F(N,N);

        long timeInlined = inlined(A,B,C);
        long timeWrapped = wrapped(A,B,C);
        long time2D = access2d(A,B,C);

        System.out.println("inlined "+timeInlined+" wrapped "+timeWrapped+" access2d "+time2D);
    }
}
