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

package org.ejml.ops;

import org.ejml.alg.dense.decomposition.lu.LUDecompositionAlt;
import org.ejml.alg.dense.linsol.lu.LinearSolverLu;
import org.ejml.alg.dense.mult.CheckMatrixMultShape;
import org.ejml.alg.dense.mult.MatrixMatrixMult;
import org.ejml.data.DenseMatrix64F;
import org.ejml.data.RowD1Matrix64F;
import org.ejml.data.UtilTestMatrix;
import org.junit.Test;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Random;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * @author Peter Abeles
 */
public class TestCommonOps {

    Random rand = new Random(0xFF);
    double tol = 1e-8;

    @Test
    public void checkInputShape() {
        CheckMatrixMultShape check = new CheckMatrixMultShape(CommonOps.class);
        check.checkAll();
    }

    /**
     * Make sure the multiplication methods here have the same behavior as the ones in MatrixMatrixMult.
     */
    @Test
    public void checkAllMatrixMults() {
        int numChecked = 0;
        Method methods[] = CommonOps.class.getMethods();

        boolean oneFailed = false;

        for( Method method : methods ) {
            String name = method.getName();

            // only look at function which perform matrix multiplication
            if( !name.contains("mult") || name.contains("Element") || 
                    name.contains("Inner") || name.contains("Outer"))
                continue;

            boolean hasAlpha = method.getGenericParameterTypes().length==4;

            Method checkMethod = findCheck(name,hasAlpha);

            boolean tranA = false;
            boolean tranB = false;
            if( name.contains("TransAB")) {
                tranA = true;
                tranB = true;
            } else if( name.contains("TransA")) {
                tranA = true;
            } else if( name.contains("TransB")) {
                tranB = true;
            }
//            System.out.println("Function = "+name+"  alpha = "+hasAlpha);

            try {
                if( !checkMultMethod(method,checkMethod,hasAlpha,tranA,tranB) ) {
                    System.out.println("Failed: Function = "+name+"  alpha = "+hasAlpha);
                    oneFailed = true;
                }
            } catch (InvocationTargetException e) {
                throw new RuntimeException(e);
            } catch (IllegalAccessException e) {
                throw new RuntimeException(e);
            }
            numChecked++;
        }
        assertEquals(16,numChecked);
        assertTrue(!oneFailed);
    }

    private Method findCheck( String name , boolean hasAlpha ) {
        Method checkMethod;
        try {
            if( hasAlpha )
                checkMethod = MatrixMatrixMult.class.getMethod(
                        name,double.class,
                        RowD1Matrix64F.class, RowD1Matrix64F.class,RowD1Matrix64F.class);
            else
                checkMethod = MatrixMatrixMult.class.getMethod(
                        name, RowD1Matrix64F.class, RowD1Matrix64F.class,RowD1Matrix64F.class);
        } catch (NoSuchMethodException e) {
            checkMethod = null;
        }
        if( checkMethod == null ) {
            try {
            if( hasAlpha )
                checkMethod = MatrixMatrixMult.class.getMethod(
                        name+"_reorder",double.class,
                        RowD1Matrix64F.class, RowD1Matrix64F.class,RowD1Matrix64F.class);
            else
                checkMethod = MatrixMatrixMult.class.getMethod(
                        name+"_reorder", RowD1Matrix64F.class, RowD1Matrix64F.class,RowD1Matrix64F.class);
            } catch (NoSuchMethodException e) {
                throw new RuntimeException(e);
            }
        }
        return checkMethod;
    }

