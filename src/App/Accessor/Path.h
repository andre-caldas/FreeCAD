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

#ifndef APP_Accessor_Path_H
#define APP_Accessor_Path_H

#include <memory>
#include <vector>
#include <string>

#include <FCGlobal.h>

namespace App {
class PropertyContainer;
}

namespace App::Accessor
{

class PathComponent;
class PathIterator;

/*!
 * \brief Class that represents one path to an object / variable.
 *
 * A path is a sequence of objects that can be "followed through"
 * in order to arrive at another object / variable.
 */
class AppExport Path
{
public:
    Path(Accessor&& root, const std::string& path_string);
    Path(PropertyContainer* root_container, const std::string& path_string); // Deprecated.

    PathIterator begin() const;
    PathIterator end() const;

private:
    /**
     * @brief Temporary hack. While the document structure does not use shared_ptr,
     * we create a shared_ptr internally that does not delete anything.
     * \attention When Path is constructed we need to construct _owner passing
     * a dummy custom deleter.
     */
    std::shared_ptr<App::PropertyContainer> _root; // Fake shared ptr.
    std::weak_ptr<App::PropertyContainer> root;
    std::vector<std::unique_ptr<PathComponent>> components;
    std::string path_string;
    friend class PathIterator;
};

} //namespace App::Accessor


#endif // APP_Accessor_Path_H
