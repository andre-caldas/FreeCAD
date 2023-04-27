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

#include <Base/Exception.h>
#include <App/Application.h>

#include "Exception.h"

#include "ReferencedObject.h"
#include "ReferenceToObject.h"

namespace Base::Accessor {

template<typename... Strings, typename>
ReferenceToObject::ReferenceToObject(std::weak_ptr<ReferencedObject> root, Strings&&... obj_path)
    : object_path(std::initializer_list<std::string>{std::forward(obj_path)...})
    , root(root)
{
}

template<typename... Strings, typename>
ReferenceToObject::ReferenceToObject(ReferencedObject* root, Strings&&... obj_path)
    : object_path(std::initializer_list<std::string>{std::forward(obj_path)...})
    , _root(root, [](auto /*p*/){}) // fake shared_ptr.
    , root(_root)
{
}

template<typename... Strings, typename>
ReferenceToObject::ReferenceToObject(Strings&&... obj_path)
    : ReferenceToObject(App::GetApplication().getActiveDocument(), std::forward(obj_path)...)
{
}

template<typename X>
typename ReferenceTo<X>::result ReferenceTo<X>::getResult () const
{
    lock_type lock = getLock();
    if(lock.remaining_tokens_start == lock.remaining_tokens_end)
    {
        return result{std::move(lock), lock.pointers.back()};
    }

    IExportPointer<X>* ref_obj;
    try
    {
        // We use reference so we get a e.what() to report.
        // TODO: If message is not useful, or if there is another way to get it,
        // change the cast to a pointer, not reference.
        ref_obj = &dynamic_cast<IExportPointer<X>&>(*lock.pointers.back());
    } catch (const std::bad_cast& e) {
        FC_THROWM(ExceptionCannotResolve, "Last object does not reference the requested type. " << e.what());
    }

    X* ref = ref_obj->resolve(lock.remaining_tokens_start, lock.remaining_tokens_end);
    if(!ref)
    {
        FC_THROWM(ExceptionCannotResolve, "Object does not recognize key: '" << pathString() << "'.");
    }

    if(lock.remaining_tokens_start != lock.remaining_tokens_end)
    {
        FC_THROWM(ExceptionCannotResolve, "Did not use all keys when resolving object. Remaining keys: '" << pathString(lock.remaining_tokens_start, lock.remaining_tokens_end) << "'.");
    }

    return result{std::move(lock), ref};
}

} // namespace Base::Accessor