    private boolean checkMultMethod(Method method, Method checkMethod, boolean hasAlpha,
                                    boolean tranA, boolean tranB ) throws InvocationTargetException, IllegalAccessException {


        // check various sizes
        for( int i = 1; i < 40; i++ ) {
            DenseMatrix64F a;
            if( tranA ) a = RandomMatrices.createRandom(i+1,i,rand);
            else  a = RandomMatrices.createRandom(i,i+1,rand);

            DenseMatrix64F b;
            if( tranB ) b = RandomMatrices.createRandom(i,i+1,rand);
            else  b = RandomMatrices.createRandom(i+1,i,rand);

            DenseMatrix64F c = RandomMatrices.createRandom(i,i,rand);
            DenseMatrix64F c_alt = c.copy();

            if( hasAlpha ) {
                method.invoke(null,2.0,a,b,c);
                checkMethod.invoke(null,2.0,a,b,c_alt);
            } else {
                method.invoke(null,a,b,c);
                checkMethod.invoke(null,a,b,c_alt);
            }

            if( !MatrixFeatures.isIdentical(c_alt,c,tol))
                return false;
        }

        // check various sizes column vector
        for( int i = 1; i < 4; i++ ) {
            DenseMatrix64F a;
            if( tranA ) a = RandomMatrices.createRandom(i,i+1,rand);
            else  a = RandomMatrices.createRandom(i+1,i,rand);

            DenseMatrix64F b;
            if( tranB ) b = RandomMatrices.createRandom(1,i,rand);
            else  b = RandomMatrices.createRandom(i,1,rand);

            DenseMatrix64F c = RandomMatrices.createRandom(i+1,1,rand);
            DenseMatrix64F c_alt = c.copy();

            if( hasAlpha ) {
                method.invoke(null,2.0,a,b,c);
                checkMethod.invoke(null,2.0,a,b,c_alt);
            } else {
                method.invoke(null,a,b,c);
                checkMethod.invoke(null,a,b,c_alt);
            }

            if( !MatrixFeatures.isIdentical(c_alt,c,tol))
                return false;
        }
        return true;
    }

    @Test
    public void multInner() {
        DenseMatrix64F a = RandomMatrices.createRandom(10,4,rand);
        DenseMatrix64F found = RandomMatrices.createRandom(4,4,rand);
        DenseMatrix64F expected = RandomMatrices.createRandom(4,4,rand);

        CommonOps.multTransA(a, a, expected);
        CommonOps.multInner(a,found);

        assertTrue(MatrixFeatures.isIdentical(expected,found,tol));
    }

    @Test
    public void multOuter() {
        DenseMatrix64F a = RandomMatrices.createRandom(10,4,rand);
        DenseMatrix64F found = RandomMatrices.createRandom(10,10,rand);
        DenseMatrix64F expected = RandomMatrices.createRandom(10,10,rand);

        CommonOps.multTransB(a, a, expected);
        CommonOps.multOuter(a, found);

        assertTrue(MatrixFeatures.isIdentical(expected,found,tol));
    }
    
    @Test
    public void elementMult_two() {
        DenseMatrix64F a = RandomMatrices.createRandom(5,4,rand);
        DenseMatrix64F b = RandomMatrices.createRandom(5,4,rand);
        DenseMatrix64F a_orig = a.copy();

        CommonOps.elementMult(a,b);

        for( int i = 0; i < 20; i++ ) {
            assertEquals(a.get(i),b.get(i)*a_orig.get(i),1e-6);
        }
    }

    @Test
    public void elementMult_three() {
        DenseMatrix64F a = RandomMatrices.createRandom(5,4,rand);
        DenseMatrix64F b = RandomMatrices.createRandom(5,4,rand);
        DenseMatrix64F c = RandomMatrices.createRandom(5,4,rand);

        CommonOps.elementMult(a,b,c);

        for( int i = 0; i < 20; i++ ) {
            assertEquals(c.get(i),b.get(i)*a.get(i),1e-6);
        }
    }

    @Test
    public void elementDiv_two() {
        DenseMatrix64F a = RandomMatrices.createRandom(5,4,rand);
        DenseMatrix64F b = RandomMatrices.createRandom(5,4,rand);
        DenseMatrix64F a_orig = a.copy();

        CommonOps.elementDiv(a,b);

        for( int i = 0; i < 20; i++ ) {
            assertEquals(a.get(i),a_orig.get(i)/b.get(i),1e-6);
        }
    }

