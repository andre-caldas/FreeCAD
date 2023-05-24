// SPDX-License-Identifier: LGPL-2.1-or-later
/****************************************************************************
 *                                                                          *
 *   Copyright (c) 2023 André Caldas <andre.em.caldas@gmail.com>            *
 *                                                                          *
 *   This file is part of FreeCAD.                                          *
 *                                                                          *
 *   FreeCAD is free software: you can redistribute it and/or modify it     *
 *   under the terms of the GNU Lesser General Public License as            *
 *   published by the Free Software Foundation, either version 2.1 of the   *
 *   License, or (at your option) any later version.                        *
 *                                                                          *
 *   FreeCAD is distributed in the hope that it will be useful, but         *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of             *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU       *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU Lesser General Public       *
 *   License along with FreeCAD. If not, see                                *
 *   <https://www.gnu.org/licenses/>.                                       *
 *                                                                          *
 ***************************************************************************/


#ifndef NAMEDSKETCHER_GCS_ImprovedMatrix_H
#define NAMEDSKETCHER_GCS_ImprovedMatrix_H

#include <Eigen/SparseCore>

namespace NamedSketcher::GCS
{

template<typename M>
class ImprovedMatrix
        : public Eigen::SparseMatrix<double,M>
{
    void removeVector(Index i)
    {
        // TODO: use memcpy... I don't really know.
        for(Index j=i+1; j < outerSize(); ++j)
        {
            m_outerIndex[j-1] = m_outerIndex[j];
        }

        if(IsRowMajor)
        {
            conservativeResize(rows()-1, cols());
        } else {
            conservativeResize(rows(), cols()-1);
        }
    }

    void addVector(Index i)
    {
        if(IsRowMajor)
        {
            conservativeResize(rows()+1, cols());
        } else {
            conservativeResize(rows(), cols()+1);
        }

        // TODO: use memcpy... I don't really know.
        for(Index j=i+1; j < outerSize(); ++j)
        {
            m_outerIndex[j] = m_outerIndex[j-1];
        }
        m_innerNonZeros[i] = 0;
    }
};

} // namespace NamedSketcher::GCS

#endif // NAMEDSKETCHER_GCS_ImprovedMatrix_H
