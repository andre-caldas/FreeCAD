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

#include <utility>

#include <Base/Exception.h>
#include <Base/Reader.h>
#include <App/Application.h>

#include "Exception.h"

#include <App/Document.h>

#include "ReferenceToObject.h"

namespace Base::Accessor {

class ReferencedObject;

template<typename X>
typename ReferenceTo<X>::locked_resource ReferenceTo<X>::getResult() const
{
    lock_type lock = pathToObject.getLock();
    if(lock.remaining_tokens_start == lock.remaining_tokens_end)
    {
        X* ptr = dynamic_cast<X*>(lock.last_object.get());
        if(!ptr)
        {
            FC_THROWM(ExceptionCannotResolve, "Last object is not a reference the requested type.");
        }
        return locked_resource{std::move(lock.last_object), ptr};
    }

    IExport<X>* ref_obj = dynamic_cast<IExport<X>*>(lock.last_object.get());
    if(!ref_obj)
    {
        FC_THROWM(ExceptionCannotResolve, "Last object does not reference the requested type.");
    }

    locked_resource shared_resource = ref_obj->resolve(lock.last_object, lock.remaining_tokens_start, lock.remaining_tokens_end);
    if(!shared_resource)
    {
        FC_THROWM(ExceptionCannotResolve, "Object does not recognize key: '" << pathString() << "'.");
    }

    if(lock.remaining_tokens_start != lock.remaining_tokens_end)
    {
        FC_THROWM(ExceptionCannotResolve, "Did not use all keys when resolving object. Remaining keys: '" << pathToObject.pathString(lock.remaining_tokens_start, lock.remaining_tokens_end) << "'.");
    }

    return shared_resource;
}

template<typename T>
template<typename X, typename... NameOrTag>
ReferenceTo<X> ReferenceTo<T>::goFurther(NameOrTag&& ...furtherPath) const
{
    return ReferenceTo<X>{pathToObject.goFurther(furtherPath...)};
}

template<typename X>
bool ReferenceTo<X>::refreshLock()
{
    old_reference = lockedResult.get();
    lockedResult = getResult();
    return isLocked();
}

/**
 * @brief Releases the internal @class locked_resource.
 */
template<typename X>
void ReferenceTo<X>::releaseLock()
{
    lockedResult.reset();
}

template<typename X>
bool ReferenceTo<X>::isLocked() const
{
    if(lockedResult)
    {
        return true;
    }
    return false;
}

template<typename X>
X* ReferenceTo<X>::get() const
{
    if(!isLocked())
    {
        FC_THROWM(RuntimeError, "Trying to get a pointer to an object that is not locked.");
    }
    return lockedResult.get();
}

} // namespace Base::Accessor