    @Test
    public void elementDiv_three() {
        DenseMatrix64F a = RandomMatrices.createRandom(5,4,rand);
        DenseMatrix64F b = RandomMatrices.createRandom(5,4,rand);
        DenseMatrix64F c = RandomMatrices.createRandom(5,4,rand);

        CommonOps.elementDiv(a,b,c);

        for( int i = 0; i < 20; i++ ) {
            assertEquals(c.get(i),a.get(i)/b.get(i),1e-6);
        }
    }

    @Test
    public void solve() {
        DenseMatrix64F a = new DenseMatrix64F(2,2, true, 1, 2, 7, -3);
        DenseMatrix64F b = RandomMatrices.createRandom(2,5,rand);
        DenseMatrix64F c = RandomMatrices.createRandom(2,5,rand);
        DenseMatrix64F c_exp = RandomMatrices.createRandom(2,5,rand);

        assertTrue(CommonOps.solve(a,b,c));
        LUDecompositionAlt alg = new LUDecompositionAlt();
        LinearSolverLu solver = new LinearSolverLu(alg);
        assertTrue(solver.setA(a));

        solver.solve(b,c_exp);

        EjmlUnitTests.assertEquals(c_exp,c,1e-8);
    }

    @Test
    public void transpose_inplace() {
        DenseMatrix64F mat = new DenseMatrix64F(3,3, true, 0, 1, 2, 3, 4, 5, 6, 7, 8);
        DenseMatrix64F matTran = new DenseMatrix64F(3,3);

        CommonOps.transpose(mat,matTran);
        CommonOps.transpose(mat);

        EjmlUnitTests.assertEquals(mat,matTran,1e-8);
    }

    @Test
    public void transpose() {
        DenseMatrix64F mat = new DenseMatrix64F(3,2, true, 0, 1, 2, 3, 4, 5);
        DenseMatrix64F matTran = new DenseMatrix64F(2,3);

        CommonOps.transpose(mat,matTran);

        assertEquals(mat.getNumCols(),matTran.getNumRows());
        assertEquals(mat.getNumRows(),matTran.getNumCols());

        for( int y = 0; y < mat.getNumRows(); y++ ){
            for( int x = 0; x < mat.getNumCols(); x++ ) {
                assertEquals(mat.get(y,x),matTran.get(x,y),1e-6);
            }
        }
    }

    @Test
    public void trace() {
        DenseMatrix64F mat = new DenseMatrix64F(3,3, true, 0, 1, 2, 3, 4, 5, 6, 7, 8);

        double val = CommonOps.trace(mat);

        assertEquals(12,val,1e-6);
    }

    @Test
    public void invert() {
        for( int i = 1; i <= 10; i++ ) {
            DenseMatrix64F a = RandomMatrices.createRandom(i,i,rand);

            LUDecompositionAlt lu = new LUDecompositionAlt();
            LinearSolverLu solver = new LinearSolverLu(lu);
            assertTrue(solver.setA(a));

            DenseMatrix64F a_inv = new DenseMatrix64F(i,i);
            DenseMatrix64F a_lu = new DenseMatrix64F(i,i);
            solver.invert(a_lu);

            CommonOps.invert(a,a_inv);
            CommonOps.invert(a);

            EjmlUnitTests.assertEquals(a,a_inv,1e-8);
            EjmlUnitTests.assertEquals(a_lu,a,1e-8);
        }
    }

