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
#include <algorithm>
#include <memory>
#include <string>

#include <App/ExpressionParser.h>

#include "PathSimpleComponent.h"
#include "PathArrayComponent.h"
#include "PathMapComponent.h"

#include "StringParser.h"

namespace App::Accessor
{

bool StringParser::parse(bool shall_throw)
{
    clear_error();
    components.reset();

    pos = str.begin();
    while(pos != end && !has_error)
    {
        parse_component();
    }
    if(shall_throw)
    {
        throwIfHasError();
    }
    return has_error;
}

void StringParser::parse_component()
{
    static const std::string separator(".[{");
    auto component_name = open_close_consumer(separator, true);
    switch(*pos)
    {
    case '[':
        parse_array_range(std::move(component_name));
        break;
    case '{':
        parse_map_list(std::move(component_name));
        break;
    case '.':
        ++pos;
        [[fallthrough]];
    default:
        components.push_back(std::make_unique<PathSimpleComponent>(std::move(component_name)));
        break;
    }
}

void StringParser::parse_array_range(std::string&& name)
{
    assert(*pos == '[');
    ++pos;

    std::vector<std::unique_ptr<NumberExpression>> expressions;
    expressions.reserve(3);

    do
    {
        expressions.push_back(parse_number_expression(expressions));
        if(expressions.size() > 3)
        {
            set_error();
            return;
        }
    } while(pos != str.end() && *pos == ':');
    if(pos == str.end() || *pos != ']')
    {
        set_error();
        return;
    }
    if(pos != str.end())
    {
        ++pos; // ']'.
    }

    while(expressions.size() < 3)
    {
        expressions.emplace_back(nullptr);
    }

    assert(expressions.size() == 3);
    std::unique_ptr<NumberExpression>&& begin = std::move(expressions[0]);
    std::unique_ptr<NumberExpression>&& end = std::move(expressions[1]);
    std::unique_ptr<NumberExpression>&& step = std::move(expressions[2]);
    auto component = std::make_unique<PathArrayComponent>(name, begin, end, step);
    components.push_back(std::move(component));
}

void StringParser::parse_number_expression(std::vector<std::unique_ptr<NumberExpression>>& expressions)
{
    if(if pos = str.end() || *pos == ']' || *pos == ':')
    {
        expressions.emplace_back(nullptr);
        return;
    }

auto expression = open_close_consumer(":]");
//    auto expression = std::make_unique<NumberExpression>();
//    expression->parse(number_expression);
    expressions.push_back(std::move(expression));
}

//xxx parse_list(std::string::iterator begin, std::string::iterator end);
//xxx parse_expression(std::string::iterator begin, std::string::iterator end);

std::string_view StringParser::open_close_consumer(const std::string& ending_chars, bool allow_end_of_string)
{
    static const std::string opening_chars("[({\"");
    static const std::string closing_chars("])}\"");
    static const std::string special_chars("\\" "[({" "])}" "\"");

    const std::string all_symbols = special_chars + ending_chars;
    std::stack<char> close_stack;
    auto begin = pos;

    // implement open_close_consumer...
    for(; pos != str.end(); ++pos)
    {
        auto found_pos = str.find_first_of(all_symbols, pos);
        if(found_pos == std::npos)
        {
            break;
        }
        pos = found_pos;
        if(*pos == '\\')
        {
            ++pos;
            if(pos == str.end())
            {
                set_error();
                return str.string_view(begin, pos);
            }
            continue;
        }

        // End of string with no pending opening symbols.
        if(close_stack.empty() && ending_chars.find(*pos) != std::string::npos)
        {
            if(allow_end_of_string)
            {
                break;
            }
            set_error();
            return str.string_view(begin, pos);
        }

        // Since "quoting" uses the same opening and closing symbols,
        // it needs a special treatment.
        if(!close_stack.empty() && close_stack.top() == '\"')
        {
            // Inside "quoting )", we ignore delimiters different from '\"'.
            if(*pos == '\"')
            {
                close_stack.pop();
            }
            continue;
        }
        // From now on, we are NOT dealing with a closing '\"'.

        auto open = opening_chars.find(*pos);
        if(open != std::string::npos)
        {
            close_stack.push(closing_chars[open]);
            continue;
        }

        auto close = closing_chars.find(*pos);
        if(close != std::string::npos)
        {
            if(close_stack.empty())
            {
                set_error(/* nothing opened */);
                return str.string_view(begin, pos);
            }
            if(*pos != close_stack.top())
            {
                set_error(/* wrong closing */);
                return str.string_view(begin, pos);
            }
            close_stack.pop();
            continue;
        }
    }

    if(!close_stack.empty())
    {
        set_error();
    }
    return str.string_view(begin, pos);
}

} //namespace App::Accessor
