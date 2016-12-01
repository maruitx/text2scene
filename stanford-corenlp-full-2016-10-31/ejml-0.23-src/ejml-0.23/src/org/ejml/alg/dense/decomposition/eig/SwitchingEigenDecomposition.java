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

package org.ejml.alg.dense.decomposition.eig;

import org.ejml.data.Complex64F;
import org.ejml.data.DenseMatrix64F;
import org.ejml.factory.DecompositionFactory;
import org.ejml.factory.EigenDecomposition;
import org.ejml.ops.MatrixFeatures;


/**
 * Checks to see what type of matrix is being decomposed and calls different eigenvalue decomposition
 * algorithms depending on the results.  This primarily checks to see if the matrix is symmetric or not.
 *
 *
 * @author Peter Abeles
 */
public class SwitchingEigenDecomposition
        implements EigenDecomposition<DenseMatrix64F> {
    // tolerance used in deciding if a matrix is symmetric or not
    private double tol;

    EigenDecomposition<DenseMatrix64F> symmetricAlg;
    EigenDecomposition<DenseMatrix64F> generalAlg;

    boolean symmetric;
    // should it compute eigenvectors or just eigenvalues?
    boolean computeVectors;

    DenseMatrix64F A = new DenseMatrix64F(1,1);

    /**
     *
     * @param computeVectors
     * @param tol Tolerance for a matrix being symmetric
     */
    public SwitchingEigenDecomposition( int matrixSize , boolean computeVectors , double tol ) {
        symmetricAlg = DecompositionFactory.eig(matrixSize,computeVectors,true);
        generalAlg = DecompositionFactory.eig(matrixSize,computeVectors,false);
        this.computeVectors = computeVectors;
        this.tol = tol;
    }

    public SwitchingEigenDecomposition( int matrixSize ) {
        this(matrixSize,true,1e-8);
    }

    @Override
    public int getNumberOfEigenvalues() {
        return symmetric ? symmetricAlg.getNumberOfEigenvalues() :
                generalAlg.getNumberOfEigenvalues();
    }

    @Override
    public Complex64F getEigenvalue(int index) {
        return symmetric ? symmetricAlg.getEigenvalue(index) :
                generalAlg.getEigenvalue(index);
    }

    @Override
    public DenseMatrix64F getEigenVector(int index) {
        if( !computeVectors )
            throw new IllegalArgumentException("Configured to not compute eignevectors");

        return symmetric ? symmetricAlg.getEigenVector(index) :
                generalAlg.getEigenVector(index);
    }

    @Override
    public boolean decompose(DenseMatrix64F orig) {
        A.setReshape(orig);

        symmetric = MatrixFeatures.isSymmetric(A,tol);

        return symmetric ?
                symmetricAlg.decompose(A) :
                generalAlg.decompose(A);

    }

    @Override
    public boolean inputModified() {
        // since it doesn't know which algorithm will be used until a matrix is provided make a copy
        // of all inputs
        return false;
    }
}