    /**
     * Checked against by computing a solution to the linear system then
     * seeing if the solution produces the expected output
     */
    @Test
    public void pinv() {
        // check wide matrix
        DenseMatrix64F A = new DenseMatrix64F(2,4,true,1,2,3,4,5,6,7,8);
        DenseMatrix64F A_inv = new DenseMatrix64F(4,2);
        DenseMatrix64F b = new DenseMatrix64F(2,1,true,3,4);
        DenseMatrix64F x = new DenseMatrix64F(4,1);
        DenseMatrix64F found = new DenseMatrix64F(2,1);
        
        CommonOps.pinv(A,A_inv);

        CommonOps.mult(A_inv,b,x);
        CommonOps.mult(A,x,found);

        assertTrue(MatrixFeatures.isIdentical(b,found,1e-4));

        // check tall matrix
        CommonOps.transpose(A);
        CommonOps.transpose(A_inv);
        b = new DenseMatrix64F(4,1,true,3,4,5,6);
        x.reshape(2,1);
        found.reshape(4,1);

        CommonOps.mult(A_inv,b,x);
        CommonOps.mult(A, x, found);

        assertTrue(MatrixFeatures.isIdentical(b,found,1e-4));
    }

    @Test
    public void columnsToVectors() {
        DenseMatrix64F M = RandomMatrices.createRandom(4,5,rand);

        DenseMatrix64F v[] = CommonOps.columnsToVector(M, null);

        assertEquals(M.numCols,v.length);

        for( int i = 0; i < v.length; i++ ) {
            DenseMatrix64F a = v[i];

            assertEquals(M.numRows,a.numRows);
            assertEquals(1,a.numCols);

            for( int j = 0; j < M.numRows; j++ ) {
                assertEquals(a.get(j),M.get(j,i),1e-8);
            }
        }
    }
    
    @Test
    public void identity() {
        DenseMatrix64F A = CommonOps.identity(4);

        assertEquals(4,A.numRows);
        assertEquals(4,A.numCols);

        assertEquals(4,CommonOps.elementSum(A),1e-8);
    }

    @Test
    public void identity_rect() {
        DenseMatrix64F A = CommonOps.identity(4,6);

        assertEquals(4,A.numRows);
        assertEquals(6,A.numCols);

        assertEquals(4,CommonOps.elementSum(A),1e-8);
    }

    @Test
    public void setIdentity() {
        DenseMatrix64F A = RandomMatrices.createRandom(4,4,rand);

        CommonOps.setIdentity(A);

        assertEquals(4,A.numRows);
        assertEquals(4,A.numCols);

        assertEquals(4,CommonOps.elementSum(A),1e-8);
    }

    @Test
    public void diag() {
        DenseMatrix64F A = CommonOps.diag(2.0,3.0,6.0,7.0);

        assertEquals(4,A.numRows);
        assertEquals(4,A.numCols);

        assertEquals(2,A.get(0,0),1e-8);
        assertEquals(3,A.get(1,1),1e-8);
        assertEquals(6,A.get(2,2),1e-8);
        assertEquals(7,A.get(3,3),1e-8);

        assertEquals(18,CommonOps.elementSum(A),1e-8);
    }

    @Test
    public void diag_rect() {
        DenseMatrix64F A = CommonOps.diagR(4,6,2.0,3.0,6.0,7.0);

        assertEquals(4,A.numRows);
        assertEquals(6,A.numCols);

        assertEquals(2,A.get(0,0),1e-8);
        assertEquals(3,A.get(1,1),1e-8);
        assertEquals(6,A.get(2,2),1e-8);
        assertEquals(7,A.get(3,3),1e-8);

        assertEquals(18,CommonOps.elementSum(A),1e-8);
    }

    @Test
    public void kron() {
        DenseMatrix64F A = new DenseMatrix64F(2,2, true, 1, 2, 3, 4);
        DenseMatrix64F B = new DenseMatrix64F(1,2, true, 4, 5);

        DenseMatrix64F C = new DenseMatrix64F(2,4);
        DenseMatrix64F C_expected = new DenseMatrix64F(2,4, true, 4, 5, 8, 10, 12, 15, 16, 20);

        CommonOps.kron(A,B,C);

        assertTrue(MatrixFeatures.isIdentical(C,C_expected,1e-8));

        // test various shapes for problems
        for( int i = 1; i <= 3; i++ ) {
            for( int j = 1; j <= 3; j++ ) {
                for( int k = 1; k <= 3; k++ ) {
                    for( int l = 1; l <= 3; l++ ) {
                        A = RandomMatrices.createRandom(i,j,rand);
                        B = RandomMatrices.createRandom(k,l,rand);
                        C = new DenseMatrix64F(A.numRows*B.numRows,A.numCols*B.numCols);

                        CommonOps.kron(A,B,C);

                        assertEquals(i*k,C.numRows);
                        assertEquals(j*l,C.numCols);
                    }
                }
            }
        }
    }

