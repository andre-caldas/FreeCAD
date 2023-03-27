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

#ifndef EXPRESSION_Any_H
#define EXPRESSION_Any_H

// This include is just for App::any.
// Maybe we should have smaller .h files.
// Maybe we could have <App/ObjectPath/Defs.h>
#include <App/ObjectPath/ObjectIdentifier.h>
#include <FCConfig.h>


namespace Base {
class Quantity;
}
namespace App {
class Expression;
}

namespace App::ExpressionHelper {

AppExport bool isAnyEqual(const App::any &v1, const App::any &v2);
AppExport Base::Quantity anyToQuantity(const App::any &value, const char *errmsg = nullptr);
AppExport Base::Quantity pyToQuantity(const Py::Object &pyobj, const Expression *e, const char *msg=nullptr);
AppExport bool pyToQuantity(Base::Quantity &q, const Py::Object &pyobj);
AppExport Py::Object pyFromQuantity(const Base::Quantity &quantity);

}

#endif // EXPRESSION_Any_H
