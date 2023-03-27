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

#include "PreCompiled.h"
#ifdef __GNUC__
# include <unistd.h>
#endif

#if defined(__clang__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wdelete-non-virtual-dtor"
#endif

#include <boost/algorithm/string/predicate.hpp>
#include <boost/math/special_functions/round.hpp>
#include <boost/math/special_functions/trunc.hpp>

#include <algorithm>
#include <sstream>
#include <stack>
#include <string>

#include <App/Application.h>
#include <App/DocumentObject.h>
#include <App/ObjectPath/ObjectIdentifier.h>
#include <App/PropertyUnits.h>
#include <Base/Interpreter.h>
#include <Base/MatrixPy.h>
#include <Base/PlacementPy.h>
#include <Base/QuantityPy.h>
#include <Base/RotationPy.h>
#include <Base/VectorPy.h>

#include "ExpressionParser.h"
#include "ExpressionAny.h"


/** \defgroup Expression Expressions framework
    \ingroup APP
    \brief The expression system allows users to write expressions and formulas that produce values
*/

using namespace Base;
using namespace App;
using namespace App::ExpressionHelper;

FC_LOG_LEVEL_INIT("Expression", true, true)

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif
#ifndef M_E
#define M_E        2.71828182845904523536
#endif
#ifndef  DOUBLE_MAX
# define DOUBLE_MAX 1.7976931348623157E+308    /* max decimal value of a "double"*/
#endif
#ifndef  DOUBLE_MIN
# define DOUBLE_MIN 2.2250738585072014E-308    /* min decimal value of a "double"*/
#endif

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

#if 0
template<class T>
inline bool essentiallyZero(T a) {
    return !a;
}

template<>
inline bool essentiallyZero(double a) {
    return essentiallyEqual(a, 0.0);
}

template<>
inline bool essentiallyZero(float a) {
    return essentiallyEqual(a, 0.0f);
}

template<class T>
static inline bool definitelyGreaterThan(T a, T b)
{
    static const T _epsilon = std::numeric_limits<T>::epsilon();
    return (a - b) > ( (std::fabs(a) < std::fabs(b) ? std::fabs(b) : std::fabs(a)) * _epsilon);
}

template<class T>
static inline bool definitelyLessThan(T a, T b)
{
    static const T _epsilon = std::numeric_limits<T>::epsilon();
    return (b - a) > ( (std::fabs(a) < std::fabs(b) ? std::fabs(b) : std::fabs(a)) * _epsilon);
}
#endif // 0

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


//
// UnitExpression class
//

TYPESYSTEM_SOURCE(App::UnitExpression, App::Expression)

UnitExpression::UnitExpression(const DocumentObject *_owner, const Base::Quantity & _quantity, const std::string &_unitStr)
    : Expression(_owner)
    , quantity(_quantity)
    , unitStr(_unitStr)
{
}

UnitExpression::~UnitExpression() {
    if(cache) {
        Base::PyGILStateLocker lock;
        Py::_XDECREF(cache);
    }
}

void UnitExpression::setQuantity(const Quantity &_quantity)
{
    quantity = _quantity;
    if(cache) {
        Base::PyGILStateLocker lock;
        Py::_XDECREF(cache);
        cache = nullptr;
    }
}

/**
  * Set unit information.
  *
  * @param _unit    A unit object
  * @param _unitstr The unit expressed as a string
  * @param _scaler  Scale factor to convert unit into internal unit.
  */

void UnitExpression::setUnit(const Quantity &_quantity)
{
    quantity = _quantity;
    if(cache) {
        Base::PyGILStateLocker lock;
        Py::_XDECREF(cache);
        cache = nullptr;
    }
}

/**
  * Simplify the expression. In this case, a NumberExpression is returned,
  * as it cannot be simplified any more.
  */

std::unique_ptr<App::Expression> UnitExpression::simplify() const
{
    return std::make_unique<NumberExpression>(owner, quantity);
}

/**
  * Return a string representation, in this case the unit string.
  */

/**
  * Return a string representation of the expression.
  */

void UnitExpression::_toString(std::ostream &ss, bool,int) const
{
    ss << unitStr;
}

/**
  * Return a copy of the expression.
  */

std::unique_ptr<Expression> UnitExpression::_copy() const
{
    return std::make_unique<UnitExpression>(owner, quantity, unitStr);
}

Py::Object UnitExpression::_getPyValue() const {
    if(!cache)
        cache = Py::new_reference_to(pyFromQuantity(quantity));
    return Py::Object(cache);
}

//
// NumberExpression class
//

TYPESYSTEM_SOURCE(App::NumberExpression, App::Expression)

NumberExpression::NumberExpression(const DocumentObject *_owner, const Quantity &_quantity)
    : UnitExpression(_owner, _quantity)
{
}

/**
  * Simplify the expression. For NumberExpressions, we return a copy(), as it cannot
  * be simplified any more.
  */

std::unique_ptr<Expression> NumberExpression::simplify() const
{
    return copy();
}

/**
  * Create and return a copy of the expression.
  */

std::unique_ptr<Expression> NumberExpression::_copy() const
{
    return std::make_unique<NumberExpression>(owner, getQuantity());
}

/**
  * Negate the stored value.
  */

void NumberExpression::negate()
{
    setQuantity(-getQuantity());
}

void NumberExpression::_toString(std::ostream &ss, bool,int) const
{
    // Restore the old implementation because using digits10 + 2 causes
    // undesired side-effects:
    // https://forum.freecadweb.org/viewtopic.php?f=3&t=44057&p=375882#p375882
    // See also:
    // https://en.cppreference.com/w/cpp/types/numeric_limits/digits10
    // https://en.cppreference.com/w/cpp/types/numeric_limits/max_digits10
    // https://www.boost.org/doc/libs/1_63_0/libs/multiprecision/doc/html/boost_multiprecision/tut/limits/constants.html
    boost::io::ios_flags_saver ifs(ss);
    ss << std::setprecision(std::numeric_limits<double>::digits10 + 1) << getValue();

    /* Trim of any extra spaces */
    //while (s.size() > 0 && s[s.size() - 1] == ' ')
//        s.erase(s.size() - 1);
}

bool NumberExpression::isInteger(long *l) const {
    long _l;
    if(!l)
        l = &_l;
    return essentiallyInteger(getValue(),*l);
}

//
// OperatorExpression class
//

TYPESYSTEM_SOURCE(App::OperatorExpression, App::Expression)

OperatorExpression::OperatorExpression(const App::DocumentObject *_owner, Expression * _left, Operator _op, Expression * _right)
    : UnitExpression(_owner)
    , op(_op)
    , left(_left)
    , right(_right)
{

}

/**
  * Determine whether the expression is touched or not, i.e relies on properties that are touched.
  */

bool OperatorExpression::isTouched() const
{
    return left->isTouched() || right->isTouched();
}