    @Test
    public void extract() {
        DenseMatrix64F A = RandomMatrices.createRandom(5,5, 0, 1, rand);

        DenseMatrix64F B = new DenseMatrix64F(2,3);

        CommonOps.extract(A,1,3,2,5,B,0,0);

        for( int i = 1; i < 3; i++ ) {
            for( int j = 2; j < 5; j++ ) {
                assertEquals(A.get(i,j),B.get(i-1,j-2),1e-8);
            }
        }
    }

    @Test
    public void extract_ret() {
        DenseMatrix64F A = RandomMatrices.createRandom(5,5, 0, 1, rand);

        DenseMatrix64F B = CommonOps.extract(A,1,3,2,5);

        assertEquals(B.numRows,2);
        assertEquals(B.numCols,3);

        for( int i = 1; i < 3; i++ ) {
            for( int j = 2; j < 5; j++ ) {
                assertEquals(A.get(i,j),B.get(i-1,j-2),1e-8);
            }
        }
    }

    @Test
    public void extractDiag() {
        DenseMatrix64F a = RandomMatrices.createRandom(3,4, 0, 1, rand);

        for( int i = 0; i < 3; i++ ) {
            a.set(i,i,i+1);
        }

        DenseMatrix64F v = new DenseMatrix64F(3,1);
        CommonOps.extractDiag(a,v);

        for( int i = 0; i < 3; i++ ) {
            assertEquals( i+1 , v.get(i) , 1e-8 );
        }
    }

    @Test
    public void insert() {
        DenseMatrix64F A = new DenseMatrix64F(5,5);
        for( int i = 0; i < A.numRows; i++ ) {
            for( int j = 0; j < A.numCols; j++ ) {
                A.set(i,j,i*A.numRows+j);
            }
        }

        DenseMatrix64F B = new DenseMatrix64F(8,8);

        CommonOps.insert(A, B, 1,2);

        for( int i = 1; i < 6; i++ ) {
            for( int j = 2; j < 7; j++ ) {
                assertEquals(A.get(i-1,j-2),B.get(i,j),1e-8);
            }
        }
    }

   @Test
    public void addEquals() {
        DenseMatrix64F a = new DenseMatrix64F(2,3, true, 0, 1, 2, 3, 4, 5);
        DenseMatrix64F b = new DenseMatrix64F(2,3, true, 5, 4, 3, 2, 1, 0);

        CommonOps.addEquals(a,b);

        UtilTestMatrix.checkMat(a,5,5,5,5,5,5);
        UtilTestMatrix.checkMat(b,5,4,3,2,1,0);
    }

    @Test
    public void addEquals_beta() {
        DenseMatrix64F a = new DenseMatrix64F(2,3, true, 0, 1, 2, 3, 4, 5);
        DenseMatrix64F b = new DenseMatrix64F(2,3, true, 5, 4, 3, 2, 1, 0);

        CommonOps.addEquals(a,2.0,b);

        UtilTestMatrix.checkMat(a,10,9,8,7,6,5);
        UtilTestMatrix.checkMat(b,5,4,3,2,1,0);
    }

    @Test
    public void add() {
        DenseMatrix64F a = new DenseMatrix64F(2,3, true, 0, 1, 2, 3, 4, 5);
        DenseMatrix64F b = new DenseMatrix64F(2,3, true, 5, 4, 3, 2, 1, 0);
        DenseMatrix64F c = RandomMatrices.createRandom(2,3,rand);

        CommonOps.add(a,b,c);

        UtilTestMatrix.checkMat(a,0,1,2,3,4,5);
        UtilTestMatrix.checkMat(b,5,4,3,2,1,0);
        UtilTestMatrix.checkMat(c,5,5,5,5,5,5);
    }

