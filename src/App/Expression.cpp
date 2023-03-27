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

#include "ExpressionComponent.h"
#include "ExpressionParser.h"
#include "ExpressionAny.h"

#include "Expression.h"


/** \defgroup Expression Expressions framework
    \ingroup APP
    \brief The expression system allows users to write expressions and formulas that produce values
*/

using namespace Base;
using namespace App;
using namespace App::ExpressionHelper;

FC_LOG_LEVEL_INIT("Expression", true, true)


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

static inline std::ostream &operator<<(std::ostream &os, const App::Expression *expr) {
    if(expr) {
        os << "\nin expression: ";
        expr->toString(os);
    }
    return os;
}

template<typename T>
void copy_vector(T &dst, const T& src) {
    dst.clear();
    dst.reserve(src.size());
    for(auto &s : src) {
        if(s)
            dst.push_back(s->copy());
        else
            dst.emplace_back();
    }
}


std::string unquote(const std::string & input)
{
    assert(input.size() >= 4);

    std::string output;
    std::string::const_iterator cur = input.begin() + 2;
    std::string::const_iterator end = input.end() - 2;

    output.reserve(input.size());

    bool escaped = false;
    while (cur != end) {
        if (escaped) {
            switch (*cur) {
            case 't':
                output += '\t';
                break;
            case 'n':
                output += '\n';
                break;
            case 'r':
                output += '\r';
                break;
            case '\\':
                output += '\\';
                break;
            case '\'':
                output += '\'';
                break;
            case '"':
                output += '"';
                break;
            }
            escaped = false;
        }
        else {
            if (*cur == '\\')
                escaped = true;
            else
                output += *cur;
        }
        ++cur;
    }

    return output;
}

////////////////////////////////////////////////////////////////////////////////////
//
// ExpressionVistor
//
void ExpressionVisitor::getIdentifiers(Expression &e, std::map<App::ObjectIdentifier,bool> &ids) {
    e._getIdentifiers(ids);
}

bool ExpressionVisitor::adjustLinks(Expression &e, const std::set<App::DocumentObject*> &inList) {
    return e._adjustLinks(inList,*this);
}

void ExpressionVisitor::importSubNames(Expression &e, const ObjectIdentifier::SubNameMap &subNameMap) {
    e._importSubNames(subNameMap);
}

void ExpressionVisitor::updateLabelReference(Expression &e,
        DocumentObject *obj, const std::string &ref, const char *newLabel)
{
    e._updateLabelReference(obj,ref,newLabel);
}

bool ExpressionVisitor::updateElementReference(Expression &e, App::DocumentObject *feature, bool reverse) {
    return e._updateElementReference(feature,reverse,*this);
}

bool ExpressionVisitor::relabeledDocument(
        Expression &e, const std::string &oldName, const std::string &newName)
{
    return e._relabeledDocument(oldName,newName,*this);
}

bool ExpressionVisitor::renameObjectIdentifier(Expression &e,
        const std::map<ObjectIdentifier,ObjectIdentifier> &paths, const ObjectIdentifier &path)
{
    return e._renameObjectIdentifier(paths,path,*this);
}

void ExpressionVisitor::collectReplacement(Expression &e,
        std::map<ObjectIdentifier,ObjectIdentifier> &paths,
        const App::DocumentObject *parent, App::DocumentObject *oldObj, App::DocumentObject *newObj) const
{
    return e._collectReplacement(paths,parent,oldObj,newObj);
}

void ExpressionVisitor::moveCells(Expression &e, const CellAddress &address, int rowCount, int colCount) {
    e._moveCells(address,rowCount,colCount,*this);
}

void ExpressionVisitor::offsetCells(Expression &e, int rowOffset, int colOffset) {
    e._offsetCells(rowOffset,colOffset,*this);
}

Expression* expressionFromPy(const DocumentObject *owner, const Py::Object &value) {
    if (value.isNone())
        return new PyObjectExpression(owner);
    if(value.isString()) {
        return new StringExpression(owner,value.as_string());
    } else if (PyObject_TypeCheck(value.ptr(),&QuantityPy::Type)) {
        return new NumberExpression(owner,
                *static_cast<QuantityPy*>(value.ptr())->getQuantityPtr());
    } else if (value.isBoolean()) {
        if(value.isTrue())
            return new ConstantExpression(owner,"True",Quantity(1.0));
        else
            return new ConstantExpression(owner,"False",Quantity(0.0));
    } else {
        Quantity q;
        if(pyToQuantity(q,value))
            return new NumberExpression(owner,q);
    }
    return new PyObjectExpression(owner,value.ptr());
}



//
// Expression base-class
//

TYPESYSTEM_SOURCE_ABSTRACT(App::Expression, Base::BaseClass)

