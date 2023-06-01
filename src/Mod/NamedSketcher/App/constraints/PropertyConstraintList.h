/***************************************************************************
 *   Copyright (c) 2010 Jürgen Riegel <juergen.riegel@web.de>              *
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

#ifndef APP_PropertyConstraintList_H
#define APP_PropertyConstraintList_H

#include <string>
#include <vector>

#include <Base/Accessor/NameAndTag.h>
#include <App/PropertyTaggedList.h>

#include "ConstraintBase.h"

namespace Base {
class Writer;
}

namespace NamedSketcher
{

class NamedSketcherExport PropertyConstraintList
        : public Base::Accessor::NameAndTag
        , public App::PropertyTaggedListT<ConstraintBase>
{
    TYPESYSTEM_HEADER_WITH_OVERRIDE();

public:
    using App::PropertyTaggedListT<ConstraintBase>::PropertyTaggedListT;

    const char* getEditorName() const override {
        return "NamedSketcherGui::PropertyConstraintListItem";
    }


    /*
     * PropertyTaggedList xml element name
     */
    const char* xmlTagName() const override {return xmlTagNameStatic();}
    static const char* xmlTagNameStatic() {return "ConstraintList";}

    /*
     * Property copy-paste.
     */
    Property *Copy() const override;
    void Paste(const App::Property &from) override;

    /*
     * Python.
     */
    PyObject* getPyObject() override;
    void setPyObject(PyObject*) override;
};

} // namespace NamedSketcher

#endif // APP_PropertyConstraintList_H
