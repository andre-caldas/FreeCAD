/****************************************************************************
 *   Copyright (c) 2015 Eivind Kvedalen <eivind@kvedalen.name>              *
 *   Copyright (c) 2019 Zheng Lei (realthunder) <realthunder.dev@gmail.com> *
 *   Copyright (c) 2023 André Caldas <andre.em.caldas@gmail.com>            *
 *                                                                          *
 *   This file is part of the FreeCAD CAx development system.               *
 *                                                                          *
 *   This library is free software; you can redistribute it and/or          *
 *   modify it under the terms of the GNU Library General Public            *
 *   License as published by the Free Software Foundation; either           *
 *   version 2 of the License, or (at your option) any later version.       *
 *                                                                          *
 *   This library  is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU Library General Public License for more details.                   *
 *                                                                          *
 *   You should have received a copy of the GNU Library General Public      *
 *   License along with this library; see the file COPYING.LIB. If not,     *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,          *
 *   Suite 330, Boston, MA  02111-1307, USA                                 *
 *                                                                          *
 ****************************************************************************/


#ifndef EXPRESSION_COMPONENT_H
#define EXPRESSION_COMPONENT_H

#include <memory>
#include <FCGlobal.h>

#include "ObjectPath/SimpleComponent.h"
#include "ObjectPath/ArrayComponent.h"
#include "ObjectPath/MapComponent.h"
#include "ObjectPath/RangeComponent.h"

namespace App {
class NumberExpression;
class StringExpression;
class ExpressionVisitor;
class Expression;
}

namespace App::ExpressionHelper {

// We make it virtual public Component because we want to call
// Component::{get,set,del}.
class AppExport ExpressionComponent : virtual public App::ObjectPath::Component
{
public:
    virtual ~ExpressionComponent() = default;

    /**
     * @brief evalAndUpdateuates the expression and substitutes
     * values on the parent class.
     */
    virtual void visit(ExpressionVisitor& v) = 0;
    virtual bool isTouched() const = 0;

    Py::Object get(const Expression* owner, const Py::Object& pyobj) const;
    void set(const Expression* owner, Py::Object& pyobj, const Py::Object& value) const;
    void del(Expression* owner, Py::Object& pyobj) const;

    virtual std::unique_ptr<ExpressionComponent> copy () const = 0;
};

class AppExport ExprSimpleComponent
        : public ExpressionComponent
        , public App::ObjectPath::SimpleComponent
{
public:
    ExprSimpleComponent(std::unique_ptr<StringExpression>&& name_expr);
    std::unique_ptr<ExpressionComponent> copy () const override;

    void toString(std::ostream &ss, bool toPython) const override;
    void visit(ExpressionVisitor&) override {}
    bool isTouched() const override {return false;}

    std::string getName() const override;

private:
    std::unique_ptr<StringExpression> name_expr;
};

class AppExport ExprArrayComponent
        : public ExpressionComponent
        , public App::ObjectPath::ArrayComponent
{
public:
    ExprArrayComponent(std::unique_ptr<NumberExpression>&& index_expr);
    std::unique_ptr<ExpressionComponent> copy () const override;

    void toString(std::ostream &ss, bool persistent) const override;
    void visit(ExpressionVisitor&) override;
    bool isTouched() const override;

    int getIndex() const override;

private:
    std::unique_ptr<NumberExpression> index_expr;
};

class AppExport ExprMapComponent
        : public ExpressionComponent
        , public App::ObjectPath::MapComponent
{
public:
    ExprMapComponent(std::unique_ptr<StringExpression>&& key_expr);
    std::unique_ptr<ExpressionComponent> copy () const override;

    void toString(std::ostream &ss, bool persistent) const override;
    void visit(ExpressionVisitor&) override;
    bool isTouched() const override;

    String getKey() const override;

private:
    std::unique_ptr<StringExpression> key_expr;
};

class AppExport ExprRangeComponent
        : public ExpressionComponent
        , public App::ObjectPath::RangeComponent
{
public:
    ExprRangeComponent(
            std::unique_ptr<NumberExpression>&& begin_expr,
            std::unique_ptr<NumberExpression>&& end_expr,
            std::unique_ptr<NumberExpression>&& step_expr);
    ExprRangeComponent(
            std::unique_ptr<NumberExpression>&& begin_expr,
            std::unique_ptr<NumberExpression>&& end_expr);
    std::unique_ptr<ExpressionComponent> copy () const override;

    void toString(std::ostream &ss, bool persistent) const override;
    void visit(ExpressionVisitor&) override;
    bool isTouched() const override;

    int getBegin() const override;
    int getEnd() const override;
    int getStep() const override;

private:
    std::unique_ptr<NumberExpression> begin_expr;
    std::unique_ptr<NumberExpression> end_expr;
    std::unique_ptr<NumberExpression> step_expr;
};

} // namespace App::ExpressionHelper

#endif //EXPRESSION_COMPONENT_H
