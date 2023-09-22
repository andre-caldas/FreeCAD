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

#ifndef BASE_EnableSharedFromThis_H
#define BASE_EnableSharedFromThis_H

#include <memory>
#include <Base/Exception.h>

namespace Base::Accessor
{

/**
 * \brief A class with some improvements over std::enable_shared_from_this.
 */
template<typename Default>
class EnableSharedFromThis
    : public std::enable_shared_from_this<EnableSharedFromThis<Default>>
{
public:
bool destructed = false;
    virtual ~EnableSharedFromThis();

    std::shared_ptr<Default> lock();
    std::shared_ptr<Default> lock() const;

    bool HasSharedPtr() const noexcept;

    template<typename X>
    std::shared_ptr<X> SharedFromThis();
    template<typename X>
    std::shared_ptr<const X> SharedFromThis() const;

    template<typename X = Default>
    std::weak_ptr<X> WeakFromThis();
    template<typename X = Default>
    std::weak_ptr<const X> WeakFromThis() const;

    /**
     * @brief In FreeCAD we mix raw pointe ownership and shared_ptr.
     * Assuming this object has ben allocated (using new),
     * but is not managed by any smart pointer,
     * we construct a shared_ptr using the raw pointer "this".
     * This method is slightly racy.
     */
    template<typename X = Default>
    std::shared_ptr<X> TakeOwnershipFirst();

    /**
     * @brief In FreeCAD we mix raw pointe ownership and shared_ptr.
     * Assuming this object has ben allocated (using new),
     * but is not managed by any smart pointer,
     * we construct a shared_ptr using the raw pointer "this".
     * This method is slightly racy.
     */
    template<typename X = Default>
    std::shared_ptr<const X> TakeOwnershipFirst() const;
};

template<typename D>
std::shared_ptr<D> EnableSharedFromThis<D>::lock()
{
    return SharedFromThis<D>();
}

template<typename D>
std::shared_ptr<D> EnableSharedFromThis<D>::lock() const
{
    return SharedFromThis<D>();
}


template<typename D>
bool EnableSharedFromThis<D>::HasSharedPtr() const noexcept
{
    return !this->weak_from_this().expired();
}

template<typename D>
template<typename X>
std::shared_ptr<X> EnableSharedFromThis<D>::SharedFromThis()
{
    return std::dynamic_pointer_cast<X>(this->shared_from_this());
}

template<typename D>
template<typename X>
std::shared_ptr<const X> EnableSharedFromThis<D>::SharedFromThis() const
{
    return std::dynamic_pointer_cast<const X>(this->shared_from_this());
}

template<typename D>
template<typename X>
std::weak_ptr<X> EnableSharedFromThis<D>::WeakFromThis()
{
    return SharedFromThis<X>();
}

template<typename D>
template<typename X>
std::weak_ptr<const X> EnableSharedFromThis<D>::WeakFromThis() const
{
    return SharedFromThis<X>();
}

template<typename D>
template<typename X>
std::shared_ptr<X> EnableSharedFromThis<D>::TakeOwnershipFirst()
{
    assert(!this->HasSharedPtr());
    if(this->HasSharedPtr())
    {
        throw Base::RuntimeError("Trying to set ownership of a pointer that is already managed by a shared_ptr.");
    }
    return std::shared_ptr<X>{dynamic_cast<X*>(this)};
}

template<typename D>
template<typename X>
std::shared_ptr<const X> EnableSharedFromThis<D>::TakeOwnershipFirst() const
{
    return const_cast<EnableSharedFromThis<D>*>(this)->TakeOwnershipFirst<const X>();
}

template<typename D>
EnableSharedFromThis<D>::~EnableSharedFromThis()
{
destructed = true;
    assert(!this->HasSharedPtr());
}

} //namespace Base::Accessor

#endif // BASE_EnableSharedFromThis_H
