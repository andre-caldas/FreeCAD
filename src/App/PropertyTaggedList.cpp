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

#include "PreCompiled.h"

#ifndef _PreComp_
#include <cassert>
#endif
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/string_generator.hpp>

#include <Base/Console.h>

#include <Base/Reader.h>
#include <Base/Writer.h>
#include <Base/Tools.h>

#include <App/ObjectIdentifier.h>
#include <App/ExpressionParser.h>

#include "PropertyTaggedList.h"

FC_LOG_LEVEL_INIT("Property", true, true)

TYPESYSTEM_SOURCE_ABSTRACT(App::PropertyTaggedList , App::Property)

namespace App {

template<typename T>
unsigned int PropertyTaggedListT<T>::getMemSize() const
{
    int size = sizeof(T);
    for (auto& item: _lValueList)
    {
        size += item->second->getMemSize();
    }
    return size;
}

template<typename T>
void PropertyTaggedListT<T>::Save(Base::Writer& writer) const
{
    writer.Stream() << writer.ind() << "<" << xmlTagName() << ">" << std::endl;
    writer.incInd();
    for(auto& item: _lValueList)
    {
        item->second->Save(writer);
    }
    writer.decInd();
    writer.Stream() << writer.ind() << "</" << xmlTagName() << ">" << std::endl;
}

template<typename T>
void PropertyTaggedListT<T>::Restore(Base::XMLReader &reader)
{
    reader.readElement(xmlTagName());
    while(reader.testEndElement(xmlTagName()))
    {
        addValue(T::factory(reader));
    }
}

template<typename T>
std::weak_ptr<T> PropertyTaggedListT<T>::getElement(const ObjectIdentifier &path) const
{
    if(path.numSubComponents()!=2 || path.getPropertyComponent(0).getName()!=getListName())
    {
        FC_THROWM(Base::ValueError,"Invalid constraint path " << path.toString());
    }

    const ObjectIdentifier::Component& c = path.getPropertyComponent(1);

    // Can't we simply check if name_or_tag is a tag?
    // It would be less noisy.
    try {
        auto tag = boost::uuids::string_generator()(c.getName());
        return _lValueList.at(tag);
    } catch(std::out_of_range&) {
        FC_WARN("TAG not found: '" << c.getName() << "'.");
        return std::weak_ptr<T>();
    } catch (std::runtime_error&) {
    }

    for (auto it = _lValueList.cbegin(); it != _lValueList.cend(); ++it) {
        if ((*it)->second->onlyName() == c.getName())
        {
            return (*it)->second;
        }
    }
    FC_WARN("Name (" << c.getName() << ") not found in path " << path.toString() << ".");
    return std::weak_ptr<T>();
}

template<typename T>
const boost::any PropertyTaggedListT<T>::getPathValue(const ObjectIdentifier &path) const
{
    auto element = getElement(path).lock();
    return boost::any(element->getValue());
}

template<typename T>
void PropertyTaggedListT<T>::getPaths(std::vector<ObjectIdentifier> &paths) const
{
    for (auto it = _lValueList.begin(); it != _lValueList.end(); ++it) {
        paths.push_back(ObjectIdentifier(*this) << ObjectIdentifier::SimpleComponent((*it)->first));
        if ((*it)->second->hasName())
            paths.push_back(ObjectIdentifier(*this) << ObjectIdentifier::SimpleComponent((*it)->second->getText()));
    }
}

} // namespace App
