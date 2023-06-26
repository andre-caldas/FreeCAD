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

#ifndef APP_Accessor_StringParser_H
#define APP_Accessor_StrinParser_H

#include <memory>
#include <vector>
#include <string>

namespace App {
//class NumberExpression;
//class StringExpression;
}

namespace App::Accessor
{

// Temporary while I do not know how to use NumberExpression.
using NumberExpression = std::string;
using StringExpression = std::string;
class PathComponent;

class AppExport StringParser
{
public:
    using string_iterator = std::string::const_iterator;

    StringParser(std::string&& str) : str(str) {}
    StringParser(const::string& str) : StringParser(std::move(str)) {}

    bool parse(bool shall_throw = false);

    /**
     * @brief Parses one component from a string
     * and appends it to the list of components.
     * @param begin String iterator positioned at the parsing point.
     * It is updated to point to \a end when this was the last component
     * or the next position after the "." that separates the components.
     */
    void parse_component();
    void parse_array_range(std::string&& name);
    void parse_map_list(std::string&& name);

private:
    std::vector<unique_ptr<PathComponent>> components;

    std::string str;
    string_iterator pos;

    bool has_error = false;
    string_iterator error_position;
    std::unique_ptr<Exception> exception;

    void set_error(...);
    void clear_error() {has_error = false; exception.release();}

    void parse_number_expression(std::vector<std::unique_ptr<NumberExpression>>& expressions);
    void parse_string_expression(std::vector<std::unique_ptr<StringExpression>>& expressions);
    std::string_view open_close_consumer(const std::string& ending_chars, bool allow_end_of_string = false);

    void throwIfHasError();
};

} //namespace App::Accessor


#endif // APP_Accessor_StringParser_H
