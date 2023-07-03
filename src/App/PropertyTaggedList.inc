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

#include <cassert>
#include <type_traits>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/string_generator.hpp>

#include <Base/Exception.h>
#include <Base/Accessor/Exception.h>
#include <Base/Console.h>

#include <Base/Reader.h>
#include <Base/Writer.h>
#include <Base/Tools.h>

#include <App/ObjectIdentifier.h>
#include <App/ExpressionParser.h>

#include "PropertyTaggedList.h"

// Templates cannot be in a ".cpp". So, to use a console, we need to use
// FC_LOG_LEVEL_INIT("PropertyTaggedList", true, true)
// But then, the cpp that includes this ".inc" would not be able to declare
// its own log level.
// This is a hack to allow it.
#ifdef FC_LOG_INSTANCE
#define OLD_FC_LOG_INSTANCE FC_LOG_INSTANCE
#undef FC_LOG_INSTANCE
#endif
#define FC_LOG_INSTANCE _fc_log_insance_tagged_list
FC_LOG_LEVEL_INIT("PropertyTaggedList", true, true)

namespace App {

template<typename T>
typename PropertyTaggedListT<T>::key_type
PropertyTaggedListT<T>::addElement(ptr_handler element)
{
    key_type uuid = element->getTag();
    elementList.emplace(uuid, std::move(element));
    return uuid;
}

template<typename T>
typename PropertyTaggedListT<T>::ptr_handler
PropertyTaggedListT<T>::getElement(key_type tag)
{
    try
    {
        return elementList.at(tag);
    }
    catch(std::out_of_range& e)
    {
        FC_THROWM(Base::ReferenceError, "Object (" << tag << ") not a list element.");
    }
}

template<typename T>
std::shared_ptr<T> PropertyTaggedListT<T>::resolve_share(token_iterator& start, const token_iterator& end, T*)
{
    assert(start != end);
    if(start == end)
    {
        FC_THROWM(Base::RuntimeError, "Empty token list. This is a bug!");
    }

    for(auto& element: *this)
    {
        if(element->pointsToMe(*start))
        {
            ++start;
            return element;
        }
    }

    FC_THROWM(Base::Accessor::ExceptionCannotResolve, "Resource (" << start->toString() << ") not found in PropertyTaggedListT.");
}

template<typename T>
std::shared_ptr<Base::Accessor::ReferencedObject>
PropertyTaggedListT<T>::resolve_share(token_iterator& start, const token_iterator& end, ReferencedObject*)
{
    static_assert(std::is_base_of_v<ReferencedObject,T>);
    return resolve_share(start, end, (T*)nullptr);
}

template<typename T>
typename PropertyTaggedListT<T>::iterator PropertyTaggedListT<T>::begin() const
{
    return iterator(elementList.begin());
}

template<typename T>
typename PropertyTaggedListT<T>::iterator PropertyTaggedListT<T>::end() const
{
    return iterator(elementList.end());
}


template<typename T>
unsigned int PropertyTaggedListT<T>::getMemSize() const
{
    int size = sizeof(T);
    for (auto& item: elementList)
    {
        size += item.second->getMemSize();
    }
    return size;
}

template<typename T>
void PropertyTaggedListT<T>::Save(Base::Writer& writer) const
{
    writer.Stream() << writer.ind() << "<" << xmlTagName() << ">" << std::endl;
    writer.incInd();
    for(auto& item: elementList)
    {
        item.second->Save(writer);
    }
    writer.decInd();
    writer.Stream() << writer.ind() << "</" << xmlTagName() << ">" << std::endl;
}

template<typename T>
void PropertyTaggedListT<T>::Restore(Base::XMLReader& reader)
{
    reader.readElement(xmlTagName());
    while(reader.testEndElement(xmlTagName()))
    {
        typename T::factory factory;
        addElement(factory.produceFromXml(reader));
    }
}


template<typename T>
PyObject* PropertyTaggedListT<T>::getPyObject()
{
    FC_THROWM(Base::NotImplementedError, "Not Implemented");
}

template<typename T>
void PropertyTaggedListT<T>::setPyObject(PyObject* /*value*/)
{
    FC_THROWM(Base::NotImplementedError, "Not Implemented");
}


template<typename T>
const boost::any PropertyTaggedListT<T>::getPathValue(const ObjectIdentifier& /*path*/) const
{
    FC_THROWM(Base::NotImplementedError, "Not Implemented");
}

template<typename T>
void PropertyTaggedListT<T>::setPathValue(const App::ObjectIdentifier& /*path*/, const boost::any& /*value*/)
{
    FC_THROWM(Base::NotImplementedError, "Not Implemented");
}

template<typename T>
bool PropertyTaggedListT<T>::getPyPathValue(const App::ObjectIdentifier&, Py::Object&) const
{
    FC_THROWM(Base::NotImplementedError, "Not Implemented");
}

template<typename T>
App::ObjectIdentifier PropertyTaggedListT<T>::canonicalPath(const App::ObjectIdentifier& /*p*/) const
{
    FC_THROWM(Base::NotImplementedError, "Not Implemented");
}



template<typename T>
void PropertyTaggedListT<T>::getPaths(std::vector<ObjectIdentifier>& paths) const
{
    for (auto& element: elementList) {
        paths.push_back(ObjectIdentifier(*this) << ObjectIdentifier::SimpleComponent(element.second->Tag::toString()));
        if (element.second->hasName())
            paths.push_back(ObjectIdentifier(*this) << ObjectIdentifier::SimpleComponent(element.second->getText()));
    }
}

} // namespace App

#undef FC_LOG_INSTANCE
#ifdef OLD_FC_LOG_INSTANCE
#define FC_LOG_INSTANCE OLD_FC_LOG_INSTANCE
#undef OLD_FC_LOG_INSTANCE
#endif
