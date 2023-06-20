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


#ifndef BASE_ElementFactory_H
#define BASE_ElementFactory_H

#include <functional>
#include <memory>
#include <list>
#include <map>
#include <string>

#include <Base/Exception.h>
#include <Base/Reader.h>

namespace Base
{

template<typename BaseClass>
class ElementFactory
{
    using pointer_type = std::unique_ptr<BaseClass>;
    using map_type = std::map<std::string, std::function<pointer_type(Base::XMLReader&)>>;

public:
    pointer_type produceFromXml(Base::XMLReader& reader);

protected:
    virtual void getAttributes(Base::XMLReader& reader) = 0;
    virtual void setAttributes(BaseClass* p) = 0;

private:
    static map_type factoryMap;
};

template<typename BaseClass>
typename ElementFactory<BaseClass>::pointer_type
ElementFactory<BaseClass>::produceFromXml(Base::XMLReader& reader)
{
    if(!reader.testElementConsume(BaseClass::xmlTagNameStatic()))
    {
        FC_THROWM(Base::RuntimeError, "Wrong tag name '" << reader.localName() << "'. Expected: '" << BaseClass::xmlTagNameStatic() << "'.");
    }

    std::string_view type = reader.getAttribute("type");
    getAttributes(reader);

    // TODO: C++20 use "contains".
    if(!factoryMap.count(type))
    {
        FC_THROWM(Base::NotImplementedError, "Type '" << type << "' not supported by NamedSketcher, yet!");
    }

    auto producer = factoryMap.at(type);
    std::unique_ptr<BaseClass> result(producer(reader));
    setAttributes(result.get());
    return result;
}

} // namespace Base

#endif // BASE_ElementFactory_H