static Py::Object calc(const Expression *expr, int op,
                 const Expression *left, const Expression *right, bool inplace)
{
    Py::Object l = left->getPyValue();

    // For security reason, restrict supported types
    if(!PyObject_TypeCheck(l.ptr(),&PyObjectBase::Type)
            && !l.isNumeric() && !l.isString() && !l.isList() && !l.isDict())
    {
        __EXPR_THROW(Base::TypeError,"Unsupported operator", expr);
    }

    // check possible unary operation first
    switch(op) {
    case OperatorExpression::POS:{
        PyObject *res = PyNumber_Positive(l.ptr());
        if(!res) EXPR_PY_THROW(expr);
        return Py::asObject(res);
    }
    case OperatorExpression::NEG:{
        PyObject *res = PyNumber_Negative(l.ptr());
        if(!res) EXPR_PY_THROW(expr);
        return Py::asObject(res);
    } default:
        break;
    }

    Py::Object r = right->getPyValue();
    // For security reason, restrict supported types
    if((op!=OperatorExpression::MOD || !l.isString())
            && !PyObject_TypeCheck(r.ptr(),&PyObjectBase::Type)
                && !r.isNumeric()
                && !r.isString()
                && !r.isList()
                && !r.isDict())
    {
        __EXPR_THROW(Base::TypeError,"Unsupported operator", expr);
    }

    switch(op) {
#define RICH_COMPARE(_op,_pyop) \
    case OperatorExpression::_op: {\
        int res = PyObject_RichCompareBool(l.ptr(),r.ptr(),Py_##_pyop);\
        if(res<0) EXPR_PY_THROW(expr);\
        return Py::Boolean(!!res);\
    }
    RICH_COMPARE(LT,LT)
    RICH_COMPARE(LTE,LE)
    RICH_COMPARE(GT,GT)
    RICH_COMPARE(GTE,GE)
    RICH_COMPARE(EQ,EQ)
    RICH_COMPARE(NEQ,NE)

#define _BINARY_OP(_pyop) \
        res = inplace?PyNumber_InPlace##_pyop(l.ptr(),r.ptr()):\
                       PyNumber_##_pyop(l.ptr(),r.ptr());\
        if(!res) EXPR_PY_THROW(expr);\
        return Py::asObject(res);

#define BINARY_OP(_op,_pyop) \
    case OperatorExpression::_op: {\
        PyObject *res;\
        _BINARY_OP(_pyop);\
    }

    BINARY_OP(SUB,Subtract)
    BINARY_OP(MUL,Multiply)
    BINARY_OP(UNIT,Multiply)
    BINARY_OP(DIV,TrueDivide)
    case OperatorExpression::ADD: {
        PyObject *res;
        if (PyUnicode_CheckExact(*l) && PyUnicode_CheckExact(*r)) {
            if(inplace) {
                res = Py::new_reference_to(l);
                // Try to make sure ob_refcnt is 1, although unlike
                // PyString_Resize above, PyUnicode_Append can handle other
                // cases.
                l = Py::Object();
                PyUnicode_Append(&res, r.ptr());
            }else
                res = PyUnicode_Concat(l.ptr(),r.ptr());
            if(!res) EXPR_PY_THROW(expr);
            return Py::asObject(res);
        }
        _BINARY_OP(Add);
    }
    case OperatorExpression::POW: {
        PyObject *res;
        if(inplace)
            res = PyNumber_InPlacePower(l.ptr(),r.ptr(),Py::None().ptr());
        else
            res = PyNumber_Power(l.ptr(),r.ptr(),Py::None().ptr());
        if(!res) EXPR_PY_THROW(expr);
        return Py::asObject(res);
    }
    case OperatorExpression::MOD: {
        PyObject *res;
        if (PyUnicode_CheckExact(l.ptr()) &&
                (!PyUnicode_Check(r.ptr()) || PyUnicode_CheckExact(r.ptr())))
            res = PyUnicode_Format(l.ptr(), r.ptr());
        else if(inplace)
            res = PyNumber_InPlaceRemainder(l.ptr(),r.ptr());
        else
            res = PyNumber_InPlaceRemainder(l.ptr(),r.ptr());
        if(!res) EXPR_PY_THROW(expr);
        return Py::asObject(res);
    }
    default:
        __EXPR_THROW(RuntimeError,"Unsupported operator",expr);
    }
}

Py::Object OperatorExpression::_getPyValue() const {
    return calc(this,op,left.get(),right.get(),false);
}

/**
  * Simplify the expression. For OperatorExpressions, we return a NumberExpression if
  * both the left and right side can be simplified to NumberExpressions. In this case
  * we can calculate the final value of the expression.
  *
  * @returns Simplified expression.
  */

std::unique_ptr<Expression> OperatorExpression::simplify() const
{
    auto v1 = left->simplify();
    auto v2 = right->simplify();

    // Both arguments reduced to numerics? Then evaluate and return answer
    if (freecad_dynamic_cast<NumberExpression>(v1.get()) && freecad_dynamic_cast<NumberExpression>(v2.get())) {
        return std::unique_ptr<Expression>(eval());
    }

    return std::make_unique<OperatorExpression>(owner, v1.release(), op, v2.release());
}

/**
  * Create a string representation of the expression.
  *
  * @returns A string representing the expression.
  */

void OperatorExpression::_toString(std::ostream &s, bool persistent,int) const
{
    bool needsParens;
    Operator leftOperator(NONE), rightOperator(NONE);

    needsParens = false;
    if (freecad_dynamic_cast<OperatorExpression>(left.get()))
        leftOperator = static_cast<OperatorExpression*>(left.get())->op;
    if (left->priority() < priority()) // Check on operator priority first
        needsParens = true;
    else if (leftOperator == op) { // Same operator ?
        if (!isLeftAssociative())
            needsParens = true;
        //else if (!isCommutative())
        //    needsParens = true;
    }

    switch (op) {
    case NEG:
        s << "-" << (needsParens ? "(" : "") << left->toString(persistent) << (needsParens ? ")" : "");
        return;
    case POS:
        s << "+" << (needsParens ? "(" : "") << left->toString(persistent) << (needsParens ? ")" : "");
        return;
    default:
        break;
    }

    if (needsParens)
        s << "(" << left->toString(persistent) << ")";
    else
        s << left->toString(persistent);

    switch (op) {
    case ADD:
        s << " + ";
        break;
    case SUB:
        s << " - ";
        break;
    case MUL:
        s << " * ";
        break;
    case DIV:
        s << " / ";
        break;
    case MOD:
        s << " % ";
        break;
    case POW:
        s << " ^ ";
        break;
    case EQ:
        s << " == ";
        break;
    case NEQ:
        s << " != ";
        break;
    case LT:
        s << " < ";
        break;
    case GT:
        s << " > ";
        break;
    case LTE:
        s << " <= ";
        break;
    case GTE:
        s << " >= ";
        break;
    case UNIT:
        s << " ";
        break;
    default:
        assert(0);
    }

    needsParens = false;
    if (freecad_dynamic_cast<OperatorExpression>(right.get()))
        rightOperator = static_cast<OperatorExpression*>(right.get())->op;
    if (right->priority() < priority()) // Check on operator priority first
        needsParens = true;
    else if (rightOperator == op) { // Same operator ?
        if (!isRightAssociative())
            needsParens = true;
        else if (!isCommutative())
            needsParens = true;
    }
    else if (right->priority() == priority()) { // Same priority ?
        if (!isRightAssociative() || rightOperator == MOD)
            needsParens = true;
    }

    if (needsParens) {
        s << "(";
        right->toString(s,persistent);
        s << ")";
    }else
        right->toString(s,persistent);
}

/**
  * A deep copy of the expression.
  */

std::unique_ptr<Expression> OperatorExpression::_copy() const
{
    return std::make_unique<OperatorExpression>(owner, left->copy(), op, right->copy());
}

/**
  * Return the operators priority. This is used to add parentheses where
  * needed when creating a string representation of the expression.
  *
  * @returns The operator's priority.
  */

int OperatorExpression::priority() const
{
    switch (op) {
    case EQ:
    case NEQ:
    case LT:
    case GT:
    case LTE:
    case GTE:
        return 1;
    case ADD:
    case SUB:
        return 3;
    case MUL:
    case DIV:
    case MOD:
        return 4;
    case POW:
        return 5;
    case UNIT:
    case NEG:
    case POS:
        return 6;
    default:
        assert(false);
        return 0;
    }
}

void OperatorExpression::_visit(ExpressionVisitor &v)
{
    if (left)
        left->visit(v);
    if (right)
        right->visit(v);
}

bool OperatorExpression::isCommutative() const
{
    switch (op) {
    case EQ:
    case NEQ:
    case ADD:
    case MUL:
        return true;
    default:
        return false;
    }
}

bool OperatorExpression::isLeftAssociative() const
{
    return true;
}

bool OperatorExpression::isRightAssociative() const
{
    switch (op) {
    case ADD:
    case MUL:
        return true;
    default:
        return false;
    }
}

//
// FunctionExpression class. This class handles functions with one or two parameters.
//

TYPESYSTEM_SOURCE(App::FunctionExpression, App::UnitExpression)

static int _HiddenReference;

struct HiddenReference {
    explicit HiddenReference(bool cond)
        :cond(cond)
    {
        if(cond)
            ++_HiddenReference;
    }
    ~HiddenReference() {
        if(cond)
            --_HiddenReference;
    }

    static bool check(int option) {
        return (option==Expression::DepNormal && _HiddenReference)
            || (option==Expression::DepHidden && !_HiddenReference);
    }

    static bool isHidden() {
        return _HiddenReference!=0;
    }

    bool cond;
};

FunctionExpression::FunctionExpression(const DocumentObject *_owner, Function _f, std::string &&name, std::vector<Expression *> _args)
    : UnitExpression(_owner)
    , f(_f)
    , fname(std::move(name))
{
    args.reserve(_args.size());
    std::for_each(_args.begin(), _args.end(), [this](auto a){args.emplace_back(a);});

    switch (f) {
    case ABS:
    case ACOS:
    case ASIN:
    case ATAN:
    case CBRT:
    case CEIL:
    case COS:
    case COSH:
    case EXP:
    case FLOOR:
    case HIDDENREF:
    case HREF:
    case LOG:
    case LOG10:
    case MINVERT:
    case ROTATIONX:
    case ROTATIONY:
    case ROTATIONZ:
    case ROUND:
    case SIN:
    case SINH:
    case SQRT:
    case STR:
    case TAN:
    case TANH:
    case TRUNC:
        if (args.size() != 1)
            ARGUMENT_THROW("exactly one required.");
        break;
    case PLACEMENT:
        if (args.size() > 3)
            ARGUMENT_THROW("exactly one, two, or three required.");
        break;
    case TRANSLATIONM:
        if (args.size() != 1 && args.size() != 3)
            ARGUMENT_THROW("exactly one or three required.");
        break;
    case ATAN2:
    case MOD:
    case MROTATEX:
    case MROTATEY:
    case MROTATEZ:
    case POW:
        if (args.size() != 2)
            ARGUMENT_THROW("exactly two required.");
        break;
    case CATH:
    case HYPOT:
    case ROTATION:
        if (args.size() < 2 || args.size() > 3)
            ARGUMENT_THROW("exactly two, or three required.");
        break;
    case MTRANSLATE:
    case MSCALE:
        if (args.size() != 2 && args.size() != 4)
            ARGUMENT_THROW("exactly two or four required.");
        break;
    case MROTATE:
        if (args.size() < 2 || args.size() > 4)
            ARGUMENT_THROW("exactly two, three, or four required.");
        break;
    case VECTOR:
        if (args.size() != 3)
            ARGUMENT_THROW("exactly three required.");
        break;
    case MATRIX:
        if (args.size() > 16)
            ARGUMENT_THROW("exactly 16 or less required.");
        break;
    case AVERAGE:
    case COUNT:
    case CREATE:
    case MAX:
    case MIN:
    case STDDEV:
    case SUM:
        if (args.empty())
            ARGUMENT_THROW("at least one required.");
        break;
    case LIST:
    case TUPLE:
        break;
    case AGGREGATES:
    case LAST:
    case NONE:
    default:
        PARSER_THROW("Unknown function");
        break;
    }
}

/**
  * Determine whether the expressions is considered touched, i.e one or both of its arguments
  * are touched.
  *
  * @return True if touched, false if not.
  */

bool FunctionExpression::isTouched() const
{
    return std::any_of(args.begin(),
                       args.end(),
                       [](const auto& p){p->isTouched();});
}

/* Various collectors for aggregate functions */

class Collector {
public:
    Collector() : first(true) { }
    virtual ~Collector() = default;
    virtual void collect(Quantity value) {
        if (first)
            q.setUnit(value.getUnit());
    }
    virtual Quantity getQuantity() const {
        return q;
    }
protected:
    bool first;
    Quantity q;
};

class SumCollector : public Collector {
public:
    SumCollector() : Collector() { }

    void collect(Quantity value) override {
        Collector::collect(value);
        q += value;
        first = false;
    }

};

class AverageCollector : public Collector {
public:
    AverageCollector() : Collector(), n(0) { }

    void collect(Quantity value) override {
        Collector::collect(value);
        q += value;
        ++n;
        first = false;
    }

    Quantity getQuantity() const override { return q/(double)n; }

private:
    unsigned int n;
};

class StdDevCollector : public Collector {
public:
    StdDevCollector() : Collector(), n(0) { }

    void collect(Quantity value) override {
        Collector::collect(value);
        if (first) {
            M2 = Quantity(0, value.getUnit() * value.getUnit());
            mean = Quantity(0, value.getUnit());
            n = 0;
        }

        const Quantity delta = value - mean;
        ++n;
        mean = mean + delta / n;
        M2 = M2 + delta * (value - mean);
        first = false;
    }

    Quantity getQuantity() const override {
        if (n < 2)
            throw ExpressionError("Invalid number of entries: at least two required.");
        else
            return Quantity((M2 / (n - 1.0)).pow(Quantity(0.5)).getValue(), mean.getUnit());
    }

private:
    unsigned int n;
    Quantity mean;
    Quantity M2;
};

class CountCollector : public Collector {
public:
    CountCollector() : Collector(), n(0) { }

    void collect(Quantity value) override {
        Collector::collect(value);
        ++n;
        first = false;
    }

    Quantity getQuantity() const override { return Quantity(n); }

private:
    unsigned int n;
};

class MinCollector : public Collector {
public:
    MinCollector() : Collector() { }

    void collect(Quantity value) override {
        Collector::collect(value);
        if (first || value < q)
            q = value;
        first = false;
    }
};

class MaxCollector : public Collector {
public:
    MaxCollector() : Collector() { }

    void collect(Quantity value) override {
        Collector::collect(value);
        if (first || value > q)
            q = value;
        first = false;
    }
};

Py::Object FunctionExpression::evalAggregate(
        const Expression *owner, int f, const std::vector<Expression*> &args)
{
    std::unique_ptr<Collector> c;

    switch (f) {
    case SUM:
        c.reset(new SumCollector);
        break;
    case AVERAGE:
        c.reset(new AverageCollector);
        break;
    case STDDEV:
        c.reset(new StdDevCollector);
        break;
    case COUNT:
        c.reset(new CountCollector);
        break;
    case MIN:
        c.reset(new MinCollector);
        break;
    case MAX:
        c.reset(new MaxCollector);
        break;
    default:
        assert(false);
    }

    for (auto &arg : args) {
        if (arg->isDerivedFrom(RangeExpression::getClassTypeId())) {
            Range range(static_cast<const RangeExpression&>(*arg).getRange());

            do {
                Property * p = owner->getOwner()->getPropertyByName(range.address().c_str());
                PropertyQuantity * qp;
                PropertyFloat * fp;
                PropertyInteger * ip;

                if (!p)
                    continue;

                if ((qp = freecad_dynamic_cast<PropertyQuantity>(p)))
                    c->collect(qp->getQuantityValue());
                else if ((fp = freecad_dynamic_cast<PropertyFloat>(p)))
                    c->collect(Quantity(fp->getValue()));
                else if ((ip = freecad_dynamic_cast<PropertyInteger>(p)))
                    c->collect(Quantity(ip->getValue()));
                else
                    _EXPR_THROW("Invalid property type for aggregate.", owner);
            } while (range.next());
        }
        else {
            Quantity q;
            if(pyToQuantity(q,arg->getPyValue()))
                c->collect(q);
        }
    }

    return pyFromQuantity(c->getQuantity());
}

Base::Vector3d FunctionExpression::evaluateSecondVectorArgument(const Expression *expression, const std::vector<Expression*> &arguments)
{
    Py::Tuple vectorValues;
    Py::Object secondParameter = arguments[1]->getPyValue();

    if (arguments.size() == 2) {
        if (!secondParameter.isSequence())
            _EXPR_THROW("Second parameter is not a sequence type: '" << secondParameter.as_string() << "'.", expression);
        if (PySequence_Size(secondParameter.ptr()) != 3)
            _EXPR_THROW("Second parameter provided has " << PySequence_Size(secondParameter.ptr()) << " elements instead of 3.", expression);

        vectorValues = Py::Tuple(Py::Sequence(secondParameter));
    } else {
        vectorValues = Py::Tuple(3);
        vectorValues.setItem(0, secondParameter);
        vectorValues.setItem(1, arguments[2]->getPyValue());
        vectorValues.setItem(2, arguments[3]->getPyValue());
    }

    Vector3d vector;
    if (!PyArg_ParseTuple(vectorValues.ptr(), "ddd", &vector.x, &vector.y, &vector.z)) {
        PyErr_Clear();
        _EXPR_THROW("Error parsing scale values.", expression);
    }

    return vector;
}

void FunctionExpression::initialiseObject(const Py::Object *object, const std::vector<Expression*> &arguments, const unsigned long offset)
{
    if (arguments.size() > offset) {
        Py::Tuple constructorArguments(arguments.size() - offset);
        for (unsigned i = offset; i < arguments.size(); ++i)
            constructorArguments.setItem(i - offset, arguments[i]->getPyValue());
        Py::Dict kwd;
        PyObjectBase::__PyInit(object->ptr(), constructorArguments.ptr(), kwd.ptr());
    }
}

Py::Object FunctionExpression::transformFirstArgument(
    const Expression* expression,
    const std::vector<Expression*> &arguments,
    const Base::Matrix4D* transformationMatrix
)
{
    Py::Object target = arguments[0]->getPyValue();

    if (PyObject_TypeCheck(target.ptr(), &Base::MatrixPy::Type)) {
        Base::Matrix4D matrix = static_cast<Base::MatrixPy*>(target.ptr())->value();
        return Py::asObject(new Base::MatrixPy(*transformationMatrix * matrix));
    } else if (PyObject_TypeCheck(target.ptr(), &Base::PlacementPy::Type)) {
        Base::Matrix4D placementMatrix =
            static_cast<Base::PlacementPy*>(target.ptr())->getPlacementPtr()->toMatrix();
        return Py::asObject(new Base::PlacementPy(Base::Placement(*transformationMatrix * placementMatrix)));
    } else if (PyObject_TypeCheck(target.ptr(), &Base::RotationPy::Type)) {
        Base::Matrix4D rotatioMatrix;
        static_cast<Base::RotationPy*>(target.ptr())->getRotationPtr()->getValue(rotatioMatrix);
        return Py::asObject(new Base::RotationPy(Base::Rotation(*transformationMatrix * rotatioMatrix)));
    }

    _EXPR_THROW("Function requires the first argument to be either Matrix, Placement or Rotation.", expression);
}

Py::Object FunctionExpression::translationMatrix(double x, double y, double z)
{
    Base::Matrix4D matrix;
    matrix.move(x, y, z);
    return Py::asObject(new Base::MatrixPy(matrix));
}

Py::Object FunctionExpression::evaluate(const Expression *expr, int f, const std::vector<Expression*> &args)
{
    if(!expr || !expr->getOwner())
        _EXPR_THROW("Invalid owner.", expr);

    // Handle aggregate functions
    if (f > AGGREGATES)
        return evalAggregate(expr, f, args);

    switch (f) {
    case LIST: {
        if (args.size() == 1 && args[0]->isDerivedFrom(RangeExpression::getClassTypeId()))
            return args[0]->getPyValue();
        Py::List list(args.size());
        int i = 0;
        for (auto &arg : args)
            list.setItem(i++, arg->getPyValue());
        return list;
    }
    case TUPLE: {
        if (args.size() == 1 && args[0]->isDerivedFrom(RangeExpression::getClassTypeId()))
            return Py::Tuple(args[0]->getPyValue());
        Py::Tuple tuple(args.size());
        int i = 0;
        for (auto &arg : args)
            tuple.setItem(i++, arg->getPyValue());
        return tuple;
    }
    }

    if(args.empty())
        _EXPR_THROW("Function requires at least one argument.",expr);

    switch (f) {
    case MINVERT: {
        Py::Object pyobj = args[0]->getPyValue();
        if (PyObject_TypeCheck(pyobj.ptr(), &Base::MatrixPy::Type)) {
            auto m = static_cast<Base::MatrixPy*>(pyobj.ptr())->value();
            if (fabs(m.determinant()) <= DBL_EPSILON)
                _EXPR_THROW("Cannot invert singular matrix.", expr);
            m.inverseGauss();
            return Py::asObject(new Base::MatrixPy(m));
        } else if (PyObject_TypeCheck(pyobj.ptr(), &Base::PlacementPy::Type)) {
            const auto &pla = *static_cast<Base::PlacementPy*>(pyobj.ptr())->getPlacementPtr();
            return Py::asObject(new Base::PlacementPy(pla.inverse()));
        } else if (PyObject_TypeCheck(pyobj.ptr(), &Base::RotationPy::Type)) {
            const auto &rot = *static_cast<Base::RotationPy*>(pyobj.ptr())->getRotationPtr();
            return Py::asObject(new Base::RotationPy(rot.inverse()));
        }
        _EXPR_THROW(
            "Function requires the first argument to be either Matrix, Placement or Rotation.",
            expr);
        break;
    }
    case MROTATE: {
        Py::Object rotationObject = args[1]->getPyValue();
        if (!PyObject_TypeCheck(rotationObject.ptr(), &Base::RotationPy::Type))
        {
            rotationObject = Py::asObject(new Base::RotationPy(Base::Rotation()));
            initialiseObject(&rotationObject, args, 1);
        }

        Base::Matrix4D rotationMatrix;
        static_cast<Base::RotationPy*>(rotationObject.ptr())->getRotationPtr()->getValue(rotationMatrix);

        return transformFirstArgument(expr, args, &rotationMatrix);
    }
    case MROTATEX:
    case MROTATEY:
    case MROTATEZ:
    {
        Py::Object rotationAngleParameter = args[1]->getPyValue();
        Quantity rotationAngle = pyToQuantity(rotationAngleParameter, expr, "Invalid rotation angle.");

        if (!(rotationAngle.isDimensionlessOrUnit(Unit::Angle)))
            _EXPR_THROW("Unit must be either empty or an angle.", expr);

        Rotation rotation = Base::Rotation(
            Vector3d(static_cast<double>(f == MROTATEX), static_cast<double>(f == MROTATEY), static_cast<double>(f == MROTATEZ)),
            rotationAngle.getValue() * M_PI / 180.0);
        Base::Matrix4D rotationMatrix;
        rotation.getValue(rotationMatrix);

        return transformFirstArgument(expr, args, &rotationMatrix);
    }
    case MSCALE: {
        Vector3d scaleValues = evaluateSecondVectorArgument(expr, args);

        Base::Matrix4D scaleMatrix;
        scaleMatrix.scale(scaleValues);

        return transformFirstArgument(expr, args, &scaleMatrix);
    }
    case MTRANSLATE: {
        Vector3d translateValues = evaluateSecondVectorArgument(expr, args);

        Base::Matrix4D translateMatrix;
        translateMatrix.move(translateValues);

        Py::Object target = args[0]->getPyValue();
        if (PyObject_TypeCheck(target.ptr(), &Base::RotationPy::Type)) {
            Base::Matrix4D targetRotatioMatrix;
            static_cast<Base::RotationPy*>(target.ptr())->getRotationPtr()->getValue(targetRotatioMatrix);
            return Py::asObject(new Base::PlacementPy(Base::Placement(translateMatrix * targetRotatioMatrix)));
        }

        return transformFirstArgument(expr, args, &translateMatrix);
    }
    case CREATE: {
        Py::Object pytype = args[0]->getPyValue();
        if (!pytype.isString())
            _EXPR_THROW("Function requires the first argument to be a string.", expr);
        std::string type(pytype.as_string());
        Py::Object res;
        if (boost::iequals(type, "matrix"))
            res = Py::asObject(new Base::MatrixPy(Base::Matrix4D()));
        else if (boost::iequals(type, "vector"))
            res = Py::asObject(new Base::VectorPy(Base::Vector3d()));
        else if (boost::iequals(type, "placement"))
            res = Py::asObject(new Base::PlacementPy(Base::Placement()));
        else if (boost::iequals(type, "rotation"))
            res = Py::asObject(new Base::RotationPy(Base::Rotation()));
        else
            _EXPR_THROW("Unknown type '" << type << "'.", expr);
        initialiseObject(&res, args, 1);
        return res;
    }
    case MATRIX: {
        Py::Object matrix = Py::asObject(new Base::MatrixPy(Base::Matrix4D()));
        initialiseObject(&matrix, args);
        return matrix;
    }
    case PLACEMENT: {
        Py::Object placement = Py::asObject(new Base::PlacementPy(Base::Placement()));
        initialiseObject(&placement, args);
        return placement;
    }
    case ROTATION: {
        Py::Object rotation = Py::asObject(new Base::RotationPy(Base::Rotation()));
        initialiseObject(&rotation, args);
        return rotation;
    }
    case STR:
        return Py::String(args[0]->getPyValue().as_string());
    case TRANSLATIONM: {
        if (args.size() != 1)
            break; // Break and proceed to 3 size version.
        Py::Object parameter = args[0]->getPyValue();
        if (!parameter.isSequence())
            _EXPR_THROW("Not sequence type: '" << parameter.as_string() << "'.", expr);
        if (PySequence_Size(parameter.ptr()) != 3)
            _EXPR_THROW("Sequence provided has " << PySequence_Size(parameter.ptr()) << " elements instead of 3.", expr);
        double x, y, z;
        if (!PyArg_ParseTuple(Py::Tuple(Py::Sequence(parameter)).ptr(), "ddd", &x, &y, &z)) {
            PyErr_Clear();
            _EXPR_THROW("Error parsing sequence.", expr);
        }
        return translationMatrix(x, y, z);
    }
    case VECTOR: {
        Py::Object vector = Py::asObject(new Base::VectorPy(Base::Vector3d()));
        initialiseObject(&vector, args);
        return vector;
    }
    case HIDDENREF:
    case HREF:
        return args[0]->getPyValue();
    }

    Py::Object e1 = args[0]->getPyValue();
    Quantity v1 = pyToQuantity(e1,expr,"Invalid first argument.");
    Py::Object e2;
    Quantity v2;
    if (args.size() > 1) {
        e2 = args[1]->getPyValue();
        v2 = pyToQuantity(e2,expr,"Invalid second argument.");
    }
    Py::Object e3;
    Quantity v3;
    if (args.size() > 2) {
        e3 = args[2]->getPyValue();
        v3 = pyToQuantity(e3,expr,"Invalid third argument.");
    }

    double output;
    Unit unit;
    double scaler = 1;

    double value = v1.getValue();

    /* Check units and arguments */
    switch (f) {
    case COS:
    case SIN:
    case TAN:
    case ROTATIONX:
    case ROTATIONY:
    case ROTATIONZ:
        if (!(v1.isDimensionlessOrUnit(Unit::Angle)))
            _EXPR_THROW("Unit must be either empty or an angle.", expr);

        // Convert value to radians
        value *= M_PI / 180.0;
        unit = Unit();
        break;
    case ACOS:
    case ASIN:
    case ATAN:
        if (!v1.isDimensionless())
            _EXPR_THROW("Unit must be empty.", expr);
        unit = Unit::Angle;
        scaler = 180.0 / M_PI;
        break;
    case EXP:
    case LOG:
    case LOG10:
    case SINH:
    case TANH:
    case COSH:
        if (!v1.isDimensionless())
            _EXPR_THROW("Unit must be empty.",expr);
        unit = Unit();
        break;
    case ROUND:
    case TRUNC:
    case CEIL:
    case FLOOR:
    case ABS:
        unit = v1.getUnit();
        break;
    case SQRT: {
        unit = v1.getUnit();

        // All components of unit must be either zero or dividable by 2
        UnitSignature s = unit.getSignature();
        if ( !((s.Length % 2) == 0) &&
              ((s.Mass % 2) == 0) &&
              ((s.Time % 2) == 0) &&
              ((s.ElectricCurrent % 2) == 0) &&
              ((s.ThermodynamicTemperature % 2) == 0) &&
              ((s.AmountOfSubstance % 2) == 0) &&
              ((s.LuminousIntensity % 2) == 0) &&
              ((s.Angle % 2) == 0))
            _EXPR_THROW("All dimensions must be even to compute the square root.",expr);

        unit = Unit(s.Length /2,
                    s.Mass / 2,
                    s.Time / 2,
                    s.ElectricCurrent / 2,
                    s.ThermodynamicTemperature / 2,
                    s.AmountOfSubstance / 2,
                    s.LuminousIntensity / 2,
                    s.Angle);
        break;
    }
    case CBRT: {
        unit = v1.getUnit();

        // All components of unit must be either zero or dividable by 3
        UnitSignature s = unit.getSignature();
        if ( !((s.Length % 3) == 0) &&
              ((s.Mass % 3) == 0) &&
              ((s.Time % 3) == 0) &&
              ((s.ElectricCurrent % 3) == 0) &&
              ((s.ThermodynamicTemperature % 3) == 0) &&
              ((s.AmountOfSubstance % 3) == 0) &&
              ((s.LuminousIntensity % 3) == 0) &&
              ((s.Angle % 3) == 0))
            _EXPR_THROW("All dimensions must be multiples of 3 to compute the cube root.",expr);

        unit = Unit(s.Length /3,
                    s.Mass / 3,
                    s.Time / 3,
                    s.ElectricCurrent / 3,
                    s.ThermodynamicTemperature / 3,
                    s.AmountOfSubstance / 3,
                    s.LuminousIntensity / 3,
                    s.Angle);
        break;
    }
    case ATAN2:
        if (e2.isNone())
            _EXPR_THROW("Invalid second argument.",expr);

        if (v1.getUnit() != v2.getUnit())
            _EXPR_THROW("Units must be equal.",expr);
        unit = Unit::Angle;
        scaler = 180.0 / M_PI;
        break;
    case MOD:
        if (e2.isNone())
            _EXPR_THROW("Invalid second argument.",expr);
        unit = v1.getUnit() / v2.getUnit();
        break;
    case POW: {
        if (e2.isNone())
            _EXPR_THROW("Invalid second argument.",expr);

        if (!v2.isDimensionless())
            _EXPR_THROW("Exponent is not allowed to have a unit.",expr);

        // Compute new unit for exponentiation
        double exponent = v2.getValue();
        if (!v1.isDimensionless()) {
            if (exponent - boost::math::round(exponent) < 1e-9)
                unit = v1.getUnit().pow(exponent);
            else
                _EXPR_THROW("Exponent must be an integer when used with a unit.",expr);
        }
        break;
    }
    case HYPOT:
    case CATH:
        if (e2.isNone())
            _EXPR_THROW("Invalid second argument.",expr);
        if (v1.getUnit() != v2.getUnit())
            _EXPR_THROW("Units must be equal.",expr);

        if (args.size() > 2) {
            if (e3.isNone())
                _EXPR_THROW("Invalid second argument.",expr);
            if (v2.getUnit() != v3.getUnit())
                _EXPR_THROW("Units must be equal.",expr);
        }
        unit = v1.getUnit();
        break;
    case TRANSLATIONM:
        if (v1.isDimensionlessOrUnit(Unit::Length) && v2.isDimensionlessOrUnit(Unit::Length) && v3.isDimensionlessOrUnit(Unit::Length))
            break;
        _EXPR_THROW("Translation units must be a length or dimensionless.", expr);
    default:
        _EXPR_THROW("Unknown function: " << f,0);
    }

    /* Compute result */
    switch (f) {
    case ACOS:
        output = acos(value);
        break;
    case ASIN:
        output = asin(value);
        break;
    case ATAN:
        output = atan(value);
        break;
    case ABS:
        output = fabs(value);
        break;
    case EXP:
        output = exp(value);
        break;
    case LOG:
        output = log(value);
        break;
    case LOG10:
        output = log(value) / log(10.0);
        break;
    case SIN:
        output = sin(value);
        break;
    case SINH:
        output = sinh(value);
        break;
    case TAN:
        output = tan(value);
        break;
    case TANH:
        output = tanh(value);
        break;
    case SQRT:
        output = sqrt(value);
        break;
    case CBRT:
        output = cbrt(value);
        break;
    case COS:
        output = cos(value);
        break;
    case COSH:
        output = cosh(value);
        break;
    case MOD: {
        output = fmod(value, v2.getValue());
        break;
    }
    case ATAN2: {
        output = atan2(value, v2.getValue());
        break;
    }
    case POW: {
        output = pow(value, v2.getValue());
        break;
    }
    case HYPOT: {
        output = sqrt(pow(v1.getValue(), 2) + pow(v2.getValue(), 2) + (!e3.isNone() ? pow(v3.getValue(), 2) : 0));
        break;
    }
    case CATH: {
        output = sqrt(pow(v1.getValue(), 2) - pow(v2.getValue(), 2) - (!e3.isNone() ? pow(v3.getValue(), 2) : 0));
        break;
    }
    case ROUND:
        output = boost::math::round(value);
        break;
    case TRUNC:
        output = boost::math::trunc(value);
        break;
    case CEIL:
        output = ceil(value);
        break;
    case FLOOR:
        output = floor(value);
        break;
    case ROTATIONX:
    case ROTATIONY:
    case ROTATIONZ:
        return Py::asObject(new Base::RotationPy(Base::Rotation(
            Vector3d(static_cast<double>(f == ROTATIONX), static_cast<double>(f == ROTATIONY), static_cast<double>(f == ROTATIONZ)),
            value)));
    case TRANSLATIONM:
        return translationMatrix(v1.getValue(), v2.getValue(), v3.getValue());
    default:
        _EXPR_THROW("Unknown function: " << f,0);
    }

    return Py::asObject(new QuantityPy(new Quantity(scaler * output, unit)));
}

Py::Object FunctionExpression::_getPyValue() const {
    return evaluate(this,f,args);
}

/**
  * Try to simplify the expression, i.e calculate all constant expressions.
  *
  * @returns A simplified expression.
  */

std::unique_ptr<Expression> FunctionExpression::simplify() const
{
    size_t numerics = 0;
    std::vector<std::unique_ptr<Expression>> a;

    // Try to simplify each argument to function
    for (auto it = args.begin(); it != args.end(); ++it) {
        std::unique_ptr<Expression> v((*it)->simplify());

        if (freecad_dynamic_cast<NumberExpression>(v.get()))
            ++numerics;
        a.emplace_back(std::move(v));
    }

    if (numerics == args.size()) {
        // All constants, then evaluation must also be constant

        auto result = std::unique_ptr<Expression>(eval());
        // Clean-up
        args.clear();

        return result
    }
    else
        return new FunctionExpression(owner, f, std::string(fname), a);
}

/**
  * Create a string representation of the expression.
  *
  * @returns A string representing the expression.
  */

void FunctionExpression::_toString(std::ostream &ss, bool persistent,int) const
{
    switch (f) {
    case ABS:
        ss << "abs("; break;;
    case ACOS:
        ss << "acos("; break;;
    case ASIN:
        ss << "asin("; break;;
    case ATAN:
        ss << "atan("; break;;
    case ATAN2:
        ss << "atan2("; break;;
    case CATH:
        ss << "cath("; break;;
    case CBRT:
        ss << "cbrt("; break;;
    case CEIL:
        ss << "ceil("; break;;
    case COS:
        ss << "cos("; break;;
    case COSH:
        ss << "cosh("; break;;
    case EXP:
        ss << "exp("; break;;
    case FLOOR:
        ss << "floor("; break;;
    case HYPOT:
        ss << "hypot("; break;;
    case LOG:
        ss << "log("; break;;
    case LOG10:
        ss << "log10("; break;;
    case MOD:
        ss << "mod("; break;;
    case POW:
        ss << "pow("; break;;
    case ROUND:
        ss << "round("; break;;
    case SIN:
        ss << "sin("; break;;
    case SINH:
        ss << "sinh("; break;;
    case SQRT:
        ss << "sqrt("; break;;
    case TAN:
        ss << "tan("; break;;
    case TANH:
        ss << "tanh("; break;;
    case TRUNC:
        ss << "trunc("; break;;
    case MINVERT:
        ss << "minvert("; break;;
    case MROTATE:
        ss << "mrotate("; break;;
    case MROTATEX:
        ss << "mrotatex("; break;;
    case MROTATEY:
        ss << "mrotatey("; break;;
    case MROTATEZ:
        ss << "mrotatez("; break;;
    case MSCALE:
        ss << "mscale("; break;;
    case MTRANSLATE:
        ss << "mtranslate("; break;;
    case CREATE:
        ss << "create("; break;;
    case LIST:
        ss << "list("; break;;
    case MATRIX:
        ss << "matrix("; break;;
    case PLACEMENT:
        ss << "placement("; break;;
    case ROTATION:
        ss << "rotation("; break;;
    case ROTATIONX:
        ss << "rotationx("; break;;
    case ROTATIONY:
        ss << "rotationy("; break;;
    case ROTATIONZ:
        ss << "rotationz("; break;;
    case STR:
        ss << "str("; break;;
    case TRANSLATIONM:
        ss << "translationm("; break;;
    case TUPLE:
        ss << "tuple("; break;;
    case VECTOR:
        ss << "vector("; break;;
    case HIDDENREF:
        ss << "hiddenref("; break;;
    case HREF:
        ss << "href("; break;;
    case AVERAGE:
        ss << "average("; break;;
    case COUNT:
        ss << "count("; break;;
    case MAX:
        ss << "max("; break;;
    case MIN:
        ss << "min("; break;;
    case STDDEV:
        ss << "stddev("; break;;
    case SUM:
        ss << "sum("; break;;
    default:
        ss << fname << "("; break;;
    }
    for (size_t i = 0; i < args.size(); ++i) {
        ss << args[i]->toString(persistent);
        if (i != args.size() - 1)
            ss << "; ";
    }
    ss << ')';
}

/**
  * Create a copy of the expression.
  *
  * @returns A deep copy of the expression.
  */

std::unique_ptr<Expression> FunctionExpression::_copy() const
{
    std::vector<Expression*>::const_iterator i = args.begin();
    std::vector<Expression*> a;

    while (i != args.end()) {
        a.push_back((*i)->copy());
        ++i;
    }
    return std::make_unique<FunctionExpression>(owner, f, std::string(fname), a);
}

void FunctionExpression::_visit(ExpressionVisitor &v)
{
    std::vector<Expression*>::const_iterator i = args.begin();

    HiddenReference ref(f == HIDDENREF || f == HREF);
    while (i != args.end()) {
        (*i)->visit(v);
        ++i;
    }
}

//
// VariableExpression class
//

TYPESYSTEM_SOURCE(App::VariableExpression, App::UnitExpression)

VariableExpression::VariableExpression(const DocumentObject *_owner, const ObjectIdentifier& _var)
    : UnitExpression(_owner)
    , var(_var)
{
}

VariableExpression::~VariableExpression() = default;

/**
  * Determine if the expression is touched or not, i.e whether the Property object it
  * refers to is touched().
  *
  * @returns True if the Property object is touched, false if not.
  */

bool VariableExpression::isTouched() const
{
    return var.isTouched();
}

/**
  * Find the property this expression referse to.
  *
  * Unqualified names (i.e the name only without any dots) are resolved in the owning DocumentObjects.
  * Qualified names are looked up in the owning Document. It is first looked up by its internal name.
  * If not found, the DocumentObjects' labels searched.
  *
  * If something fails, an exception is thrown.
  *
  * @returns The Property object if it is derived from either PropertyInteger, PropertyFloat, or PropertyString.
  */

const Property * VariableExpression::getProperty() const
{
    const Property * prop = var.getProperty();

    if (prop)
        return prop;
    else
        throw Expression::Exception(var.resolveErrorString().c_str());
}

bool VariableExpression::_isIndexable() const {
    return true;
}

Py::Object VariableExpression::_getPyValue() const {
    return var.getPyValue(true);
}

void VariableExpression::_toString(std::ostream &ss, bool persistent,int) const {
    if(persistent)
        ss << var.toPersistentString();
    else
        ss << var.toString();
}

/**
  * Simplify the expression. Simplification of VariableExpression objects is
  * not possible (if it is instantiated it would be an evaluation instead).
  *
  * @returns A copy of the expression.
  */

std::unique_ptr<Expression> VariableExpression::simplify() const
{
    return copy();
}

/**
  * Return a copy of the expression.
  */

std::unique_ptr<Expression> VariableExpression::_copy() const
{
    return std::make_unique<VariableExpression>(owner, var);
}

void VariableExpression::_getIdentifiers(std::map<App::ObjectIdentifier,bool> &deps) const
{
    bool hidden = HiddenReference::isHidden();
    auto res = deps.insert(std::make_pair(var,hidden));
    if(!hidden || res.second)
        res.first->second = hidden;
}

bool VariableExpression::_relabeledDocument(const std::string &oldName,
        const std::string &newName, ExpressionVisitor &v)
{
    return var.relabeledDocument(v, oldName, newName);
}

bool VariableExpression::_adjustLinks(
        const std::set<App::DocumentObject *> &inList, ExpressionVisitor &v)
{
    return var.adjustLinks(v,inList);
}

void VariableExpression::_importSubNames(const ObjectIdentifier::SubNameMap &subNameMap)
{
    var.importSubNames(subNameMap);
}

void VariableExpression::_updateLabelReference(
        App::DocumentObject *obj, const std::string &ref, const char *newLabel)
{
    var.updateLabelReference(obj,ref,newLabel);
}

bool VariableExpression::_updateElementReference(
        App::DocumentObject *feature, bool reverse, ExpressionVisitor &v)
{
    return var.updateElementReference(v,feature,reverse);
}

bool VariableExpression::_renameObjectIdentifier(
        const std::map<ObjectIdentifier,ObjectIdentifier> &paths,
        const ObjectIdentifier &path, ExpressionVisitor &v)
{
    const auto &oldPath = var.canonicalPath();
    auto it = paths.find(oldPath);
    if (it != paths.end()) {
        v.aboutToChange();
        if(path.getOwner())
            var = it->second.relativeTo(path);
        else
            var = it->second;
        return true;
    }
    return false;
}

void VariableExpression::_collectReplacement(
        std::map<ObjectIdentifier,ObjectIdentifier> &paths,
        const App::DocumentObject *parent,
        App::DocumentObject *oldObj,
        App::DocumentObject *newObj) const
{
    ObjectIdentifier path;
    if(var.replaceObject(path,parent,oldObj,newObj))
        paths[var.canonicalPath()] = std::move(path);
}

void VariableExpression::_moveCells(const CellAddress &address,
        int rowCount, int colCount, ExpressionVisitor &v)
{
    if(var.hasDocumentObjectName(true))
        return;

    int idx = 0;
    const auto &comp = var.getPropertyComponent(0,&idx);
    CellAddress addr = stringToAddress(comp.getName().c_str(),true);
    if(!addr.isValid())
        return;

    int thisRow = addr.row();
    int thisCol = addr.col();
    if (thisRow >= address.row() || thisCol >= address.col()) {
        v.aboutToChange();
        addr.setRow(thisRow + rowCount);
        addr.setCol(thisCol + colCount);
        var.setComponent(idx,ObjectIdentifier::SimpleComponent(addr.toString()));
    }
}

void VariableExpression::_offsetCells(int rowOffset, int colOffset, ExpressionVisitor &v) {
    if(var.hasDocumentObjectName(true))
        return;

    int idx = 0;
    const auto &comp = var.getPropertyComponent(0,&idx);
    CellAddress addr = stringToAddress(comp.getName().c_str(),true);
    if(!addr.isValid() || (addr.isAbsoluteCol() && addr.isAbsoluteRow()))
        return;

    if(!addr.isAbsoluteCol())
        addr.setCol(addr.col()+colOffset);
    if(!addr.isAbsoluteRow())
        addr.setRow(addr.row()+rowOffset);
    if(!addr.isValid()) {
        FC_WARN("Not changing relative cell reference '"
                << comp.getName() << "' due to invalid offset "
                << '(' << colOffset << ", " << rowOffset << ')');
    } else {
        v.aboutToChange();
        var.setComponent(idx,ObjectIdentifier::SimpleComponent(addr.toString()));
    }
}

void VariableExpression::setPath(const ObjectIdentifier &path)
{
     var = path;
}

//
// PyObjectExpression class
//

TYPESYSTEM_SOURCE(App::PyObjectExpression, App::Expression)

PyObjectExpression::~PyObjectExpression() {
    if(pyObj) {
        Base::PyGILStateLocker lock;
        Py::_XDECREF(pyObj);
    }
}

Py::Object PyObjectExpression::_getPyValue() const {
    if(!pyObj)
        return Py::Object();
    return Py::Object(pyObj);
}

void PyObjectExpression::setPyValue(Py::Object obj) {
    Py::_XDECREF(pyObj);
    pyObj = obj.ptr();
    Py::_XINCREF(pyObj);
}

void PyObjectExpression::setPyValue(PyObject *obj, bool owned) {
    if(pyObj == obj)
        return;
    Py::_XDECREF(pyObj);
    pyObj = obj;
    if(!owned)
        Py::_XINCREF(pyObj);
}

void PyObjectExpression::_toString(std::ostream &ss, bool,int) const
{
    if(!pyObj)
        ss << "None";
    else {
        Base::PyGILStateLocker lock;
        ss << Py::Object(pyObj).as_string();
    }
}

std::unique_ptr<Expression> PyObjectExpression::_copy() const
{
    return std::make_unique<PyObjectExpression>(owner,pyObj,false);
}

//
// StringExpression class
//

TYPESYSTEM_SOURCE(App::StringExpression, App::Expression)

StringExpression::StringExpression(const DocumentObject *_owner, const std::string &_text)
    : Expression(_owner)
    , text(_text)
{
}

StringExpression::~StringExpression() {
    if(cache) {
        Base::PyGILStateLocker lock;
        Py::_XDECREF(cache);
    }
}

/**
  * Simplify the expression. For strings, this is a simple copy of the object.
  */

std::unique_ptr<Expression> StringExpression::simplify() const
{
    return copy();
}

void StringExpression::_toString(std::ostream &ss, bool,int) const
{
    ss << quote(text);
}

/**
  * Return a copy of the expression.
  */

std::unique_ptr<Expression> StringExpression::_copy() const
{
    return std::make_unique<StringExpression>(owner, text);
}

Py::Object StringExpression::_getPyValue() const {
    return Py::String(text);
}

TYPESYSTEM_SOURCE(App::ConditionalExpression, App::Expression)

ConditionalExpression::ConditionalExpression(const DocumentObject *_owner, Expression *_condition, Expression *_trueExpr, Expression *_falseExpr)
    : Expression(_owner)
    , condition(_condition)
    , trueExpr(_trueExpr)
    , falseExpr(_falseExpr)
{
}

ConditionalExpression::~ConditionalExpression()
{
    delete condition;
    delete trueExpr;
    delete falseExpr;
}

bool ConditionalExpression::isTouched() const
{
    return condition->isTouched() || trueExpr->isTouched() || falseExpr->isTouched();
}

Py::Object ConditionalExpression::_getPyValue() const {
    if(condition->getPyValue().isTrue())
        return trueExpr->getPyValue();
    else
        return falseExpr->getPyValue();
}

std::unique_ptr<Expression> ConditionalExpression::simplify() const
{
    std::unique_ptr<Expression> e(condition->simplify());
    NumberExpression * v = freecad_dynamic_cast<NumberExpression>(e.get());

    if (!v)
        return std::make_unique<ConditionalExpression>(owner, condition->simplify(), trueExpr->simplify(), falseExpr->simplify());
    else {
        if (fabs(v->getValue()) > 0.5)
            return trueExpr->simplify();
        else
            return falseExpr->simplify();
    }
}

void ConditionalExpression::_toString(std::ostream &ss, bool persistent,int) const
{
    condition->toString(ss,persistent);
    ss << " ? ";
    if (trueExpr->priority() <= priority()) {
        ss << '(';
        trueExpr->toString(ss,persistent);
        ss << ')';
    } else
        trueExpr->toString(ss,persistent);

    ss << " : ";

    if (falseExpr->priority() <= priority()) {
        ss << '(';
        falseExpr->toString(ss,persistent);
        ss << ')';
    } else
        falseExpr->toString(ss,persistent);
}

std::unique_ptr<Expression> ConditionalExpression::_copy() const
{
    return std::make_unique<ConditionalExpression>(owner, condition->copy(), trueExpr->copy(), falseExpr->copy());
}

int ConditionalExpression::priority() const
{
    return 2;
}

void ConditionalExpression::_visit(ExpressionVisitor &v)
{
    condition->visit(v);
    trueExpr->visit(v);
    falseExpr->visit(v);
}

TYPESYSTEM_SOURCE(App::ConstantExpression, App::NumberExpression)

ConstantExpression::ConstantExpression(const DocumentObject *_owner,
        const char *_name, const Quantity & _quantity)
    : NumberExpression(_owner, _quantity)
    , name(_name)
{
}

std::unique_ptr<Expression> ConstantExpression::_copy() const
{
    return std::make_unique<ConstantExpression>(owner, name, getQuantity());
}

void ConstantExpression::_toString(std::ostream &ss, bool,int) const
{
    ss << name;
}

Py::Object ConstantExpression::_getPyValue() const {
    if(!cache) {
        if(strcmp(name,"None")==0)
            cache = Py::new_reference_to(Py::None());
        else if(strcmp(name,"True")==0)
            cache = Py::new_reference_to(Py::True());
        else if(strcmp(name, "False")==0)
            cache = Py::new_reference_to(Py::False());
        else
            return NumberExpression::_getPyValue();
    }
    return Py::Object(cache);
}

bool ConstantExpression::isNumber() const {
    return strcmp(name,"None")
        && strcmp(name,"True")
        && strcmp(name, "False");
}

TYPESYSTEM_SOURCE(App::RangeExpression, App::Expression)

RangeExpression::RangeExpression(const DocumentObject *_owner, const std::string &begin, const std::string &end)
    : Expression(_owner), begin(begin), end(end)
{
}

bool RangeExpression::isTouched() const
{
    Range i(getRange());

    do {
        Property * prop = owner->getPropertyByName(i.address().c_str());

        if (prop && prop->isTouched())
            return true;
    } while (i.next());

    return false;
}

Py::Object RangeExpression::_getPyValue() const {
    Py::List list;
    Range range(getRange());
    do {
        Property * p = owner->getPropertyByName(range.address().c_str());
        if(p)
            list.append(Py::asObject(p->getPyObject()));
    } while (range.next());
    return list;
}

void RangeExpression::_toString(std::ostream &ss, bool,int) const
{
    ss << begin << ":" << end;
}

std::unique_ptr<Expression> RangeExpression::_copy() const
{
    return std::make_unique<RangeExpression>(owner, begin, end);
}

std::unique_ptr<Expression> RangeExpression::simplify() const
{
    return copy();
}

void RangeExpression::_getIdentifiers(std::map<App::ObjectIdentifier,bool> &deps) const
{
    bool hidden = HiddenReference::isHidden();

    assert(owner);

    Range i(getRange());

    do {
        ObjectIdentifier var(owner,i.address());
        auto res = deps.insert(std::make_pair(var,hidden));
        if(!hidden || res.second)
            res.first->second = hidden;
    } while (i.next());
}

Range RangeExpression::getRange() const
{
    auto c1 = stringToAddress(begin.c_str(),true);
    auto c2 = stringToAddress(end.c_str(),true);
    if(c1.isValid() && c2.isValid())
        return Range(c1,c2);

    Base::PyGILStateLocker lock;
    static const std::string attr("getCellFromAlias");
    Py::Object pyobj(owner->getPyObject(),true);
    if(!pyobj.hasAttr(attr))
        EXPR_THROW("Invalid cell range " << begin << ':' << end);
    Py::Callable callable(pyobj.getAttr(attr));
    if(!c1.isValid()) {
        try {
            Py::Tuple arg(1);
            arg.setItem(0,Py::String(begin));
            c1 = CellAddress(callable.apply(arg).as_string().c_str());
        } catch(Py::Exception &) {
            _EXPR_PY_THROW("Invalid cell address '" << begin << "': ",this);
        } catch(Base::Exception &e) {
            _EXPR_RETHROW(e,"Invalid cell address '" << begin << "': ",this);
        }
    }
    if(!c2.isValid()) {
        try {
            Py::Tuple arg(1);
            arg.setItem(0,Py::String(end));
            c2 = CellAddress(callable.apply(arg).as_string().c_str());
        } catch(Py::Exception &) {
            _EXPR_PY_THROW("Invalid cell address '" << end << "': ", this);
        } catch(Base::Exception &e) {
            _EXPR_RETHROW(e,"Invalid cell address '" << end << "': ", this);
        }
    }
    return Range(c1,c2);
}

bool RangeExpression::_renameObjectIdentifier(
        const std::map<ObjectIdentifier,ObjectIdentifier> &paths,
        const ObjectIdentifier &path, ExpressionVisitor &v)
{
    (void)path;
    bool touched =false;
    auto it = paths.find(ObjectIdentifier(owner,begin));
    if (it != paths.end()) {
        v.aboutToChange();
        begin = it->second.getPropertyName();
        touched = true;
    }
    it = paths.find(ObjectIdentifier(owner,end));
    if (it != paths.end()) {
        v.aboutToChange();
        end = it->second.getPropertyName();
        touched = true;
    }
    return touched;
}

void RangeExpression::_moveCells(const CellAddress &address,
        int rowCount, int colCount, ExpressionVisitor &v)
{
    CellAddress addr = stringToAddress(begin.c_str(),true);
    if(addr.isValid()) {
        int thisRow = addr.row();
        int thisCol = addr.col();
        if (thisRow >= address.row() || thisCol >= address.col()) {
            v.aboutToChange();
            addr.setRow(thisRow+rowCount);
            addr.setCol(thisCol+colCount);
            begin = addr.toString();
        }
    }
    addr = stringToAddress(end.c_str(),true);
    if(addr.isValid()) {
        int thisRow = addr.row();
        int thisCol = addr.col();
        if (thisRow >= address.row() || thisCol >= address.col()) {
            v.aboutToChange();
            addr.setRow(thisRow + rowCount);
            addr.setCol(thisCol + colCount);
            end = addr.toString();
        }
    }
}

void RangeExpression::_offsetCells(int rowOffset, int colOffset, ExpressionVisitor &v)
{
    CellAddress addr = stringToAddress(begin.c_str(),true);
    if(addr.isValid() && (!addr.isAbsoluteRow() || !addr.isAbsoluteCol())) {
        v.aboutToChange();
        if(!addr.isAbsoluteRow())
            addr.setRow(addr.row()+rowOffset);
        if(!addr.isAbsoluteCol())
            addr.setCol(addr.col()+colOffset);
        begin = addr.toString();
    }
    addr = stringToAddress(end.c_str(),true);
    if(addr.isValid() && (!addr.isAbsoluteRow() || !addr.isAbsoluteCol())) {
        v.aboutToChange();
        if(!addr.isAbsoluteRow())
            addr.setRow(addr.row()+rowOffset);
        if(!addr.isAbsoluteCol())
            addr.setCol(addr.col()+colOffset);
        end = addr.toString();
    }
}


////////////////////////////////////////////////////////////////////////////////////

static Base::XMLReader *_Reader = nullptr;
ExpressionParser::ExpressionImporter::ExpressionImporter(Base::XMLReader &reader) {
    assert(!_Reader);
    _Reader = &reader;
}

ExpressionParser::ExpressionImporter::~ExpressionImporter() {
    assert(_Reader);
    _Reader = nullptr;
}

Base::XMLReader *ExpressionParser::ExpressionImporter::reader() {
    return _Reader;
}

namespace App {

namespace ExpressionParser {

bool isModuleImported(PyObject *module) {
    (void)module;
    return false;
}

/**
 * Error function for parser. Throws a generic Base::Exception with the parser error.
 */

void ExpressionParser_yyerror(const char *errorinfo)
{
    (void)errorinfo;
}

/* helper function for tuning number strings with groups in a locale agnostic way... */
double num_change(char* yytext,char dez_delim,char grp_delim)
{
    double ret_val;
    char temp[40];
    int i = 0;
    for(char* c=yytext;*c!='\0';c++){
        // skip group delimiter
        if(*c==grp_delim) continue;
        // check for a dez delimiter other then dot
        if(*c==dez_delim && dez_delim !='.')
             temp[i++] = '.';
        else
            temp[i++] = *c;
        // check buffer overflow
        if (i>39)
            return 0.0;
    }
    temp[i] = '\0';

    errno = 0;
    ret_val = strtod( temp, nullptr );
    if (ret_val == 0 && errno == ERANGE)
        throw Base::UnderflowError("Number underflow.");
    if (ret_val == HUGE_VAL || ret_val == -HUGE_VAL)
        throw Base::OverflowError("Number overflow.");

    return ret_val;
}

static Expression * ScanResult = nullptr;                    /**< The resulting expression after a successful parsing */
static const App::DocumentObject * DocumentObject = nullptr; /**< The DocumentObject that will own the expression */
static bool unitExpression = false;                    /**< True if the parsed string is a unit only */
static bool valueExpression = false;                   /**< True if the parsed string is a full expression */
static std::stack<std::string> labels;                /**< Label string primitive */
static std::map<std::string, FunctionExpression::Function> registered_functions;                /**< Registered functions */
static int last_column;
static int column;

// show the parser the lexer method
#define yylex ExpressionParserlex
int ExpressionParserlex();

#if defined(__clang__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wsign-compare"
# pragma clang diagnostic ignored "-Wunneeded-internal-declaration"
#elif defined (__GNUC__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wsign-compare"
# pragma GCC diagnostic ignored "-Wfree-nonheap-object"
#endif

// Parser, defined in ExpressionParser.y
# define YYTOKENTYPE
#include "ExpressionParser.tab.c"

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// Scanner, defined in ExpressionParser.l
#include "lex.ExpressionParser.c"
#endif // DOXYGEN_SHOULD_SKIP_THIS

#if defined(__clang__)
# pragma clang diagnostic pop
#elif defined (__GNUC__)
# pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
# define strdup _strdup
#endif

static void initParser(const App::DocumentObject *owner)
{
    static bool has_registered_functions = false;

    using namespace App::ExpressionParser;

    ScanResult = nullptr;
    App::ExpressionParser::DocumentObject = owner;
    labels = std::stack<std::string>();
    column = 0;
    unitExpression = valueExpression = false;

    if (!has_registered_functions) {
        registered_functions["abs"] = FunctionExpression::ABS;
        registered_functions["acos"] = FunctionExpression::ACOS;
        registered_functions["asin"] = FunctionExpression::ASIN;
        registered_functions["atan"] = FunctionExpression::ATAN;
        registered_functions["atan2"] = FunctionExpression::ATAN2;
        registered_functions["cath"] = FunctionExpression::CATH;
        registered_functions["cbrt"] = FunctionExpression::CBRT;
        registered_functions["ceil"] = FunctionExpression::CEIL;
        registered_functions["cos"] = FunctionExpression::COS;
        registered_functions["cosh"] = FunctionExpression::COSH;
        registered_functions["exp"] = FunctionExpression::EXP;
        registered_functions["floor"] = FunctionExpression::FLOOR;
        registered_functions["hypot"] = FunctionExpression::HYPOT;
        registered_functions["log"] = FunctionExpression::LOG;
        registered_functions["log10"] = FunctionExpression::LOG10;
        registered_functions["mod"] = FunctionExpression::MOD;
        registered_functions["pow"] = FunctionExpression::POW;
        registered_functions["round"] = FunctionExpression::ROUND;
        registered_functions["sin"] = FunctionExpression::SIN;
        registered_functions["sinh"] = FunctionExpression::SINH;
        registered_functions["sqrt"] = FunctionExpression::SQRT;
        registered_functions["tan"] = FunctionExpression::TAN;
        registered_functions["tanh"] = FunctionExpression::TANH;
        registered_functions["trunc"] = FunctionExpression::TRUNC;

        registered_functions["minvert"] = FunctionExpression::MINVERT;
        registered_functions["mrotate"] = FunctionExpression::MROTATE;
        registered_functions["mrotatex"] = FunctionExpression::MROTATEX;
        registered_functions["mrotatey"] = FunctionExpression::MROTATEY;
        registered_functions["mrotatez"] = FunctionExpression::MROTATEZ;
        registered_functions["mscale"] = FunctionExpression::MSCALE;
        registered_functions["mtranslate"] = FunctionExpression::MTRANSLATE;

        registered_functions["create"] = FunctionExpression::CREATE;
        registered_functions["list"] = FunctionExpression::LIST;
        registered_functions["matrix"] = FunctionExpression::MATRIX;
        registered_functions["placement"] = FunctionExpression::PLACEMENT;
        registered_functions["rotation"] = FunctionExpression::ROTATION;
        registered_functions["rotationx"] = FunctionExpression::ROTATIONX;
        registered_functions["rotationy"] = FunctionExpression::ROTATIONY;
        registered_functions["rotationz"] = FunctionExpression::ROTATIONZ;
        registered_functions["str"] = FunctionExpression::STR;
        registered_functions["translationm"] = FunctionExpression::TRANSLATIONM;
        registered_functions["tuple"] = FunctionExpression::TUPLE;
        registered_functions["vector"] = FunctionExpression::VECTOR;

        registered_functions["hiddenref"] = FunctionExpression::HIDDENREF;
        registered_functions["href"] = FunctionExpression::HREF;

        // Aggregates
        registered_functions["average"] = FunctionExpression::AVERAGE;
        registered_functions["count"] = FunctionExpression::COUNT;
        registered_functions["max"] = FunctionExpression::MAX;
        registered_functions["min"] = FunctionExpression::MIN;
        registered_functions["stddev"] = FunctionExpression::STDDEV;
        registered_functions["sum"] = FunctionExpression::SUM;

        has_registered_functions = true;
    }
}

std::vector<std::tuple<int, int, std::string> > tokenize(const std::string &str)
{
    ExpressionParser::YY_BUFFER_STATE buf = ExpressionParser_scan_string(str.c_str());
    std::vector<std::tuple<int, int, std::string> > result;
    int token;

    column = 0;
    try {
        while ( (token  = ExpressionParserlex()) != 0)
            result.emplace_back(token, ExpressionParser::last_column, yytext);
    }
    catch (...) {
        // Ignore all exceptions
    }

    ExpressionParser_delete_buffer(buf);
    return result;
}

}

}

/**
  * Parse the expression given by \a buffer, and use \a owner as the owner of the
  * returned expression. If the parser fails for some reason, and exception is thrown.
  *
  * @param owner  The DocumentObject that will own the expression.
  * @param buffer The string buffer to parse.
  *
  * @returns A pointer to an expression.
  *
  */

Expression * App::ExpressionParser::parse(const App::DocumentObject *owner, const char* buffer)
{
    // parse from buffer
    ExpressionParser::YY_BUFFER_STATE my_string_buffer = ExpressionParser::ExpressionParser_scan_string (buffer);

    initParser(owner);

    // run the parser
    int result = ExpressionParser::ExpressionParser_yyparse ();

    // free the scan buffer
    ExpressionParser::ExpressionParser_delete_buffer (my_string_buffer);

    if (result != 0)
        throw ParserError("Failed to parse expression.");

    if (!ScanResult)
        throw ParserError("Unknown error in expression");

    if (valueExpression)
        return ScanResult;
    else {
        delete ScanResult;
        throw Expression::Exception("Expression can not evaluate to a value.");
    }
}

UnitExpression * ExpressionParser::parseUnit(const App::DocumentObject *owner, const char* buffer)
{
    // parse from buffer
    ExpressionParser::YY_BUFFER_STATE my_string_buffer = ExpressionParser::ExpressionParser_scan_string (buffer);

    initParser(owner);

    // run the parser
    int result = ExpressionParser::ExpressionParser_yyparse ();

    // free the scan buffer
    ExpressionParser::ExpressionParser_delete_buffer (my_string_buffer);

    if (result != 0)
        throw ParserError("Failed to parse expression.");

    if (!ScanResult)
        throw ParserError("Unknown error in expression");

    // Simplify expression
    Expression * simplified = ScanResult->simplify();

    if (!unitExpression) {
        OperatorExpression * fraction = freecad_dynamic_cast<OperatorExpression>(ScanResult);

        if (fraction && fraction->getOperator() == OperatorExpression::DIV) {
            NumberExpression * nom = freecad_dynamic_cast<NumberExpression>(fraction->getLeft());
            UnitExpression * denom = freecad_dynamic_cast<UnitExpression>(fraction->getRight());

            // If not initially a unit expression, but value is equal to 1, it means the expression is something like 1/unit
            if (denom && nom && essentiallyEqual(nom->getValue(), 1.0))
                unitExpression = true;
        }
    }
    delete ScanResult;

    if (unitExpression) {
        NumberExpression * num = freecad_dynamic_cast<NumberExpression>(simplified);

        if (num) {
           simplified = new UnitExpression(num->getOwner(), num->getQuantity());
            delete num;
        }
        return freecad_dynamic_cast<UnitExpression>(simplified);
    }
    else {
        delete simplified;
        throw Expression::Exception("Expression is not a unit.");
    }
}

namespace {
std::tuple<int, int> getTokenAndStatus(const std::string & str)
{
    ExpressionParser::YY_BUFFER_STATE buf = ExpressionParser::ExpressionParser_scan_string(str.c_str());
    int token = ExpressionParser::ExpressionParserlex();
    int status = ExpressionParser::ExpressionParserlex();
    ExpressionParser::ExpressionParser_delete_buffer(buf);

    return std::make_tuple(token, status);
}
}

bool ExpressionParser::isTokenAnIndentifier(const std::string & str)
{
    int token{}, status{};
    std::tie(token, status) = getTokenAndStatus(str);
    return (status == 0 && (token == IDENTIFIER || token == CELLADDRESS));
}

bool ExpressionParser::isTokenAUnit(const std::string & str)
{
    int token{}, status{};
    std::tie(token, status) = getTokenAndStatus(str);
    return (status == 0 && token == UNIT);
}

#if defined(__clang__)
# pragma clang diagnostic pop
#endif