    @Test
    public void add_beta() {
        DenseMatrix64F a = new DenseMatrix64F(2,3, true, 0, 1, 2, 3, 4, 5);
        DenseMatrix64F b = new DenseMatrix64F(2,3, true, 5, 4, 3, 2, 1, 0);
        DenseMatrix64F c = RandomMatrices.createRandom(2,3,rand);

        CommonOps.add(a,2.0,b,c);

        UtilTestMatrix.checkMat(a,0,1,2,3,4,5);
        UtilTestMatrix.checkMat(b,5,4,3,2,1,0);
        UtilTestMatrix.checkMat(c,10,9,8,7,6,5);
    }

    @Test
    public void add_alpha_beta() {
        DenseMatrix64F a = new DenseMatrix64F(2,3, true, 0, 1, 2, 3, 4, 5);
        DenseMatrix64F b = new DenseMatrix64F(2,3, true, 5, 4, 3, 2, 1, 0);
        DenseMatrix64F c = RandomMatrices.createRandom(2,3,rand);

        CommonOps.add(2.0,a,2.0,b,c);

        UtilTestMatrix.checkMat(a,0,1,2,3,4,5);
        UtilTestMatrix.checkMat(b,5,4,3,2,1,0);
        UtilTestMatrix.checkMat(c,10,10,10,10,10,10);
    }

    @Test
    public void add_alpha() {
        DenseMatrix64F a = new DenseMatrix64F(2,3, true, 0, 1, 2, 3, 4, 5);
        DenseMatrix64F b = new DenseMatrix64F(2,3, true, 5, 4, 3, 2, 1, 0);
        DenseMatrix64F c = RandomMatrices.createRandom(2,3,rand);

        CommonOps.add(2.0,a,b,c);

        UtilTestMatrix.checkMat(a,0,1,2,3,4,5);
        UtilTestMatrix.checkMat(b,5,4,3,2,1,0);
        UtilTestMatrix.checkMat(c,5,6,7,8,9,10);
    }

    @Test
    public void add_scalar_c() {
        DenseMatrix64F a = new DenseMatrix64F(2,3, true, 0, 1, 2, 3, 4, 5);
        DenseMatrix64F c = RandomMatrices.createRandom(2,3,rand);

        CommonOps.add(a,2.0,c);

        UtilTestMatrix.checkMat(a,0,1,2,3,4,5);
        UtilTestMatrix.checkMat(c,2,3,4,5,6,7);
    }

    @Test
    public void add_scalar() {
        DenseMatrix64F a = new DenseMatrix64F(2,3, true, 0, 1, 2, 3, 4, 5);

        CommonOps.add(a,2.0);

        UtilTestMatrix.checkMat(a,2,3,4,5,6,7);
    }

    @Test
    public void subEquals() {
        DenseMatrix64F a = new DenseMatrix64F(2,3, true, 0, 1, 2, 3, 4, 5);
        DenseMatrix64F b = new DenseMatrix64F(2,3, true, 5, 5, 5, 5, 5, 5);

        CommonOps.subEquals(a,b);

        UtilTestMatrix.checkMat(a,-5,-4,-3,-2,-1,0);
        UtilTestMatrix.checkMat(b,5,5,5,5,5,5);
    }

    @Test
    public void sub() {
        DenseMatrix64F a = new DenseMatrix64F(2,3, true, 0, 1, 2, 3, 4, 5);
        DenseMatrix64F b = new DenseMatrix64F(2,3, true, 5, 5, 5, 5, 5, 5);
        DenseMatrix64F c = RandomMatrices.createRandom(2,3,rand);

        CommonOps.sub(a,b,c);

        UtilTestMatrix.checkMat(a,0,1,2,3,4,5);
        UtilTestMatrix.checkMat(b,5,5,5,5,5,5);
        UtilTestMatrix.checkMat(c,-5,-4,-3,-2,-1,0);
    }

