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

#ifndef BASE_Threads_Exception_H
#define BASE_Threads_Exception_H

#include <Base/Exception.h>

namespace Base::Threads
{

class ExceptionNoExclusiveOverNonExclusive : public Base::TypeError
{
public:
    ExceptionNoExclusiveOverNonExclusive()
        : Base::TypeError("Cannot lock exclusively a mutex that is already non-exclusive.") {}
};

class ExceptionExclusiveParentNotLocked : public Base::TypeError
{
public:
    ExceptionExclusiveParentNotLocked()
        : Base::TypeError("An exclusive lock cannot come after non-chainable locks.") {}
};

class ExceptionNoLocksAfterExclusiveLock : public Base::TypeError
{
public:
    ExceptionNoLocksAfterExclusiveLock()
        : Base::TypeError("After an exclusive lock there can be no other locks.") {}
};

class ExceptionNeedLockToAccessContainer : public Base::TypeError
{
public:
    ExceptionNeedLockToAccessContainer()
        : Base::TypeError("You do not have a lock for the container you are trying to access.") {}
};

} //namespace Base::Threads

#endif // BASE_Threads_Exception_H
