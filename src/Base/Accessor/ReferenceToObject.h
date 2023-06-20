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
 * 1. A @class Tag that identifies a root shared @class ReferencedObject.
 * 2. A sequence of @class NameAndTag that identifies the path
 * to the referenced entity.
 *
 * Instances of the @class ReferenceToObject are not aware of the type
 * of variable they point to.
 *
 * @attention This is a base class to @class ReferenceTo<T>.
 * Use @class ReferenceTo<T>, instead.
 */
class BaseExport ReferenceToObject
{
public:
    ReferenceToObject(const ReferenceToObject&) = default;

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
    Tag rootTag;
    token_list objectPath;
};

/**
 * @brief Instances of the @class ReferenceToObject are not aware of the type
 * of variable they point to. Through the means of the templated
 * @class ReferenceTo<variable_type>, objects can "export" the correct
 * @class ReferenceTo<variable_type> (\see @class IExport<>).
 * Also, classes and methods that expect a specific type of reference
 * shall declare the correct @class ReferenceTo<variable_type>.
 *
 * @example ReferenceTo<double> ref(root, "start point", "x");
 */
template<typename T>
class BaseExport ReferenceTo : public ReferenceToObject
{
public:
    using ReferenceToObject::ReferenceToObject;

    /**
     * @brief Locks the last object and gets a raw pointer to @class T.
     * We assume the managed object owns the referenced @class T.
     *
     * The base class @class ReferenceToObject is not aware of the
     * type of object being referenced. It resolves the token chain
     * up to the last object and "locks" it through a shared_ptr.
     * This lock also contains an iterator to the remaining tokens.
     *
     * To complete the resolution process, we expect that
     * this last found lock.last_object is of type:
     * * @class T, when there are no remaining tokens.
     * * @class IExport<T>, where there are tokens to be resolved.
     *
     * After resolving a pointer to T, we store the lock.last_object
     * and the T pointer into a shared_ptr<T>.
     */
    using locked_resource = std::shared_ptr<T>;

    /**
     * @brief Fully resolves the chain up to the last token.
     * @return A @class locked_resource holding a lock and a pointer.
     */
    locked_resource getResult() const;

    /**
     * @brief We keep an internal @class locked_resource.
     * This function resoleves the token path (again) and
     * stores the result internally (lockedResult).
     * @return True if lookup is successful.
     */
    bool refreshLock();

    /**
     * @brief Releases the internal @class locked_resource.
     */
    void releaseLock();

    bool isLocked() const;

    /**
     * @brief Gets a pointer to the locked resource.
     * It throws an exception if the internal @class locked_resource is not locked.
     * @return A pointer to the referenced resource.
     */
    T* get() const;

    bool hasChanged() const {return (old_reference != lockedResult.get());}
    T* getOldReference() const {return old_reference;}

    static ReferenceTo<T> unserialize(Base::XMLReader& reader);

    template<typename X, typename... NameOrTag,
             std::enable_if_t<std::conjunction_v<
                     std::is_convertible<NameOrTag, NameAndTag>...
                 >>* = nullptr>
    ReferenceTo<X> goFurther(NameOrTag&& ...furtherPath) const;

private:
    locked_resource lockedResult;
    /**
     * @brief The previous value of lockedResult.reference.
     * @attention It might be an invalid pointer!
     */
    T* old_reference = nullptr;
};

} //namespace Base::Accessor

#include "ReferenceToObject.inl"

#endif // BASE_Accessor_ReferenceToObject_H