    @Test
    public void scale() {
        double s = 2.5;
        double d[] = new double[]{10,12.5,-2,5.5};
        DenseMatrix64F mat = new DenseMatrix64F(2,2, true, d);

        CommonOps.scale(s,mat);

        assertEquals(d[0]*s,mat.get(0,0),1e-8);
        assertEquals(d[1]*s,mat.get(0,1),1e-8);
        assertEquals(d[2]*s,mat.get(1,0),1e-8);
        assertEquals(d[3]*s,mat.get(1,1),1e-8);
    }

    @Test
    public void scale_two_input() {
        double s = 2.5;
        double d[] = new double[]{10,12.5,-2,5.5};
        DenseMatrix64F mat = new DenseMatrix64F(2,2, true, d);
        DenseMatrix64F r = new DenseMatrix64F(2,2, true, d);

        CommonOps.scale(s,mat,r);

        assertEquals(d[0],mat.get(0,0),1e-8);
        assertEquals(d[1],mat.get(0,1),1e-8);
        assertEquals(d[2],mat.get(1,0),1e-8);
        assertEquals(d[3],mat.get(1,1),1e-8);

        assertEquals(d[0]*s,r.get(0,0),1e-8);
        assertEquals(d[1]*s,r.get(0,1),1e-8);
        assertEquals(d[2]*s,r.get(1,0),1e-8);
        assertEquals(d[3]*s,r.get(1,1),1e-8);
    }

    @Test
    public void div() {
        double s = 2.5;
        double d[] = new double[]{10,12.5,-2,5.5};
        DenseMatrix64F mat = new DenseMatrix64F(2,2, true, d);

        CommonOps.divide(s,mat);

        assertEquals(d[0]/s,mat.get(0,0),1e-8);
        assertEquals(d[1]/s,mat.get(0,1),1e-8);
        assertEquals(d[2]/s,mat.get(1,0),1e-8);
        assertEquals(d[3]/s,mat.get(1,1),1e-8);
    }

    @Test
    public void div_two_input() {
        double s = 2.5;
        double d[] = new double[]{10,12.5,-2,5.5};
        DenseMatrix64F mat = new DenseMatrix64F(2,2, true, d);
        DenseMatrix64F r = new DenseMatrix64F(2,2, true, d);

        CommonOps.divide(s,mat,r);

        assertEquals(d[0],mat.get(0,0),1e-8);
        assertEquals(d[1],mat.get(0,1),1e-8);
        assertEquals(d[2],mat.get(1,0),1e-8);
        assertEquals(d[3],mat.get(1,1),1e-8);

        assertEquals(d[0]/s,r.get(0,0),1e-8);
        assertEquals(d[1]/s,r.get(0,1),1e-8);
        assertEquals(d[2]/s,r.get(1,0),1e-8);
        assertEquals(d[3]/s,r.get(1,1),1e-8);
    }

    @Test
    public void fill() {
        double d[] = new double[]{10,12.5,-2,5.5};
        DenseMatrix64F mat = new DenseMatrix64F(2,2, true, d);

        CommonOps.fill(mat, 1);

        for( int i = 0; i < mat.getNumElements(); i++ ) {
            assertEquals(1,mat.get(i),1e-8);
        }
    }

    @Test
    public void zero() {
        double d[] = new double[]{10,12.5,-2,5.5};
        DenseMatrix64F mat = new DenseMatrix64F(2,2, true, d);

        mat.zero();

        for( int i = 0; i < mat.getNumElements(); i++ ) {
            assertEquals(0,mat.get(i),1e-8);
        }
    }

    @Test
    public void elementMax() {
        DenseMatrix64F mat = new DenseMatrix64F(3,3, true, 0, 1, -2, 3, 4, 5, 6, 7, -8);

        double m = CommonOps.elementMax(mat);
        assertEquals(7,m,1e-8);
    }

