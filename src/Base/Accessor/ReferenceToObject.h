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

#ifndef BASE_Accessor_ReferenceToObject_H
#define BASE_Accessor_ReferenceToObject_H

#include <FCGlobal.h>

#include <type_traits>
#include <memory>
#include <vector>
#include <string>

// User shall not be required to include this.
#include "NameAndTag.h"
#include "Types.h"

namespace Base::Accessor
{

class BaseExport ReferenceToObject
{
public:
    ReferenceToObject() = delete;

    /**
     * @brief References an object through an initial @a root
     * and a chain of strings/tags. The ownership of root
     * and every @class ReferencedObject in the path
     * shall be managed through a shared_ptr.
     *
     * @param root - shared resource (weak_ptr) where the path begins.
     * @param obj_path - list of token items that identify
     * the path from @root to the referenced resource.
     */
    template<typename... NameOrTag>
    ReferenceToObject(std::weak_ptr<ReferencedObject> root,
                      std::enable_if_t<
                          std::conjunction_v
                          <
                              std::is_convertible_v<Base::Accessor::NameAndTag, NameOrTag>...
                          >
                      , NameOrTag&&>... obj_path);

    /**
     * @brief (DEPRECATED) References an object using a root
     * not managed by a shared_ptr.
     *
     * @param root - shared resource (raw pointer) where the path begins.
     * @param obj_path - list of token items that identify
     * the path from @root to the referenced resource.
     */
    template<typename... NameOrTag>
    ReferenceToObject(ReferencedObject* root,
                      std::enable_if_t<
                          std::conjunction_v
                          <
                              std::is_convertible_v<Base::Accessor::NameAndTag, NameOrTag>...
                          >
                      , NameOrTag&&>... obj_path);

    /**
     * @brief References an object whose root is the current document.
     *
     * @param root - shared resource (weak_ptr) where the path begins.
     * @param obj_path - list of token items that identify
     * the path from @root to the referenced resource.
     */
    template<typename... NameOrTag>
    ReferenceToObject(std::enable_if_t<
                          std::conjunction_v
                          <
                              std::is_convertible_v<Base::Accessor::NameAndTag, NameOrTag>...
                          >
                      , NameOrTag&&>... obj_path);

    std::string pathString() const;
    static std::string pathString(token_iterator first, const token_iterator end);

    virtual ~ReferenceToObject() {}

    template<typename T>
    T& getObject ();

protected:
    struct lock_type
    {
        pointer_list pointers;
        // TODO: Use ranges when in C++20.
        token_list::const_iterator remaining_tokens_start;
        token_list::const_iterator remaining_tokens_end;
    };
    lock_type getLock() const;

private:
    token_list object_path;
    std::shared_ptr<ReferencedObject> _root; // This has to come before "root".
    std::weak_ptr<ReferencedObject> root;
};

template<typename T>
class BaseExport ReferenceTo : public ReferenceToObject
{
public:
    using ReferenceToObject::ReferenceToObject;

    struct result
    {
        lock_type lock;
        T* reference;
    };
    result getResult () const;
};

} //namespace Base::Accessor

#include "ReferenceToObject.inl"

#endif // BASE_Accessor_ReferenceToObject_H
