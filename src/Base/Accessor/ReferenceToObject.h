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

namespace Base {
class XMLReader;
class Writer;
}

namespace Base::Accessor
{

/**
 * @brief A reference to an object is more precisely,
 * a reference to a member of some object.
 * It is composed of:
 * 1. A \class Tag that identifies a root shared \class ReferencedObject.
 * 2. A sequence of \class NameAndTag that identifies the path
 * to the referenced entity.
 *
 * Instances of the \class ReferenceToObject are not aware of the type
 * of variable they point to.
 */
class BaseExport ReferenceToObject
{
public:
    /*
     * Variadic constructors! :-)
     * Thanks to @Artyer:
     * https://stackoverflow.com/a/76133079/1290711
     */

    /**
     * @brief References an object through an initial @a root
     * and a chain of strings/tags. The ownership of root
     * and every @class ReferencedObject in the path
     * shall be managed through a shared_ptr.
     *
     * @param root - shared resource (shared_ptr) where the path begins.
     * @param obj_path - list of token items that identify
     * the path from @root to the referenced resource.
     */
    template<typename... NameOrTag,
             std::enable_if_t<std::conjunction_v<
                     std::is_convertible<NameOrTag, NameAndTag>...
                 >>* = nullptr>
    ReferenceToObject(std::shared_ptr<ReferencedObject> root, NameOrTag&&... obj_path);

    /**
     * @brief (DEPRECATED) References an object using a root
     * not managed by a shared_ptr.
     *
     * @param root - shared resource (raw pointer) where the path begins.
     * @param obj_path - list of token items that identify
     * the path from @root to the referenced resource.
     */
    template<typename... NameOrTag,
             std::enable_if_t<std::conjunction_v<
                     std::is_convertible<NameOrTag, NameAndTag>...
                 >>* = nullptr>
    ReferenceToObject(ReferencedObject* root, NameOrTag&&... obj_path);

    /**
     * @brief References an object whose root is the current document.
     *
     * @param root - shared resource (weak_ptr) where the path begins.
     * @param obj_path - list of token items that identify
     * the path from @root to the referenced resource.
     */
    template<typename... NameOrTag,
             std::enable_if_t<std::conjunction_v<
                     std::is_convertible<NameOrTag, NameAndTag>...
                 >>* = nullptr>
    ReferenceToObject(NameOrTag&&... obj_path);

    ReferenceToObject(const Tag& tag, const token_list& path)
        : rootTag(tag)
        , objectPath(path) {}

    std::string pathString() const;
    static std::string pathString(token_iterator first, const token_iterator end);

    virtual ~ReferenceToObject() {}

    /**
     * @brief References are *NOT* serialized with knowledge
     * of what is is the most derived class.
     * When unserializing, the application needs to know what specific
     * subclass must be instantiated.
     * Then, serialization is implemented as a static method of the derived class.
     * @param writer - stream to write to.
     */
    void serialize(Base::Writer& writer) const;

protected:
    struct lock_type
    {
        pointer_list pointers;
        // TODO: Use ranges when in C++20.
        token_list::const_iterator remaining_tokens_start;
        token_list::const_iterator remaining_tokens_end;
    };
    lock_type getLock() const;

protected:
    Tag rootTag;
    token_list objectPath;
};

/**
 * @brief Instances of the \class ReferenceToObject are not aware of the type
 * of variable they point to. Through the means of the templated
 * \class ReferenceTo<variable_type>, objects can "export" the correct
 * \class ReferenceTo<variable_type> (\see \class IExport<>).
 * Also, classes and methods that expect a specific type of reference
 * shall declare the correct \class ReferenceTo<variable_type>.
 *
 * @example ReferenceTo<double> ref(root, "start point", "x");
 */
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

    static ReferenceTo<T> unserialize(Base::XMLReader& reader);

    template<typename X, typename... NameOrTag,
             std::enable_if_t<std::conjunction_v<
                     std::is_convertible<NameOrTag, NameAndTag>...
                 >>* = nullptr>
    ReferenceTo<X> goFurther(NameOrTag&& ...furtherPath) const;
};

} //namespace Base::Accessor

#include "ReferenceToObject.inl"

#endif // BASE_Accessor_ReferenceToObject_H
