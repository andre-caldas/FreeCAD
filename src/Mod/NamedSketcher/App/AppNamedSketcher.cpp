/***************************************************************************
 *   Copyright (c) 2008 Jürgen Riegel <juergen.riegel@web.de>              *
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

#include <Base/Console.h>
#include <Base/Interpreter.h>

//#include "Constraint.h"
//#include "ConstraintPy.h"
//#include "Geometry.h"
//#include "GeometryPy.h"
#include "NamedSketch.h"
#include "NamedSketchPy.h"


namespace NamedSketcher {
extern PyObject* initModule();
}

/* Python entry */
PyMOD_INIT_FUNC(NamedSketcher)
{
    // load dependent module
    try {
        Base::Interpreter().runString("import Part");
    }
    catch(const Base::Exception& e) {
        PyErr_SetString(PyExc_ImportError, e.what());
        PyMOD_Return(nullptr);
    }

    PyObject* namedSketcherModule = NamedSketcher::initModule();

    // Add Types to module
//    Base::Interpreter().addType(&Sketcher::ConstraintPy   ::Type,namedSketcherModule,"Constraint");
//    Base::Interpreter().addType(&Sketcher::GeometryPy     ::Type,namedSketcherModule,"Geometry");
    Base::Interpreter().addType(&NamedSketcher::NamedSketchPy  ::Type,namedSketcherModule,"NamedSketch");


    // NOTE: To finish the initialization of our own type objects we must
    // call PyType_Ready, otherwise we run into a segmentation fault, later on.
    // This function is responsible for adding inherited slots from a type's base class.

//    Sketcher::Constraint   ::init();
//    Sketcher::Geometry     ::init();
//    Sketcher::NamedSketch  ::init();

    Base::Console().Log("Loading NamedSketcher module... done\n");

    PyMOD_Return(namedSketcherModule);
}