Expression::Expression(const DocumentObject *_owner)
    : owner(const_cast<App::DocumentObject*>(_owner))
{

}

Expression::~Expression()
{
}

int Expression::priority() const {
    return 20;
}

Expression * Expression::parse(const DocumentObject *owner, const std::string &buffer)
{
    return ExpressionParser::parse(owner, buffer.c_str());
}

void Expression::getDeps(ExpressionDeps &deps, int option)  const {
    for(auto &v : getIdentifiers()) {
        bool hidden = v.second;
        const ObjectIdentifier &var = v.first;
        if((hidden && option==DepNormal)
                || (!hidden && option==DepHidden))
            continue;
        for(auto &dep : var.getDep(true)) {
            DocumentObject *obj = dep.first;
            for(auto &propName : dep.second) {
                deps[obj][propName].push_back(var);
            }
        }
    }
}

ExpressionDeps Expression::getDeps(int option)  const {
    ExpressionDeps deps;
    getDeps(deps, option);
    return deps;
}

void Expression::getDepObjects(
        std::map<App::DocumentObject*,bool> &deps, std::vector<std::string> *labels) const
{
    for(auto &v : getIdentifiers()) {
        bool hidden = v.second;
        const ObjectIdentifier &var = v.first;
        std::vector<std::string> strings;
        for(auto &dep : var.getDep(false, &strings)) {
            DocumentObject *obj = dep.first;
            if (!obj->testStatus(ObjectStatus::Remove)) {
                if (labels) {
                    std::copy(strings.begin(), strings.end(), std::back_inserter(*labels));
                }

                auto res = deps.insert(std::make_pair(obj, hidden));
                if (!hidden || res.second)
                    res.first->second = hidden;
            }

            strings.clear();
        }
    }
}

std::map<App::DocumentObject*,bool> Expression::getDepObjects(std::vector<std::string> *labels)  const {
    std::map<App::DocumentObject*,bool> deps;
    getDepObjects(deps,labels);
    return deps;
}

class GetIdentifiersExpressionVisitor : public ExpressionVisitor {
public:
    explicit GetIdentifiersExpressionVisitor(std::map<App::ObjectIdentifier,bool> &deps)
        :deps(deps)
    {}

    void visit(Expression &e) override {
        this->getIdentifiers(e,deps);
    }

    std::map<App::ObjectIdentifier,bool> &deps;
};

void Expression::getIdentifiers(std::map<App::ObjectIdentifier,bool> &deps)  const {
    GetIdentifiersExpressionVisitor v(deps);
    const_cast<Expression*>(this)->visit(v);
}

std::map<App::ObjectIdentifier,bool> Expression::getIdentifiers()  const {
    std::map<App::ObjectIdentifier,bool> deps;
    getIdentifiers(deps);
    return deps;
}

class AdjustLinksExpressionVisitor : public ExpressionVisitor {
public:
    explicit AdjustLinksExpressionVisitor(const std::set<App::DocumentObject*> &inList)
        :inList(inList),res(false)
    {}

    void visit(Expression &e) override {
        if(this->adjustLinks(e,inList))
            res = true;
    }

    const std::set<App::DocumentObject*> &inList;
    bool res;
};

bool Expression::adjustLinks(const std::set<App::DocumentObject*> &inList) {
    AdjustLinksExpressionVisitor v(inList);
    visit(v);
    return v.res;
}

class ImportSubNamesExpressionVisitor : public ExpressionVisitor {
public:
    explicit ImportSubNamesExpressionVisitor(const ObjectIdentifier::SubNameMap &subNameMap)
        :subNameMap(subNameMap)
    {}

    void visit(Expression &e) override {
        this->importSubNames(e,subNameMap);
    }

    const ObjectIdentifier::SubNameMap &subNameMap;
};

ExpressionPtr Expression::importSubNames(const std::map<std::string,std::string> &nameMap) const {
    if(!owner || !owner->getDocument())
        return nullptr;
    ObjectIdentifier::SubNameMap subNameMap;
    for(auto &dep : getDeps(DepAll)) {
        for(auto &info : dep.second) {
            for(auto &path : info.second) {
                auto obj = path.getDocumentObject();
                if(!obj)
                    continue;
                auto it = nameMap.find(obj->getExportName(true));
                if(it!=nameMap.end())
                    subNameMap.emplace(std::make_pair(obj,std::string()),it->second);
                auto key = std::make_pair(obj,path.getSubObjectName());
                if(key.second.empty() || subNameMap.count(key))
                    continue;
                std::string imported = PropertyLinkBase::tryImportSubName(
                               obj,key.second.c_str(),owner->getDocument(), nameMap);
                if(!imported.empty())
                    subNameMap.emplace(std::move(key),std::move(imported));
            }
        }
    }
    if(subNameMap.empty())
        return nullptr;
    ImportSubNamesExpressionVisitor v(subNameMap);
    auto res = copy();
    res->visit(v);
    return res;
}

