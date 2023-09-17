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

#ifndef BASE_Accessor_ReferencedObject_H
#define BASE_Accessor_ReferencedObject_H

#include <mutex>
#include <memory>
#include <map>
#include <string>

#include <FCGlobal.h>

#include "NameAndTag.h"
#include "Types.h"

namespace Base::Accessor
{

template<typename T> class ReferenceTo;
template<typename T> class IExport;

/**
 * \brief A ReferencedObject is an object that can be queried
 * to resolve the next step in a path.
 */
class BaseExport ReferencedObject
    : public NameAndTag
    , public std::enable_shared_from_this<ReferencedObject>
{
public:
    virtual ~ReferencedObject() = default;

    template<typename X>
    std::shared_ptr<X> SharedFromThis();
    template<typename X>
    std::shared_ptr<const X> SharedFromThis() const;
    template<typename X>
    std::weak_ptr<X> WeakFromThis() noexcept;
    template<typename X>
    std::weak_ptr<const X> WeakFromThis() const noexcept;

    // Avoid static cast using this template.
    // Use: resolveType<T>(...).
    // TODO: use it when chain resolving.
    template<typename X>
    std::shared_ptr<X> resolveType(std::shared_ptr<ReferencedObject>& parent_lock, token_iterator& start, const token_iterator& end)
    {return static_cast<IExport<X>*>(this)->resolve(parent_lock, start, end);}

    // Avoid static cast using this template.
    // Use: getReferencesTo<T>().
    template<typename X>
    std::vector<ReferenceTo<X>> getReferencesTo();

    /**
     * @brief Globally registers a Tag.
     * This is specially useful when serializing (Save)
     * and unserializing (Restore).
     * @param weak_ptr - a weak_ptr to the @class ReferencedObject.
     */
    static void registerTag(const std::shared_ptr<ReferencedObject>& shared_ptr);

    /**
     * @brief Globally registers an unmanaged Tag.
     * @param deprecated - You have to pass "I know it is deprecated".
     * @return weak reference to generated "fake shared_ptr".
     * @attention This does not, in fact, owns the shared resource.
     * Object might be destructude without the returned weak reference
     * becoming invalid.
     */
    Tag::tag_type registerTag(std::string deprecated);

    /**
     * @brief When serializing (Save), tags are saved as strings.
     * When unserializing (Restore), the string can be used
     * to get a pointer to @class ReferencedObject.
     * @param tag - string representation of the tag.
     * @return A weak_ptr to the referenced object.
     */
    static std::weak_ptr<ReferencedObject> getWeakPtr(std::string_view tag);

    /**
     * @brief Same as above.
     * @param tag - the tag.
     * @return A weak_ptr to the referenced object.
     */
    static std::weak_ptr<ReferencedObject> getWeakPtr(Tag::tag_type tag);

private:
    /**
     * @brief Referenced objects can register themselves.
     * This is specialy useful when
     * serializing (Save) and unserializing (Restore) those pointers.
     */
    static std::map<Tag::tag_type, std::weak_ptr<ReferencedObject>> map;

    /**
     * @brief (DEPRECATED) For objects that do not use std::shared_ptr yet,
     * one can register a "fake shared_ptr" that does not in fact
     * manage the pointer.
     * @attention It is assumed that the pointed object is never destructed.
     */
    static std::map<const ReferencedObject*, std::shared_ptr<ReferencedObject>> deprecatedFakeSharePointers;

    /**
     * @brief Coordinates access to containers.
     */
    static std::mutex mutex;
};

template<typename T>
class BaseExport IExport
        : public virtual ReferencedObject
{
public:
    using export_type = std::shared_ptr<T>;
    using export_share_type = export_type;
    using export_ptr_type = T*;
    using token_iterator = Base::Accessor::token_iterator;

    // TODO: Use range in C++20.
    export_type resolve(std::shared_ptr<ReferencedObject>& parent_lock, token_iterator& start, const token_iterator& end);
    virtual std::vector<ReferenceTo<T>> getReferences(T* = nullptr);

protected:
    /*
     * To allow a class to derive from multiple IExport<T> classes,
     * we have added a T* to the end of the methods signature.
     */
    virtual T* resolve_ptr(token_iterator& start, const token_iterator& end, T* = nullptr);
    virtual export_type resolve_share(token_iterator& start, const token_iterator& end, T* = nullptr);
};

class BaseExport Chainable
    : public IExport<ReferencedObject>
{
};

} //namespace Base::Accessor

#include "ReferencedObject.inl"

#endif // BASE_Accessor_ReferencedObject_H
