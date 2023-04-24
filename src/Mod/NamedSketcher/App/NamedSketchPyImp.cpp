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

# include <memory>
# include <sstream>

#include <App/Document.h>

#include "NamedSketchPy.h"
#include "NamedSketchPy.cpp"


using namespace Sketcher;

// returns a string which represents the object e.g. when printed in python
std::string NamedSketchPy::representation() const
{
    return "<NamedSketcher::NamedSketch>";
}

PyObject* NamedSketchPy::solve(PyObject *args)
{
    if (!PyArg_ParseTuple(args, ""))
        return nullptr;
    int ret = this->getNamedSketchPtr()->solve();
    return Py_BuildValue("i", ret);
}

PyObject* NamedSketchPy::addGeometry(PyObject *args)
{
    PyObject *pcObj;
    if (!PyArg_ParseTuple(args, "O", &pcObj))
        return nullptr;

    if (PyObject_TypeCheck(pcObj, &(Part::GeometryPy::Type))) {
        Part::Geometry *geo = static_cast<Part::GeometryPy*>(pcObj)->getGeometryPtr();
        auto& ret = this->getNamedSketchPtr()->addGeometry(*geo);
        // TODO: How to return NamedSketcher::GeometryBase?
        return Py::new_reference_to(Py::Long(ret));
    }

    std::string error = std::string("type must be a subclass of 'Part::Geometry', not ");
    error += pcObj->ob_type->tp_name;
    throw Py::TypeError(error);
}

PyObject* NamedSketchPy::delGeometry(PyObject *args)
{
    const char* tag;
    if (!PyArg_ParseTuple(args, "S", &tag)) {
        if (.... GeometryBase) {
            tag = xxxxx;
        } else {
            return nullptr;
        }
    }

    if (this->getNamedSketchPtr()->delGeometry(tag)) {
        std::stringstream str;
        str << "Not able to delete a geometry with the given tag: " << tag;
        PyErr_SetString(PyExc_ValueError, str.str().c_str());
        return nullptr;
    }

    Py_Return;
}


PyObject* NamedSketchPy::addConstraint(PyObject *args)
{
    PyObject *pcObj;
    if (!PyArg_ParseTuple(args, "O", &pcObj))
        return nullptr;

    if (PyObject_TypeCheck(pcObj, &(Sketcher::ConstraintPy::Type))) {
        Sketcher::Constraint *constr = static_cast<Sketcher::ConstraintPy*>(pcObj)->getConstraintPtr();
        if (!this->getNamedSketchPtr()->evaluateConstraint(constr)) {
            PyErr_SetString(PyExc_IndexError, "Constraint has invalid indexes");
            return nullptr;
        }
        int ret = this->getNamedSketchPtr()->addConstraint(constr);
        // this solve is necessary because:
        // 1. The addition of constraint is part of a command addition
        // 2. This solve happens before the command is committed
        // 3. A constraint, may effect a geometry change (think of coincident,
        // a line's point moves to meet the other line's point
        // 4. The transaction is committed before any other solve, for example
        // the one of execute() triggered by a recompute (UpdateActive) is generated.
        // 5. Upon "undo", the constraint is removed (it was before the command was committed)
        //    however, the geometry changed after the command was committed, so the point that
        //    moved do not go back to the position where it was.
        //
        // N.B.: However, the solve itself may be inhibited in cases where groups of geometry/constraints
        //      are added together, because in that case undoing will also make the geometry disappear.
        this->getNamedSketchPtr()->solve();
        // if the geometry moved during the solve, then the initial solution is invalid
        // at this point, so a point movement may not work in cases where redundant constraints exist.
        // this forces recalculation of the initial solution (not a full solve)
        if(this->getNamedSketchPtr()->noRecomputes) {
            this->getNamedSketchPtr()->setUpSketch();
            this->getNamedSketchPtr()->Constraints.touch(); // update solver information
        }
        return Py::new_reference_to(Py::Long(ret));
    }
    else if (PyObject_TypeCheck(pcObj, &(PyList_Type)) ||
             PyObject_TypeCheck(pcObj, &(PyTuple_Type))) {
        std::vector<Constraint*> values;
        Py::Sequence list(pcObj);
        for (Py::Sequence::iterator it = list.begin(); it != list.end(); ++it) {
            if (PyObject_TypeCheck((*it).ptr(), &(ConstraintPy::Type))) {
                Constraint *con = static_cast<ConstraintPy*>((*it).ptr())->getConstraintPtr();
                values.push_back(con);
            }
        }

        for (std::vector<Constraint*>::iterator it = values.begin(); it != values.end(); ++it) {
            if (!this->getNamedSketchPtr()->evaluateConstraint(*it)) {
                PyErr_SetString(PyExc_IndexError, "Constraint has invalid indexes");
                return nullptr;
            }
        }
        int ret = getNamedSketchPtr()->addConstraints(values) + 1;
        std::size_t numCon = values.size();
        Py::Tuple tuple(numCon);
        for (std::size_t i=0; i<numCon; ++i) {
            int conId = ret - int(numCon - i);
            tuple.setItem(i, Py::Long(conId));
        }
        return Py::new_reference_to(tuple);
    }

    std::string error = std::string("type must be 'Constraint' or list of 'Constraint', not ");
    error += pcObj->ob_type->tp_name;
    throw Py::TypeError(error);
}

