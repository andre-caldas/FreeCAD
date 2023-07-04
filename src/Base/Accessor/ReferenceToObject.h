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
#include "PathToObject.h"

namespace Base {
class XMLReader;
class Writer;
}

namespace Base::Accessor
{

/**
 * @brief A @class PathToObject is not aware of the type
 * of variable it points to.
 * Through the means of the templated @class IExport<variable_type>,
 * objects can "export" the correct type to be referenced to by
 * @class ReferenceTo<variable_type> (\see @class IExport<>).
 * Also, classes and methods that expect a specific type of reference
 * shall declare the correct @class ReferenceTo<variable_type>.
 *
 * @example ReferenceTo<double> ref(root, "start point", "x");
 */
template<typename T>
class BaseExport ReferenceTo
        : public PathToObject
{
public:
    ReferenceTo(PathToObject&& path) : PathToObject(path) {}
    ReferenceTo(const PathToObject& path) : PathToObject(path) {}
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
    template<typename... NameOrTag>
    ReferenceTo(std::shared_ptr<ReferencedObject> root, NameOrTag&&... obj_path)
        : PathToObject(root, obj_path...) {}

    /**
     * @brief (DEPRECATED) References an object using a root
     * not managed by a shared_ptr.
     *
     * @param root - shared resource (raw pointer) where the path begins.
     * @param obj_path - list of token items that identify
     * the path from @root to the referenced resource.
     */
    template<typename... NameOrTag>
    ReferenceTo(ReferencedObject* root, NameOrTag&&... obj_path)
        : PathToObject(root, obj_path...) {}

    using lock_type = PathToObject::lock_type;
    /**
     * @brief Locks the last object and gets a raw pointer to @class T.
     * We assume the managed object owns the referenced @class T.
     *
     * The class @class PathToObject is not aware of the
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

    template<typename X, typename... NameOrTag>
    ReferenceTo<X> goFurther(NameOrTag&& ...furtherPath) const;

    static ReferenceTo<T> unserialize(Base::XMLReader& reader) {return ReferenceTo<T>{PathToObject::unserialize(reader)};}

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
