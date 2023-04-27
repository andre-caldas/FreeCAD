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

#include "../PreCompiled.h"

#ifndef _PreComp_
#include <string>
#endif // _PreComp_

#include <Base/Exception.h>

#include "Exception.h"

#include "ReferenceToObject.h"

namespace Base::Accessor {

std::string ReferenceToObject::pathString() const
{
    return pathString(object_path.cbegin(), object_path.cend());
}

std::string ReferenceToObject::pathString(token_iterator start, const token_iterator end)
{
    std::string result;
    for(auto pos = start; pos != end; ++pos)
    {
        result += pos->getText();
    }
    return result;
}

ReferenceToObject::lock_type ReferenceToObject::getLock() const
{
    // TODO: Implement different "cache expire" policies.
    lock_type lock;
    lock.pointers.reserve(object_path.size());
    lock.pointers.emplace_back(root.lock());
    if(!lock.pointers.front())
    {
        FC_THROWM(ExceptionCannotResolve, "Root object is not available. Path: '" << pathString() << "'.");
    }

    lock.remaining_tokens_start = object_path.cbegin();
    lock.remaining_tokens_end = object_path.cend();
    while(lock.remaining_tokens_start != lock.remaining_tokens_end)
    {
        auto previous_it = lock.remaining_tokens_start;
        auto ref_obj = dynamic_cast<Chainable*>(lock.pointers.back().get());
        if(!ref_obj)
        {
            // Current object is not chainable.
            break;
        }
        ref_obj->resolve(lock.remaining_tokens_start, object_path.cend());

        if(lock.remaining_tokens_start == previous_it)
        {
            FC_THROWM(Base::RuntimeError, "Object's path resolution is not consuming tokens. Path: '" << pathString() << "'. This is a BUG!");
        }
    }

    return lock;
}

} // namespace Base::Accessor
