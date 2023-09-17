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

#include <type_traits>

#include "Exception.h"

#include "Types.h"
#include "ReferenceToObject.h"
#include "ReferencedObject.h"

namespace Base::Accessor
{

template<typename X>
std::shared_ptr<X> ReferencedObject::SharedFromThis()
{
    return std::dynamic_pointer_cast<X>(shared_from_this());
}

template<typename X>
std::shared_ptr<const X> ReferencedObject::SharedFromThis() const
{
    return std::dynamic_pointer_cast<const X>(shared_from_this());
}

template<typename X>
std::weak_ptr<X> ReferencedObject::WeakFromThis() noexcept
{
    return std::weak_ptr<X>{weak_from_this()};
}

template<typename X>
std::weak_ptr<const X> ReferencedObject::WeakFromThis() const noexcept
{
    return std::weak_ptr<const X>{weak_from_this()};
}


template<typename X>
std::vector<ReferenceTo<X>> ReferencedObject::getReferencesTo()
{
    IExport<X>* ptr = dynamic_cast<IExport<X>*>(this);
    if(!ptr)
    {
        FC_THROWM(ExceptionNoExport, "Cannot getReferencesTo().");
    }
    return ptr->getReferences();
}

template<typename T>
typename IExport<T>::export_type
IExport<T>::resolve(std::shared_ptr<ReferencedObject>& parent_lock, token_iterator& start, const token_iterator& end)
{
    auto share = resolve_share(start, end);
    if(share)
    {
        return share;
    }

    auto ptr = resolve_ptr(start, end);
    if(ptr)
    {
        return export_share_type{parent_lock, ptr};
    }
    return export_share_type();
}


template<typename T>
typename IExport<T>::export_ptr_type
IExport<T>::resolve_ptr(token_iterator& /*start*/, const token_iterator& /*end*/, T*)
{
    return nullptr;
}

template<typename T>
typename IExport<T>::export_share_type
IExport<T>::resolve_share(token_iterator& /*start*/, const token_iterator& /*end*/, T*)
{
    return export_share_type();
}

template<typename T>
typename std::vector<ReferenceTo<T>>
IExport<T>::getReferences(T*)
{
    return std::vector<ReferenceTo<T>>();
}

} //namespace Base::Accessor
