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

#include <cassert>
#include <memory>

#include <Base/Exception.h>

#include "Accessor.h"
#include "AccessorProperty.h"
#include "PathIterator.h"
#include "StringParser.h"
#include "Path.h"

namespace App::Accessor
{

Path::Path(Accessor&& root, const std::string& path_string)
    : root(root)
    , path_string(path_string)
{
    // split.
    // instantiate component vector.
}

Path::Path(ChainSolver* _root, const std::string& path_string)
    : _root(_root, [](auto /*p*/){}) // Fake shared_ptr.
    , Path(_root, path_string)
{
}

PathIterator Path::begin() const
{
    return PathIterator(*this);
}

PathIterator Path::end() const
{
    return PathIterator(*this, true);
}


PathIterator::PathIterator(const Path& path, bool end)
    :path(path)
{
    if(!end)
    {
        if(!path.components.empty())
        {
            iterator_stack.push(path.components.front()->begin());
        }
    }
}

PathIterator& PathIterator::operator++()
{
    if(iterator_stack.empty())
    {
        return *this;
    }

    const auto stack_size = iterator_stack.size();
    ++(iterator_stack.top());
    if(iterator_stack.top() == path.components.at(stack_size-1).end())
    {
        iterator_stack.pop();
        return ++(*this);
    }

    completeIteratorChain();
    return *this;
}

PathIterator PathIterator::operator++(int)
{
    PathIterator copy = *this;
    ++(*this);
    return copy;
}

bool PathIterator::operator==(const PathIterator& other) const
{
    // In C++20 we could simply iterator_stack == other.iterator_stack.
    if(iterator_stack.size() != other.iterator_stack.size())
    {
        return false;
    }

    for(int i=0; i < iterator_stack.size(); ++i)
    {
        if(iterator_stack.at(i) != other.iterator_stack.at(i))
        {
            return false;
        }
    }

    return true;
}

Accessor& PathIterator::operator*() const
{
    if(iterator_stack.empty())
    {
        throw FC_THROWM(Base::ReferenceError, "Access itterator out of bounds.");
    }

    return *(iterator_stack.top())
}

Accessor* operator->() const
{
    return &*(*this);
}

void PathIterator::completeIteratorChain()
{
    if(path.components.empty())
    {
        return;
    }

    if(iterator_stack.empty())
    {
        auto root_object = root.lock();
        if(!shared_ptr)
        {
            FC_THROWM(Base::RuntimeError, "The path root object does not exist (anymore)");
        }

        const std::string& name = path.components.first()->getName();
        auto iterator = components.at(0).begin(root_object.getAccessor(name));
        iterator_stack.push(iterator);
    }

    assert(iterator_stack.size() > 0);
    for(auto i = iterator_stack.size(); i < path.components.size(); ++i)
    {
        try
        {
            const std::string& name = path.components.at(i)->getName();
            Accessor next = iterator_stack.top()->solve(name);
            auto iterator = path.components.at(i).begin(&next);
            iterator_stack.push(iterator);
        } catch (std::bad_cast&) {
            FC_THROWM(Base::ReferenceError, "Non terminal Accessor is not subclass of ChainSolver.");
        }
    }

    assert(iterator_stack.size() == path.components.size());
}

} // namespace App::Accessor
