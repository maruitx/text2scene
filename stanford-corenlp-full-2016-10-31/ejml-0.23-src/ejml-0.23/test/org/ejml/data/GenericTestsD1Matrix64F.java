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

package org.ejml.data;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;


/**
 * @author Peter Abeles
 */
public abstract class GenericTestsD1Matrix64F extends GenericTestsMatrix64F {

    protected abstract D1Matrix64F createMatrix( int numRows , int numCols );

    public void allTests() {
        super.allTests();
        testReshape();
        testSetAndGet_1D();
    }

    public void testReshape() {
        D1Matrix64F mat = createMatrix(3,2);

        double []origData = mat.getData();

        mat.reshape(6,1, false);

        assertTrue(origData == mat.getData());
        assertEquals(1,mat.getNumCols());
        assertEquals(6,mat.getNumRows());
    }

    public void testSetAndGet_1D() {
        D1Matrix64F mat = createMatrix(3,4);

        int indexA = mat.getIndex(1,2);
        int indexB = mat.getIndex(2,1);

        assertTrue(indexA!=indexB);

        mat.set(indexA,2.0);

        assertEquals(0,mat.get(indexB),1e-6);
        assertEquals(2,mat.get(indexA),1e-6);
    }
}