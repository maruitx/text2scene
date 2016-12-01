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

import org.ejml.factory.EigenDecomposition;
import org.junit.Test;


/**
 * @author Peter Abeles
 */
public class TestSymmetricQRAlgorithmDecomposition extends GeneralEigenDecompositionCheck {
    boolean together;

    @Override
    public EigenDecomposition createDecomposition() {
        SymmetricQRAlgorithmDecomposition alg = new SymmetricQRAlgorithmDecomposition(computeVectors);
        if( computeVectors )
            alg.setComputeVectorsWithValues(together);

        return alg;
    }

    @Test
    public void justSymmetricTests_separate() {
        together = false;
        computeVectors = true;

        checkRandomSymmetric();
        checkIdentity();
        checkAllZeros();
        checkWithSomeRepeatedValuesSymm();
        checkWithSingularSymm();
        checkSmallValue(true);
        checkLargeValue(true);

        computeVectors = false;
        checkKnownSymmetric_JustValue();
    }

    @Test
    public void justSymmetricTests_together() {
        together = true;
        computeVectors = true;

        checkRandomSymmetric();
        checkIdentity();
        checkAllZeros();
        checkWithSomeRepeatedValuesSymm();
        checkWithSingularSymm();
        checkSmallValue(true);
        checkLargeValue(true);

        computeVectors = false;
        checkKnownSymmetric_JustValue();
    }
}
