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


#ifndef APP_Path_DocumentMapper_H
#define APP_Path_DocumentMapper_H

#include <map>
#include <string>

#include <FCConfig.h>


namespace App::ObjectPath
{

class AppExport DocumentMapper {
    using map_type = std::map<std::string,std::string>;

public:
    explicit DocumentMapper(const map_type &);
    ~DocumentMapper();

    static bool hasMap ();
    static map_type::const_iterator find(const std::string& name);
    static map_type::const_iterator end();

private:
    static const map_type *_DocumentMap;
};

} // namespace App::ObjectPath

#endif // APP_Path_DocumentMapper_H
