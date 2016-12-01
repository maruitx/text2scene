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
import org.ejml.ops.CommonOps;
import org.ejml.ops.EjmlUnitTests;
import org.ejml.ops.RandomMatrices;
import org.junit.Test;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Random;

import static org.junit.Assert.*;


/**
 * @author Peter Abeles
 */
public class TestMatrixMatrixMult {
    Random rand = new Random(121342);

    /**
     * Checks to see that it only accepts input matrices that have compatible shapes
     */
    @Test
    public void checkShapesOfInput() {
        CheckMatrixMultShape check = new CheckMatrixMultShape(MatrixMatrixMult.class);
        check.checkAll();
    }

    /**
     * Checks to see if the input 'c' matrix is not 'a' or 'b'
     */
    @Test
    public void checkInputInstance() throws IllegalAccessException {
        Method methods[] = MatrixMatrixMult.class.getMethods();
        for( Method method : methods ) {
            String name = method.getName();

            if( !name.contains("mult") )
                continue;


            // make sure it checks that the c matrix is not a or b
            try {
                DenseMatrix64F a = new DenseMatrix64F(2,2);
                DenseMatrix64F b = new DenseMatrix64F(2,2);
                invoke(method,2.0,a,b,a);
                fail("An exception should have been thrown");
            } catch( InvocationTargetException e ) {
                assertTrue(e.getTargetException() instanceof IllegalArgumentException );
            }

            try {
                DenseMatrix64F a = new DenseMatrix64F(2,2);
                DenseMatrix64F b = new DenseMatrix64F(2,2);
                invoke(method,2.0,a,b,b);
                fail("An exception should have been thrown");
            } catch( InvocationTargetException e ) {
                assertTrue(e.getTargetException() instanceof IllegalArgumentException );
            }
        }
    }

    /**
     * Use java reflections to get a list of all the functions.  From the name extract what
     * the function is supposed to do.  then compute the expected results.
     *
     * Correctness is tested against a known case.
     */
    @Test
    public void checkAllAgainstKnown() throws InvocationTargetException, IllegalAccessException {
        double d[] = new double[]{0,1,2,3,4,5,6,7,8,9,10,11,12};
        DenseMatrix64F a_orig = new DenseMatrix64F(2,3, true, d);
        DenseMatrix64F b_orig = new DenseMatrix64F(3,4, true, d);
        DenseMatrix64F c_orig = RandomMatrices.createRandom(2,4,rand);

        DenseMatrix64F r_orig = new DenseMatrix64F(2,4, true, 20, 23, 26, 29, 56, 68, 80, 92);

        checkResults(a_orig,b_orig,c_orig,r_orig);
    }

    /**
     * Creates a bunch of random matrices and computes the expected results using mult().
     *
     * The known case is needed since this test case tests against other algorithms in
     * this library, which could in theory be wrong.
     *
     * @throws InvocationTargetException
     * @throws IllegalAccessException
     */
    @Test
    public void checkAgainstRandomDiffShapes() throws InvocationTargetException, IllegalAccessException {

        for( int i = 1; i <= 4; i++ ) {
            for( int j = 1; j <= 4; j++ ) {
                for( int k = 1; k <= 4; k++ ) {
                    DenseMatrix64F a_orig = RandomMatrices.createRandom(i,j, rand);
                    DenseMatrix64F b_orig = RandomMatrices.createRandom(j,k, rand);
                    DenseMatrix64F c_orig = RandomMatrices.createRandom(i,k, rand);

                    DenseMatrix64F r_orig = RandomMatrices.createRandom(i,k,rand);

                    MatrixMatrixMult.mult_small(a_orig,b_orig,r_orig);

                    checkResults(a_orig,b_orig,c_orig,r_orig);
                }
            }
        }
    }

    /**
     * Sees if all the matrix multiplications produce the expected results against the provided
     * known solution.
     */
    private void checkResults( DenseMatrix64F a_orig ,
                               DenseMatrix64F b_orig ,
                               DenseMatrix64F c_orig ,
                               DenseMatrix64F r_orig )
            throws InvocationTargetException, IllegalAccessException
    {
        double alpha = 2.5;

        int numChecked = 0;
        Method methods[] = MatrixMatrixMult.class.getMethods();

        for( Method method : methods ) {
            String name = method.getName();
//            System.out.println(name);

            // only look at function which perform matrix multiplications
            if( !name.contains("mult") )
                continue;

            DenseMatrix64F a = a_orig.copy();
            DenseMatrix64F b = b_orig.copy();
            DenseMatrix64F c = c_orig.copy();

            boolean add = name.contains("multAdd");
            boolean hasAlpha = method.getGenericParameterTypes()[0] == double.class;

            if( name.contains("TransAB")) {
                transpose(a);
                transpose(b);
            } else if( name.contains("TransA")) {
                transpose(a);
            } else if( name.contains("TransB")) {
                transpose(b);
            }

            DenseMatrix64F expected = r_orig.copy();
            double []expectedData = expected.data;

            if( hasAlpha ) {
                for( int i = 0; i < expectedData.length; i++ ) {
                    expectedData[i] *= alpha;
                }
            }
            if( add ) {
                for( int i = 0; i < expectedData.length; i++ ) {
                    expectedData[i] += c_orig.get(i);
                }
            }

            invoke(method,alpha,a,b,c);

            EjmlUnitTests.assertEquals(expected,c,1e-12);
            numChecked++;
        }

        assertEquals(numChecked,32);
    }

    private void transpose( DenseMatrix64F a ) {
        DenseMatrix64F b = new DenseMatrix64F(a.numCols,a.numRows);
        CommonOps.transpose(a,b);
        a.setReshape(b);
    }

    public static void invoke(Method func,
                              double alpha,
                              DenseMatrix64F a, DenseMatrix64F b, DenseMatrix64F c)
            throws IllegalAccessException, InvocationTargetException {
        if( func.getParameterTypes().length == 3 ) {
            func.invoke(null, a, b, c);
        } else {
            if( func.getParameterTypes()[0] == double.class ) {
                if( func.getParameterTypes().length == 4 )
                    func.invoke(null,alpha, a, b, c);
                else
                    func.invoke(null,alpha, a, b, c,null);
            } else {
                func.invoke(null, a, b, c,null);
            }
        }
    }
}
