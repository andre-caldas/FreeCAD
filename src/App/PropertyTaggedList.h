/***************************************************************************
 *   Copyright (c) 2002 Jürgen Riegel <juergen.riegel@web.de>              *
 *   Copyright (c) 2023 André Caldas <andre.em.caldas@gmail.com>           *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/


#ifndef APP_PropertyTaggedList_H
#define APP_PropertyTaggedList_H

#include <type_traits>
#include <utility>
#include <memory>
#include <string>
#include <boost/uuid/uuid.hpp>

#include <App/Property.h>
#include <Base/Exception.h>
#include <Base/Persistence.h>
#include <Base/Accessor/ReferenceToObject.h>
#include <Base/Accessor/ReferencedObject.h>

#include <FCGlobal.h>

namespace Py {
class Object;
}

namespace App
{

class PropertyContainer;
class ObjectIdentifier;

/**
 * @brief Base class for a list of properties.
 * Each property is identified not with a sequential number,
 * but using a UUID, instead.
 */
class AppExport PropertyTaggedList : public Property
{
    TYPESYSTEM_HEADER_WITH_OVERRIDE();
};


// TODO: this should go to some Base/Utils/ directory.
/**
 * @brief Iterators for std::map return an std::pair.
 * This returns only the mapped std::pair::second.
 */
template<typename T>
class MappedTypeIterator
        : public T::const_iterator
{
public:
    using list_iterator = typename T::const_iterator;
    using mapped_type = typename T::mapped_type;

    MappedTypeIterator() = default;
    MappedTypeIterator(const MappedTypeIterator&) = default;
    MappedTypeIterator(list_iterator it) : list_iterator(it) {}
    const mapped_type& operator*() const {return list_iterator::operator*().second;}
    const mapped_type* operator->() const {return &(list_iterator::operator*().second);}
};


/**
 * @brief Template to help on the subclassing of @class PropertyTaggedList.
 */
template<typename T>
class AppExport PropertyTaggedListT
        : public PropertyTaggedList
        , public Base::Accessor::IExport<T>
{
public:
    using key_type = boost::uuids::uuid;
    using ptr_handler = std::shared_ptr<T>;
    using list_type = std::map<key_type,ptr_handler>;
    using token_iterator = Base::Accessor::token_iterator;
    using list_node_type = typename list_type::node_type;
    using item_reference = Base::Accessor::ReferenceTo<T>;
    using reference_to_type = Base::Accessor::ReferenceTo<T>;

    using ReferencedObject = Base::Accessor::ReferencedObject;

    key_type addElement(ptr_handler&& element);
    ptr_handler getElement(key_type tag);
    list_node_type removeElement(key_type tag) { return elementList.extract(tag); }
    /**
     * @brief Implements @class IExportShared<T> resolution.
     * @param start
     * @param end
     * @return
     */
    std::shared_ptr<T> resolve_share(token_iterator& start, const token_iterator& end) override;

    using iterator = class MappedTypeIterator<list_type>;
    iterator begin() const;
    iterator end() const;

protected:
    list_type elementList;
    virtual const char* xmlTagName() const = 0;

public:
    /*
     * Persistence
     */
    unsigned int getMemSize() const override;
    void Save(Base::Writer& writer) const override;
    void Restore(Base::XMLReader& reader) override;

    /*
     * Base getters and setters
     */
    PyObject* getPyObject() override;
    void setPyObject(PyObject* value) override;

    /*
     * Property getters and setters.
     */
    void setPathValue(const App::ObjectIdentifier& path, const boost::any& value) override;
    const boost::any getPathValue(const App::ObjectIdentifier& path) const override;
    bool getPyPathValue(const App::ObjectIdentifier&, Py::Object&) const override;
    App::ObjectIdentifier canonicalPath(const App::ObjectIdentifier& p) const override;
    void getPaths(std::vector<App::ObjectIdentifier>& paths) const override;
};

} // namespace App

#include "PropertyTaggedList.inc"

#endif // APP_PROPERTY_H
