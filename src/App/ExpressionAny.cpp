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

#include <Base/QuantityPy.h>
#include <Base/Interpreter.h>

#include "ExpressionAny.h"

namespace App {
class Expression;
}

using namespace App;
using namespace Base;


#if defined(_MSC_VER)
#define strtoll _strtoi64
#pragma warning(disable : 4003)
#pragma warning(disable : 4065)
#endif

#define __EXPR_THROW(_e,_msg,_expr) do {\
    std::ostringstream ss;\
    ss << _msg << (_expr);\
    throw _e(ss.str().c_str());\
}while(0)

#define _EXPR_THROW(_msg,_expr) __EXPR_THROW(ExpressionError,_msg,_expr)

#define __EXPR_SET_MSG(_e,_msg,_expr) do {\
    std::ostringstream ss;\
    ss << _msg << _e.what() << (_expr);\
    _e.setMessage(ss.str());\
}while(0)

#define _EXPR_RETHROW(_e,_msg,_expr) do {\
    __EXPR_SET_MSG(_e,_msg,_expr);\
    throw;\
}while(0)

#define _EXPR_PY_THROW(_msg,_expr) do {\
    Base::PyException _e;\
    __EXPR_SET_MSG(_e,_msg,_expr);\
    _e.raiseException();\
}while(0)

#define EXPR_PY_THROW(_expr) _EXPR_PY_THROW("", _expr)

#define EXPR_THROW(_msg) _EXPR_THROW(_msg, this)

#define ARGUMENT_THROW(_msg) EXPR_THROW("Invalid number of arguments: " _msg)

#define RUNTIME_THROW(_msg) __EXPR_THROW(Base::RuntimeError, _msg, static_cast<Expression*>(nullptr))

#define TYPE_THROW(_msg) __EXPR_THROW(Base::TypeError, _msg, static_cast<Expression*>(nullptr))

#define PARSER_THROW(_msg) __EXPR_THROW(Base::ParserError, _msg, static_cast<Expression*>(nullptr))

#define PY_THROW(_msg) __EXPR_THROW(Py::RuntimeError, _msg, static_cast<Expression*>(nullptr))


namespace App::ExpressionHelper {

/////////////////////////////////////////////////////////////////////////////////////
// Helper functions

/* The following definitions are from The art of computer programming by Knuth
 * (copied from http://stackoverflow.com/questions/17333/most-effective-way-for-float-and-double-comparison)
 */

template<class T>
static inline bool essentiallyEqual(T a, T b)
{
    static const T _epsilon = std::numeric_limits<T>::epsilon();
    return std::fabs(a - b) <= ( (std::fabs(a) > std::fabs(b) ? std::fabs(b) : std::fabs(a)) * _epsilon);
}

static inline int essentiallyInteger(double a, long &l, int &i) {
    double intpart;
    if (std::modf(a,&intpart) == 0.0) {
        if (intpart<0.0) {
            if (intpart >= INT_MIN) {
                i = static_cast<int>(intpart);
                l = i;
                return 1;
            }
            if (intpart >= LONG_MIN) {
                l = static_cast<long>(intpart);
                return 2;
            }
        }
        else if (intpart <= INT_MAX) {
            i = static_cast<int>(intpart);
            l = i;
            return 1;
        }
        else if (intpart <= static_cast<double>(LONG_MAX)) {
            l = static_cast<int>(intpart);
            return 2;
        }
    }
    return 0;
}

static inline bool essentiallyInteger(double a, long &l) {
    double intpart;
    if (std::modf(a,&intpart) == 0.0) {
        if (intpart<0.0) {
            if (intpart >= LONG_MIN) {
                l = static_cast<long>(intpart);
                return true;
            }
        }
        else if (intpart <= static_cast<double>(LONG_MAX)) {
            l = static_cast<long>(intpart);
            return true;
        }
    }
    return false;
}


////////////////////////////////////////////////////////////////////////////////

// WARNING! The following define enables slightly faster any type comparison which
// is not standard conforming, and may break in some rare cases (although not likely)
//
// #define USE_FAST_ANY

static inline bool is_type(const App::any &value, const std::type_info& t) {
#ifdef USE_FAST_ANY
    return &value.type() == &t;
#else
    return value.type() == t;
#endif
}

template<class T>
static inline const T &cast(const App::any &value) {
#ifdef USE_FAST_ANY
    return *value.cast<T>();
#else
    return App::any_cast<const T&>(value);
#endif
}

template<class T>
static inline T &cast(App::any &value) {
#ifdef USE_FAST_ANY
    return *value.cast<T>();
#else
    return App::any_cast<T&>(value);
#endif
}

template<class T>
static inline T &&cast(App::any &&value) {
#ifdef USE_FAST_ANY
    return std::move(*value.cast<T>());
#else
    return App::any_cast<T&&>(std::move(value));
#endif
}


// This class is intended to be contained inside App::any (via a shared_ptr)
// without holding Python global lock
struct PyObjectWrapper {
public:
    using Pointer = std::shared_ptr<PyObjectWrapper>;