    @Test
    public void elementMin() {
        DenseMatrix64F mat = new DenseMatrix64F(3,3, true, 0, 1, 2, -3, 4, 5, 6, 7, 8);

        double m = CommonOps.elementMin(mat);
        assertEquals(-3,m,1e-8);
    }

    @Test
    public void elementMinAbs() {
        DenseMatrix64F mat = new DenseMatrix64F(3,3, true, 0, 1, -2, 3, 4, 5, 6, 7, -8);

        double m = CommonOps.elementMinAbs(mat);
        assertEquals(0,m,1e-8);
    }

    @Test
    public void elementMaxAbs() {
        DenseMatrix64F mat = new DenseMatrix64F(3,3, true, 0, 1, 2, 3, 4, 5, -6, 7, -8);

        double m = CommonOps.elementMaxAbs(mat);
        assertEquals(8,m,1e-8);
    }

    @Test
    public void elementSum() {
        DenseMatrix64F M = RandomMatrices.createRandom(5,5,rand);
        // make it smaller than the original size to make sure it is bounding
        // the summation correctly
        M.reshape(4,3, false);

        double sum = 0;
        for( int i = 0; i < M.numRows; i++ ) {
            for( int j = 0; j < M.numCols; j++ ) {
                sum += M.get(i,j);
            }
        }

        assertEquals(sum,CommonOps.elementSum(M),1e-8);
    }

    @Test
    public void elementSumAbs() {
        DenseMatrix64F M = RandomMatrices.createRandom(5,5,rand);
        // make it smaller than the original size to make sure it is bounding
        // the summation correctly
        M.reshape(4,3, false);

        double sum = 0;
        for( int i = 0; i < M.numRows; i++ ) {
            for( int j = 0; j < M.numCols; j++ ) {
                sum += Math.abs(M.get(i,j));
            }
        }

        assertEquals(sum,CommonOps.elementSum(M),1e-8);
    }

    @Test
    public void sumRows() {
        DenseMatrix64F input = RandomMatrices.createRandom(4,5,rand);
        DenseMatrix64F output = new DenseMatrix64F(4,1);

        assertTrue( output == CommonOps.sumRows(input,output));

        for( int i = 0; i < input.numRows; i++ ) {
            double total = 0;
            for( int j = 0; j < input.numCols; j++ ) {
                total += input.get(i,j);
            }
            assertEquals( total, output.get(i),1e-8);
        }

        // check with a null output
        DenseMatrix64F output2 = CommonOps.sumRows(input,null);

        EjmlUnitTests.assertEquals(output,output2,1e-8);
    }

    @Test
    public void sumCols() {
        DenseMatrix64F input = RandomMatrices.createRandom(4,5,rand);
        DenseMatrix64F output = new DenseMatrix64F(1,5);

        assertTrue( output == CommonOps.sumCols(input,output));

        for( int i = 0; i < input.numCols; i++ ) {
            double total = 0;
            for( int j = 0; j < input.numRows; j++ ) {
                total += input.get(j,i);
            }
            assertEquals( total, output.get(i),1e-8);
        }

        // check with a null output
        DenseMatrix64F output2 = CommonOps.sumCols(input,null);

        EjmlUnitTests.assertEquals(output,output2,1e-8);
    }

    @Test
    public void rref() {
        DenseMatrix64F A = new DenseMatrix64F(4,6,true,
                0,0,1,-1,-1,4,
                2,4,2,4,2,4,
                2,4,3,3,3,4,
                3,6,6,3,6,6);

        DenseMatrix64F expected = new DenseMatrix64F(4,6,true,
                1,2,0,3,0,2,
                0,0,1,-1,0,2,
                0,0,0,0,1,-2,
                0,0,0,0,0,0);

        DenseMatrix64F found = CommonOps.rref(A,5, null);


        assertTrue(MatrixFeatures.isEquals(found,expected));
    }
}
