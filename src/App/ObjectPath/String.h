/***************************************************************************
 *   Copyright (c) 2015 Eivind Kvedalen <eivind@kvedalen.name>             *
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


#ifndef APP_ObjectPath_String_H
#define APP_ObjectPath_String_H

#include <string>
#include <FCConfig.h>

namespace App {
class ObjectIdentifier;
class DocumentObject;
}

namespace App::ObjectPath {

AppExport std::string quote(const std::string &input, bool toPython=false);

class String {
    friend class App::ObjectIdentifier;

public:

    String(const std::string &s = "",
           bool _isRealString = false,
           bool _forceIdentifier = false)
        : str(s)
        , isString(_isRealString)
        , forceIdentifier(_forceIdentifier)
    {}//explicit bombs

    explicit String(std::string &&s,
                    bool _isRealString = false,
                    bool _forceIdentifier = false)
        : str(std::move(s))
        , isString(_isRealString)
        , forceIdentifier(_forceIdentifier)
    {}

    // Accessors

    /** Returns the string */
    const std::string &getString() const { return str; }

    /** Return true is string need to be quoted */
    bool isRealString() const { return isString; }

    bool isForceIdentifier() const { return forceIdentifier; }

    bool empty() const { return str.empty(); }

    /** Returns a possibly quoted string */
    std::string toString(bool toPython=false) const;

    // Operators

    explicit operator std::string() const { return str; }

//    explicit operator const char *() const { return str.c_str(); }

    bool operator==(const String & other) const { return str == other.str; }

    bool operator!=(const String & other) const { return str != other.str; }

    bool operator>=(const String & other) const { return str >= other.str; }

    bool operator<(const String & other) const { return str < other.str; }

    bool operator>(const String & other) const { return str > other.str; }

    void checkImport(const App::DocumentObject *owner,
                     const App::DocumentObject *obj=nullptr, String *objName=nullptr);

private:
    std::string str;
    bool isString;
    bool forceIdentifier;
};

} // namespace App::Path

#endif // APP_ObjectPath_String_H
