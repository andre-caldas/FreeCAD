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

#ifndef BASE_Accessor_PathToObject_H
#define BASE_Accessor_PathToObject_H

#include <FCGlobal.h>

#include <type_traits>
#include <memory>
#include <vector>
#include <string>

// User shall not be required to include this.
#include "NameAndUuid.h"
#include "Types.h"

namespace Base {
class XMLReader;
class Writer;
}

namespace Base::Accessor
{

/**
 * @brief A path to an object is more precisely,
 * a reference to a member of some object.
 * It is composed of:
 * 1. A url path to a document.
 * 1. A @class Uuid that identifies a root shared @class ReferencedObject.
 * 2. A sequence of @class NameAndUuid that identifies the path
 * to the referenced entity.
 *
 * Instances of the @class PathToObject are not aware of the type
 * of variable they point to.
 *
 * @attention This is internally used by @class ReferenceTo<T>,
 * and also used by the python bindings so we do not need
 * to create one binding for each possible @class ReferenceTo<T>.
 * Usually you should use @class ReferenceTo<T>, instead.
 */
class BaseExport PathToObject
{
public:
    PathToObject(PathToObject&&) = default;
    PathToObject(const PathToObject&) = default;
    PathToObject& operator= (const PathToObject&) = default;
    PathToObject& operator= (PathToObject&&) = default;

    /*
     * Variadic constructors! :-)
     * Thanks to @Artyer:
     * https://stackoverflow.com/a/76133079/1290711
     */

    /**
     * @brief References an object through an initial @a root
     * and a chain of strings/uuids. The ownership of root
     * and every @class ReferencedObject in the path
     * shall be managed through a shared_ptr.
     *
     * @param root - shared resource (shared_ptr) where the path begins.
     * @param obj_path - list of token items that identify
     * the path from @root to the referenced resource.
     */
    template<typename... NameOrUuid,
             std::enable_if_t<std::conjunction_v<
                     std::is_convertible<NameOrUuid, NameAndUuid>...
                 >>* = nullptr>
    PathToObject(std::shared_ptr<ReferencedObject> root, NameOrUuid&&... obj_path);

    /**
     * @brief (DEPRECATED) References an object using a root
     * not managed by a shared_ptr.
     *
     * @param root - shared resource (raw pointer) where the path begins.
     * @param obj_path - list of token items that identify
     * the path from @root to the referenced resource.
     */
    template<typename... NameOrUuid,
             std::enable_if_t<std::conjunction_v<
                     std::is_convertible<NameOrUuid, NameAndUuid>...
                 >>* = nullptr>
    PathToObject(ReferencedObject* root, NameOrUuid&&... obj_path);

    PathToObject(const Uuid& uuid, const token_list& path)
        : rootUuid(uuid)
        , objectPath(path) {}

    PathToObject(std::shared_ptr<ReferencedObject> root, const token_list& path);

    std::string pathString() const;
    static std::string pathString(token_iterator first, const token_iterator end);

    /**
     * @brief References are *NOT* serialized with knowledge
     * of what is is the most derived class.
     * When unserializing, the application needs to know what specific
     * subclass must be instantiated.
     * Then, serialization is implemented as a static method of the derived class.
     * @param writer - stream to write to.
     */
    void serialize(Base::Writer& writer) const;
    static PathToObject unserialize(Base::XMLReader& reader);

    virtual ~PathToObject() {}

    template<typename... NameOrUuid,
             std::enable_if_t<std::conjunction_v<
                     std::is_convertible<NameOrUuid, NameAndUuid>...
                 >>* = nullptr>
    PathToObject goFurther(NameOrUuid&& ...furtherPath) const;

    PathToObject operator+ (std::string extra_path) const {return goFurther(std::move(extra_path));}
    PathToObject operator+ (PathToObject extra_path) const {return PathToObject(*this) += extra_path;}
    PathToObject& operator+= (PathToObject extra_path);

    template<typename T>
    std::shared_ptr<T> resolveType();

    /**
     * @brief The resolution mechanism resolves the token chain
     * up to the end of the chain, or up to the first non-chainable object.
     * The last found object is kept (using a shared_ptr) at "last_object".
     *
     * In the context of @class ReferenceTo<X>,
     * this last found object has to be of type:
     * * @class X, when there are no remaining tokens.
     * * @class IExport<X>, where there are tokens to be resolved.
     */
    struct lock_type
    {
        object_pointer_type last_object;
        // TODO: Use ranges when in C++20.
        token_list::const_iterator remaining_tokens_start;
        token_list::const_iterator remaining_tokens_end;
    };

    /**
     * @brief Resolves the reference until it finds a non @class Chainable object,
     * or until the chain is fully consumed.
     * @return A lock to the shared resource and a "list" of the remaining tokens.
     * That is, a @class lock_type.
     */
    lock_type getLock() const;

protected:
    std::string documentUrl; // Not used yet!
    Uuid rootUuid;
    std::weak_ptr<ReferencedObject> rootWeakPtr;
    token_list objectPath;
};

} //namespace Base::Accessor

#include "PathToObject.inl"

#endif // BASE_Accessor_PathToObject_H