    explicit PyObjectWrapper(PyObject *obj):pyobj(obj) {
        Py::_XINCREF(pyobj);
    }
    ~PyObjectWrapper() {
        if(pyobj) {
            Base::PyGILStateLocker lock;
            Py::_XDECREF(pyobj);
        }
    }
    PyObjectWrapper(const PyObjectWrapper &) = delete;
    PyObjectWrapper &operator=(const PyObjectWrapper &) = delete;

    Py::Object get() const {
        if(!pyobj)
            return Py::Object();
        return Py::Object(const_cast<PyObject*>(pyobj));
    }

private:
    PyObject *pyobj;
};

static inline PyObjectWrapper::Pointer pyObjectWrap(PyObject *obj) {
    return std::make_shared<PyObjectWrapper>(obj);
}

static inline bool isAnyPyObject(const App::any &value) {
    return is_type(value,typeid(PyObjectWrapper::Pointer));
}

static inline Py::Object __pyObjectFromAny(const App::any &value) {
    return cast<PyObjectWrapper::Pointer>(value)->get();
}

static Py::Object _pyObjectFromAny(const App::any &value, const Expression *e) {
    if(value.empty())
        return Py::Object();
    else if (isAnyPyObject(value))
        return __pyObjectFromAny(value);
    if (is_type(value,typeid(Quantity)))
        return Py::asObject(new QuantityPy(new Quantity(cast<Quantity>(value))));
    else if (is_type(value,typeid(double)))
        return Py::Float(cast<double>(value));
    else if (is_type(value,typeid(float)))
        return Py::Float(cast<float>(value));
    else if (is_type(value,typeid(int)))
        return Py::Long(cast<int>(value));
    else if (is_type(value,typeid(long))) {
        return Py::Long(cast<long>(value));
    } else if (is_type(value,typeid(bool)))
        return Py::Boolean(cast<bool>(value));
    else if (is_type(value,typeid(std::string)))
        return Py::String(cast<string>(value));
    else if (is_type(value,typeid(const char*)))
        return Py::String(cast<const char*>(value));

    _EXPR_THROW("Unknown type", e);
}

Py::Object pyObjectFromAny(const App::any &value) {
    return _pyObjectFromAny(value,nullptr);
}

App::any pyObjectToAny(Py::Object value, bool check) {

    if(value.isNone())
        return App::any();

    PyObject *pyvalue = value.ptr();

    if(!check)
        return App::any(pyObjectWrap(pyvalue));

    if (PyObject_TypeCheck(pyvalue, &Base::QuantityPy::Type)) {
        Base::QuantityPy * qp = static_cast<Base::QuantityPy*>(pyvalue);
        Base::Quantity * q = qp->getQuantityPtr();

        return App::any(*q);
    }
    if (PyFloat_Check(pyvalue))
        return App::any(PyFloat_AsDouble(pyvalue));
    if (PyLong_Check(pyvalue))
        return App::any(PyLong_AsLong(pyvalue));
    else if (PyUnicode_Check(pyvalue)) {
        const char* utf8value = PyUnicode_AsUTF8(pyvalue);
        if (!utf8value) {
            FC_THROWM(Base::ValueError, "Invalid unicode string");
        }
        return App::any(std::string(utf8value));
    }
    else {
        return App::any(pyObjectWrap(pyvalue));
    }
}

bool pyToQuantity(Quantity &q, const Py::Object &pyobj) {
    if (PyObject_TypeCheck(*pyobj, &Base::QuantityPy::Type))
        q = *static_cast<Base::QuantityPy*>(*pyobj)->getQuantityPtr();
    else if (PyFloat_Check(*pyobj))
        q = Quantity(PyFloat_AsDouble(*pyobj));
    else if (PyLong_Check(*pyobj))
        q = Quantity(PyLong_AsLong(*pyobj));
    else
        return false;
    return true;
}

Quantity pyToQuantity(const Py::Object &pyobj, const Expression *e, const char *msg)
{
    Quantity q;
    if(!pyToQuantity(q,pyobj)) {
        if(!msg)
            msg = "Failed to convert to Quantity.";
        __EXPR_THROW(TypeError,msg,e);
    }
    return q;
}

Py::Object pyFromQuantity(const Quantity &quantity) {
    if(!quantity.getUnit().isEmpty())
        return Py::asObject(new QuantityPy(new Quantity(quantity)));
    double v = quantity.getValue();
    long l;
    int i;
    switch(essentiallyInteger(v,l,i)) {
    case 1:
    case 2:
        return Py::Long(l);
    default:
        return Py::Float(v);
    }
}

Quantity anyToQuantity(const App::any &value, const char *msg) {
    if (is_type(value,typeid(Quantity))) {
        return cast<Quantity>(value);
    } else if (is_type(value,typeid(bool))) {
        return Quantity(cast<bool>(value)?1.0:0.0);
    } else if (is_type(value,typeid(int))) {
        return Quantity(cast<int>(value));
    } else if (is_type(value,typeid(long))) {
        return Quantity(cast<long>(value));
    } else if (is_type(value,typeid(float))) {
        return Quantity(cast<float>(value));
    } else if (is_type(value,typeid(double))) {
        return Quantity(cast<double>(value));
    }
    if(!msg)
        msg = "Failed to convert to Quantity";
    TYPE_THROW(msg);
}

static inline bool anyToLong(long &res, const App::any &value) {
    if (is_type(value,typeid(int))) {
        res = cast<int>(value);
    } else if (is_type(value,typeid(long))) {
        res = cast<long>(value);
    } else if (is_type(value,typeid(bool)))
        res = cast<bool>(value)?1:0;
    else
        return false;
    return true;
}

static inline bool anyToDouble(double &res, const App::any &value) {
    if (is_type(value,typeid(double)))
        res = cast<double>(value);
    else if (is_type(value,typeid(float)))
        res = cast<float>(value);
    else if (is_type(value,typeid(long)))
        res = cast<long>(value);
    else if (is_type(value,typeid(int)))
        res = cast<int>(value);
    else if (is_type(value,typeid(bool)))
        res = cast<bool>(value)?1:0;
    else
        return false;
    return true;
}

bool isAnyEqual(const App::any &v1, const App::any &v2) {
    if(v1.empty())
        return v2.empty();
    else if(v2.empty())
        return false;

    if(!is_type(v1,v2.type())) {
        if(is_type(v1,typeid(Quantity)))
            return cast<Quantity>(v1) == anyToQuantity(v2);
        else if(is_type(v2,typeid(Quantity)))
            return anyToQuantity(v1) == cast<Quantity>(v2);

        long l1,l2;
        double d1,d2;
        if(anyToLong(l1,v1)) {
            if(anyToLong(l2,v2))
                return l1==l2;
            else if(anyToDouble(d2,v2))
                return essentiallyEqual((double)l1,d2);
            else
                return false;
        }else if(anyToDouble(d1,v1))
           return anyToDouble(d2,v2) && essentiallyEqual(d1,d2);

        if(is_type(v1,typeid(std::string))) {
            if(is_type(v2,typeid(const char*))) {
                auto c = cast<const char*>(v2);
                return c && cast<std::string>(v1)==c;
            }
            return false;
        }else if(is_type(v1,typeid(const char*))) {
            if(is_type(v2,typeid(std::string))) {
                auto c = cast<const char*>(v1);
                return c && cast<std::string>(v2)==c;
            }
            return false;
        }
    }

    if (is_type(v1,typeid(int)))
        return cast<int>(v1) == cast<int>(v2);
    if (is_type(v1,typeid(long)))
        return cast<long>(v1) == cast<long>(v2);
    if (is_type(v1,typeid(std::string)))
        return cast<std::string>(v1) == cast<std::string>(v2);
    if (is_type(v1,typeid(const char*))) {
        auto c1 = cast<const char*>(v1);
        auto c2 = cast<const char*>(v2);
        return c1==c2 || (c1 && c2 && strcmp(c1,c2)==0);
    }
    if (is_type(v1,typeid(bool)))
        return cast<bool>(v1) == cast<bool>(v2);
    if (is_type(v1,typeid(double)))
        return essentiallyEqual(cast<double>(v1), cast<double>(v2));
    if (is_type(v1,typeid(float)))
        return essentiallyEqual(cast<float>(v1), cast<float>(v2));

    if (is_type(v1,typeid(Quantity)))
        return cast<Quantity>(v1) == cast<Quantity>(v2);

    if (!isAnyPyObject(v1))
        throw Base::TypeError("Unknown type");

    Base::PyGILStateLocker lock;
    Py::Object o1 = __pyObjectFromAny(v1);
    Py::Object o2 = __pyObjectFromAny(v2);
    if(!o1.isType(o2.type()))
        return false;
    int res = PyObject_RichCompareBool(o1.ptr(),o2.ptr(),Py_EQ);
    if(res<0)
        PyException::ThrowException();
    return !!res;
}

} // namespace App::ExpressionHelper