PyObject* NamedSketchPy::delConstraint(PyObject *args)
{
    int Index;
    if (!PyArg_ParseTuple(args, "i", &Index))
        return nullptr;

    if (this->getNamedSketchPtr()->delConstraint(Index)) {
        std::stringstream str;
        str << "Not able to delete a constraint with the given index: " << Index;
        PyErr_SetString(PyExc_ValueError, str.str().c_str());
        return nullptr;
    }

    Py_Return;
}


PyObject* NamedSketchPy::addExternalSketch(PyObject *args)
{
    char *ObjectName;
    char *SubName;
    if (!PyArg_ParseTuple(args, "ss:Give an object and subelement name", &ObjectName,&SubName))
        return nullptr;

    // get the target object for the external link
    Sketcher::NamedSketch* skObj = this->getNamedSketchPtr();
    App::DocumentObject * Obj = skObj->getDocument()->getObject(ObjectName);
    if (!Obj) {
        std::stringstream str;
        str << ObjectName << " does not exist in the document";
        PyErr_SetString(PyExc_ValueError, str.str().c_str());
        return nullptr;
    }
    // check if this type of external geometry is allowed
    if (!skObj->isExternalAllowed(Obj->getDocument(), Obj)) {
        std::stringstream str;
        str << ObjectName << " is not allowed as external geometry of this sketch";
        PyErr_SetString(PyExc_ValueError, str.str().c_str());
        return nullptr;
    }

    // add the external
    if (skObj->addExternal(Obj,SubName) < 0) {
        std::stringstream str;
        str << "Not able to add external shape element";
        PyErr_SetString(PyExc_ValueError, str.str().c_str());
        return nullptr;
    }

    Py_Return;
}

PyObject* NamedSketchPy::delExternalSketch(PyObject *args)
{
    int Index;
    if (!PyArg_ParseTuple(args, "i", &Index))
        return nullptr;

    if (this->getNamedSketchPtr()->delExternal(Index)) {
        std::stringstream str;
        str << "Not able to delete an external geometry with the given index: " << Index;
        PyErr_SetString(PyExc_ValueError, str.str().c_str());
        return nullptr;
    }

    Py_Return;
}

PyObject* NamedSketchPy::delConstraintOnPoint(PyObject *args)
{
    int Index, pos=-1;
    if (!PyArg_ParseTuple(args, "i|i", &Index, &pos))
        return nullptr;

    if (pos >= static_cast<int>(Sketcher::PointPos::none) && pos <= static_cast<int>(Sketcher::PointPos::mid)) { // This is the whole range of valid positions
        if (this->getNamedSketchPtr()->delConstraintOnPoint(Index,static_cast<Sketcher::PointPos>(pos))) {
            std::stringstream str;
            str << "Not able to delete a constraint on point with the given index: " << Index
                << " and position: " << pos;
            PyErr_SetString(PyExc_ValueError, str.str().c_str());
            return nullptr;
        }
    }
    else if (pos==-1) {
        if (this->getNamedSketchPtr()->delConstraintOnPoint(Index)) {
            std::stringstream str;
            str << "Not able to delete a constraint on point with the given index: " << Index;
            PyErr_SetString(PyExc_ValueError, str.str().c_str());
            return nullptr;
        }
    }
    else {
        PyErr_SetString(PyExc_ValueError, "Wrong PointPos argument");
        return nullptr;
    }

    Py_Return;
}



PyObject *NamedSketchPy::getCustomAttributes(const char* /*attr*/) const
{
    return nullptr;
}

int NamedSketchPy::setCustomAttributes(const char* attr, PyObject* obj)
{
    // search in PropertyList
    App::Property *prop = getNamedSketchPtr()->getPropertyByName(attr);
    if (prop) {
        // Read-only attributes must not be set over its Python interface
        short Type =  getNamedSketchPtr()->getPropertyType(prop);
        if (Type & App::Prop_ReadOnly) {
            std::stringstream s;
            s << "Object attribute '" << attr << "' is read-only";
            throw Py::AttributeError(s.str());
        }

        prop->setPyObject(obj);

        return 1;
    }

    return 0;
}
