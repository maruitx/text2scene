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

package org.ejml;

import org.ejml.data.BlockMatrix64F;
import org.ejml.data.D1Matrix64F;
import org.ejml.data.DenseMatrix64F;
import org.ejml.data.Matrix64F;
import org.ejml.ops.CommonOps;


/**
 * @author Peter Abeles
 */
public class BenchmarkInstanceOf {

    public static final double SCALE = 1.1;

    public static final StuffA stuff = new StuffA();

    public interface Stuff
    {
        public void process( Stuff a, Matrix64F M );
    }

    public static class StuffA implements Stuff
    {

        @Override
        public void process(Stuff a, Matrix64F M) {

            if( M instanceof BlockMatrix64F) {
                CommonOps.scale(1.0,(BlockMatrix64F)M);
            } else if( M instanceof DenseMatrix64F) {
                CommonOps.scale(SCALE,(DenseMatrix64F)M);
//                CommonOps.scale(0.5,(DenseMatrix64F)M);
            } else if(M instanceof D1Matrix64F) {
                CommonOps.scale(1.0,(D1Matrix64F)M);
            } else {
               throw new IllegalArgumentException("Who knows");
            }
        }
    }

    public static void withIfStatement( DenseMatrix64F M )
    {
        if( M.numCols > 10 ) {
            CommonOps.scale(2.0,M);
        } else if( M.numRows > 12 ) {
            CommonOps.scale(2.0,M);
        } else {
            CommonOps.scale(SCALE,M);
//            CommonOps.scale(0.5,M);
        }
    }


    public static long processInstanceOf( DenseMatrix64F M , int N ) {

        long before = System.currentTimeMillis();

        for( int i = 0; i < N; i++ ) {
            stuff.process(null,M);
        }

        return System.currentTimeMillis() - before;
    }

    public static long processDirect( DenseMatrix64F M , int N ) {

        long before = System.currentTimeMillis();

        for( int i = 0; i < N; i++ ) {
            CommonOps.scale(SCALE,M);
//            CommonOps.scale(0.5,M);
        }

        return System.currentTimeMillis() - before;
    }

    public static long processIf( DenseMatrix64F M , int N ) {

        long before = System.currentTimeMillis();

        for( int i = 0; i < N; i++ ) {
            withIfStatement(M);
        }

        return System.currentTimeMillis() - before;
    }


    public static void main( String args[] ) {
        DenseMatrix64F A = new DenseMatrix64F(2,2,true,0.1,0.5,0.7,10.0);

        int N = 200000000;

        System.out.println("instanceof "+processInstanceOf(A,N));
        System.out.println("direct     "+processDirect(A,N));
        System.out.println("if         "+processIf(A,N));
    }
}
