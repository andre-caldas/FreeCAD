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

#include <utility>

#include <Base/Exception.h>
#include <App/Application.h>

#include "Exception.h"

#include <App/Document.h>

#include "ReferenceToObject.h"

namespace Base::Accessor {

class ReferencedObject;

template<typename... NameOrTag,
         std::enable_if_t<std::conjunction_v<
                 std::is_convertible<NameOrTag, NameAndTag>...
             >>*>
ReferenceToObject::ReferenceToObject(std::shared_ptr<ReferencedObject> root, NameOrTag&&... obj_path)
    : rootTag(root->getTag())
    , objectPath(std::initializer_list<NameAndTag>{std::forward(obj_path)...})
{
}

template<typename... NameOrTag,
         std::enable_if_t<std::conjunction_v<
                 std::is_convertible<NameOrTag, NameAndTag>...
             >>*>
ReferenceToObject::ReferenceToObject(ReferencedObject* root, NameOrTag&&... obj_path)
    : rootTag(root->registerTag("I know it is deprecated"))
    , objectPath({NameAndTag(std::forward<NameOrTag>(obj_path))...})
{
}

template<typename... NameOrTag,
         std::enable_if_t<std::conjunction_v<
                 std::is_convertible<NameOrTag, NameAndTag>...
             >>*>
ReferenceToObject::ReferenceToObject(NameOrTag&&... obj_path)
    : ReferenceToObject(App::GetApplication().getActiveDocument(), std::forward<NameOrTag>(obj_path)...)
{
}

template<typename X>
typename ReferenceTo<X>::result ReferenceTo<X>::getResult () const
{
    lock_type lock = getLock();
    if(lock.remaining_tokens_start == lock.remaining_tokens_end)
    {
        X* ptr = dynamic_cast<X*>(lock.last_object.get());
        if(!ptr)
        {
            FC_THROWM(ExceptionCannotResolve, "Last object is not a reference the requested type.");
        }
        return result{std::move(lock), ptr};
    }

    IExportPointer<X>* ref_obj;
    try
    {
        // We use reference so we get a e.what() to report.
        // TODO: If message is not useful, or if there is another way to get it,
        // change the cast to a pointer, not reference.
        ref_obj = &dynamic_cast<IExportPointer<X>&>(*lock.last_object);
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

template<typename X>
ReferenceTo<X>
ReferenceTo<X>::unserialize(Base::XMLReader& reader)
{
    reader.readElement("ReferenceTo");
    reader.readElement("RootTag");
    auto locked_root = ReferencedObject::getWeakPtr(reader.getCharacters()).lock();
    if(!locked_root)
    {
        FC_THROWM(ReferenceError, "Root element does not exist when unserializing RferenceTo: '" << reader.getCharacters() << "'");
    }
    ReferenceTo<X> result(locked_root);
    while(!reader.testEndElement("ReferenceTo"))
    {
        reader.readElement("NameOrTag");
        result.objectPath.push_back(NameAndTag(reader.getCharacters()));
    }
    return result;
}

template<typename T>
template<typename X, typename... NameOrTag,
         std::enable_if_t<std::conjunction_v<
                 std::is_convertible<NameOrTag, NameAndTag>...
             >>*>
ReferenceTo<X> ReferenceTo<T>::goFurther(NameOrTag&& ...furtherPath) const
{
    auto new_path = objectPath;
    new_path.insert(new_path.end(), {NameAndTag(std::forward<NameOrTag>(furtherPath))...});
    return ReferenceTo<X>{rootTag, new_path};
}

template<typename X>
bool ReferenceTo<X>::refreshLock()
{
    lockedResult = getResult();
}

/**
 * @brief Releases the internal @class result.
 */
template<typename X>
void ReferenceTo<X>::releaseLock()
{
    lockedResult.lock.last_object.reset();
}

template<typename X>
bool ReferenceTo<X>::isLocked() const
{
    return lockedResult.lock.last_object;
}

template<typename X>
X* ReferenceTo<X>::get() const
{
    if(!isLocked())
    {
        FC_THROWM(RuntimeError, "Trying to get a pointer to an object that is not locked.");
    }
//    lockedResult.lock.last_object->resolve the rest of the lock;
    FC_THROWM(NotImplementedError, "Not implemented!");
}

} // namespace Base::Accessor
