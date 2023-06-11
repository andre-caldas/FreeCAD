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

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/string_generator.hpp>

#include <Base/Console.h>

#include <Base/Reader.h>
#include <Base/Writer.h>
#include <Base/Tools.h>

#include <App/ObjectIdentifier.h>
#include <App/ExpressionParser.h>

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
unsigned int PropertyTaggedListT<T>::getMemSize() const
{
    int size = sizeof(T);
    for (auto& item: _lValueList)
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
    for(auto& item: _lValueList)
    {
        item.second->Save(writer);
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
        addValue(typename T::factory(reader));
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

template<typename T>
typename PropertyTaggedListT<T>::iterator PropertyTaggedListT<T>::begin() const
{
    return iterator(_lValueList.cbegin());
}

template<typename T>
typename PropertyTaggedListT<T>::iterator PropertyTaggedListT<T>::end() const
{
    return iterator(_lValueList.cend());
}

} // namespace App

#undef FC_LOG_INSTANCE
#ifdef OLD_FC_LOG_INSTANCE
#define FC_LOG_INSTANCE OLD_FC_LOG_INSTANCE
#undef OLD_FC_LOG_INSTANCE
#endif
