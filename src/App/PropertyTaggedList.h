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

#include <utility>
#include <memory>
#include <string>
#include <boost/uuid/uuid.hpp>

#include <App/Property.h>
#include <Base/Exception.h>
#include <Base/Persistence.h>
#include <FCGlobal.h>

namespace Py {
class Object;
}

namespace App
{

class PropertyContainer;
class ObjectIdentifier;

class AppExport PropertyTaggedList : public Property
{
    TYPESYSTEM_HEADER_WITH_OVERRIDE();

public:
    const std::set<boost::uuids::uuid>& getTouchList() const {
        return _touchList;
    }

    void clearTouchList() {
        _touchList.clear();
    }

    virtual std::string_view listName() const = 0;

protected:
    std::set<boost::uuids::uuid> _touchList;
};

/** Helper class to implement PropertyTaggedLists */
template<typename T>
class AppExport PropertyTaggedListT
        : public PropertyTaggedList
        , public AtomicPropertyChangeInterface<PropertyTaggedListT<T>>
{
public:
    using ptr_handler = std::shared_ptr;
    using list_type = std::map<boost::uuids::uuid,ptr_handler<T>>;
    using key_type = boost::uuids::uuid;
    using parent_type = PropertyTaggedList;
    using atomic_change = typename AtomicPropertyChangeInterface<PropertyTaggedListT<T>>::AtomicPropertyChange;

    friend class atomic_change;

    boost::uuids::uuid addValue(ptr_handler<T>&& value) {
        boost::uuids::uuid uuid = value->getUuid();
        _lValueList.emplace(uuid, std::move(value));
        return uuid;
    }

    void removeValue(boost::uuids::uuid tag) {
        _lValueList.erase(tag);
    }

    // Much nicer if it is an iterator!
    const list_type &getValues() const {return _lValueList;}
    std::weak_ptr<T> getElement(const ObjectIdentifier &path) const;

    bool isSame(const Property& other) const override {
        if (static_cast<void*>(&other) == static_cast<void*>(this))
            return true;
        return this->getTypeId() == other.getTypeId()
            && this->getValue() == static_cast<decltype(this)>(&other)->getValue();
    }

protected:
    list_type _lValueList;
    virtual const char* xmlTagName() const = 0;

public:
    /*
     * Persistence
     */
    unsigned int getMemSize() const override;
    void Save(Base::Writer& writer) const override;
    void Restore(Base::XMLReader& reader);

    /*
     * Base getters and setters
     */
    PyObject* getPyObject() override;
    void setPyObject(PyObject* value) override;

    /*
     * Property getters and setters.
     */
    void setPathValue(const App::ObjectIdentifier & path, const boost::any & value) override;
    const boost::any getPathValue(const App::ObjectIdentifier & path) const override;
    bool getPyPathValue(const App::ObjectIdentifier &, Py::Object &) const override;
    App::ObjectIdentifier canonicalPath(const App::ObjectIdentifier & p) const override;
    void getPaths(std::vector<App::ObjectIdentifier> & paths) const override;

//    boost::signals2::signal<void (const std::map<App::ObjectIdentifier, App::ObjectIdentifier> &)> signalGeometriesRenamed;
//    boost::signals2::signal<void (const std::set<App::ObjectIdentifier> &)> signalGeometriesRemoved;

//    App::ObjectIdentifier createPath(boost::uuids::uuid tag) const;
};

} // namespace App

#endif // APP_PROPERTY_H