class UpdateLabelExpressionVisitor : public ExpressionVisitor {
public:
    UpdateLabelExpressionVisitor(App::DocumentObject *obj, const std::string &ref, const char *newLabel)
        :obj(obj),ref(ref),newLabel(newLabel)
    {}

    void visit(Expression &e) override {
        this->updateLabelReference(e,obj,ref,newLabel);
    }

    App::DocumentObject *obj;
    const std::string &ref;
    const char *newLabel;
};

ExpressionPtr Expression::updateLabelReference(
        App::DocumentObject *obj, const std::string &ref, const char *newLabel) const
{
    if(ref.size()<=2)
        return ExpressionPtr();
    std::vector<std::string> labels;
    for(auto &v : getIdentifiers())
        v.first.getDepLabels(labels);
    for(auto &label : labels) {
        // ref contains something like $label. and we need to strip '$' and '.'
        if(ref.compare(1,ref.size()-2,label)==0) {
            UpdateLabelExpressionVisitor v(obj,ref,newLabel);
            auto expr = copy();
            expr->visit(v);
            return expr;
        }
    }
    return ExpressionPtr();
}

class ReplaceObjectExpressionVisitor : public ExpressionVisitor {
public:
    ReplaceObjectExpressionVisitor(const DocumentObject *parent,
            DocumentObject *oldObj, DocumentObject *newObj)
        : parent(parent),oldObj(oldObj),newObj(newObj)
    {
    }

    void visit(Expression &e) override {
        if(collect)
            this->collectReplacement(e,paths,parent,oldObj,newObj);
        else
            this->renameObjectIdentifier(e,paths,dummy);
    }

    const DocumentObject *parent;
    DocumentObject *oldObj;
    DocumentObject *newObj;
    ObjectIdentifier dummy;
    std::map<ObjectIdentifier, ObjectIdentifier> paths;
    bool collect = true;
};

ExpressionPtr Expression::replaceObject(const DocumentObject *parent,
        DocumentObject *oldObj, DocumentObject *newObj) const
{
    ReplaceObjectExpressionVisitor v(parent,oldObj,newObj);

    // First pass, collect any changes. We have to const_cast it, as visit() is
    // not const. This is ugly...
    const_cast<Expression*>(this)->visit(v);

    if(v.paths.empty())
        return ExpressionPtr();

    // Now make a copy and do the actual replacement
    auto expr = copy();
    v.collect = false;
    expr->visit(v);
    return expr;
}

App::any Expression::getValueAsAny() const {
    Base::PyGILStateLocker lock;
    return pyObjectToAny(getPyValue());
}

Py::Object Expression::getPyValue() const {
    try {
        Py::Object pyobj = _getPyValue();
        if(!components.empty()) {
            for(auto &c : components)
                pyobj = c->get(this,pyobj);
        }
        return pyobj;
    }catch(Py::Exception &) {
        EXPR_PY_THROW(this);
    }
    return Py::Object();
}

void Expression::addComponent(ExpressionComponentRef&& component) {
    assert(component);
    components.emplace_back(std::move(component));
}

void Expression::visit(ExpressionVisitor &v) {
    _visit(v);
    for(auto &c : components)
        c->visit(v);
    v.visit(*this);
}

Expression* Expression::eval() const {
    Base::PyGILStateLocker lock;
    return expressionFromPy(owner,getPyValue());
}

bool Expression::isSame(const Expression &other, bool checkComment) const {
    if(&other == this)
        return true;
    if(getTypeId()!=other.getTypeId())
        return false;
    return (!checkComment || comment==other.comment)
        && toString(true,true) == other.toString(true,true);
}

std::string Expression::toString(bool persistent, bool checkPriority, int indent) const {
    std::ostringstream ss;
    toString(ss,persistent,checkPriority,indent);
    return ss.str();
}

void Expression::toString(std::ostream &ss, bool persistent, bool checkPriority, int indent) const {
    if(components.empty()) {
        bool needsParens = checkPriority && priority()<20;
        if(needsParens)
            ss << '(';
        _toString(ss,persistent,indent);
        if(needsParens)
            ss << ')';
        return;
    }
    if(!_isIndexable()) {
        ss << '(';
        _toString(ss,persistent,indent);
        ss << ')';
    }else
        _toString(ss,persistent,indent);
    for(auto &c : components)
        c->toString(ss,persistent);
}

std::unique_ptr<App::Expression> Expression::copy() const {
    auto expr = _copy();
    copy_vector(expr->components,components);
    expr->comment = comment;
    return expr;
}

#if defined(__clang__)
# pragma clang diagnostic pop
#endif
