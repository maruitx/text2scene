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

import org.ejml.alg.dense.decomposition.lu.LUDecompositionAlt;
import org.ejml.data.DenseMatrix64F;
import org.ejml.ops.RandomMatrices;
import org.junit.Test;

import java.util.Random;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;


/**
 * @author Peter Abeles
 */
public class TestUnrolledDeterminantFromMinor {

    Random rand = new Random(234234);

    @Test
    public void testAll() {
        for( int N = 2; N <= UnrolledDeterminantFromMinor.MAX; N++ ) {
            DenseMatrix64F A = RandomMatrices.createRandom(N,N,rand);

            double unrolled = UnrolledDeterminantFromMinor.det(A);
            LUDecompositionAlt alg = new LUDecompositionAlt();
            assertTrue( alg.decompose(A) );
            double expected = alg.computeDeterminant();

            assertEquals(expected,unrolled,1e-8);
        }
    }
}