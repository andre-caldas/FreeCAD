/***************************************************************************
 *   Copyright (c) 2015 Eivind Kvedalen <eivind@kvedalen.name>             *
 *   Copyright (c) 2023 André Caldas <andre.em.caldas@gmail.com>           *
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

//#include "PreCompiled.h"

//#ifndef _PreComp_
# include <cassert>
//#endif

#include <memory>
#include <sstream>

#include <Base/Console.h>
#include <Base/Interpreter.h>

#include "ExpressionParser.h"
#include "ExpressionComponent.h"


FC_LOG_LEVEL_INIT("ObjectPath",true,true)

using namespace App::ExpressionHelper;

Py::Object ExpressionComponent::get(const Expression* owner, const Py::Object& pyobj) const
{
    try {
        return Component::get(pyobj);
    } catch(Py::Exception&) {
        Base::PyException pye(pyobj);
        pye.setMessage(owner?(owner->toString()):"");
        throw pye;
    }
    return Py::Object();
}

void ExpressionComponent::set(const Expression* owner, Py::Object& pyobj, const Py::Object& value) const
{
    try {
        return Component::set(pyobj, value);
    } catch(Py::Exception&) {
        Base::PyException pye(pyobj);
        pye.setMessage(owner?(owner->toString()):"");
        throw pye;
    }
}

void ExpressionComponent::del(Expression* owner, Py::Object& pyobj) const
{
    try {
        Component::del(pyobj);
    } catch(Py::Exception&) {
        Base::PyException pye(pyobj);
        pye.setMessage(owner?(owner->toString()):"");
        throw pye;
    }
}


ExprSimpleComponent::ExprSimpleComponent(std::unique_ptr<StringExpression>&& name_expr)
    : name_expr(std::move(name_expr))
{}

std::unique_ptr<ExpressionComponent> ExprSimpleComponent::copy() const
{
    return std::make_unique<ExprSimpleComponent>(name_expr->copy<StringExpression>());
}

void ExprSimpleComponent::toString(std::ostream &ss, bool toPython) const
{
    ss << '.';
    SimpleComponent::toString(ss, toPython);
}

std::string ExprSimpleComponent::getName() const
{
    return std::string(".") + SimpleComponent::getName();
}

ExprArrayComponent::ExprArrayComponent(std::unique_ptr<NumberExpression>&& index_expr)
    : index_expr(std::move(index_expr))
{}

std::unique_ptr<ExpressionComponent> ExprArrayComponent::copy() const
{
    return std::make_unique<ExprArrayComponent>(index_expr->copy<NumberExpression>());
}

void ExprArrayComponent::toString(std::ostream &ss, bool /*toPython*/) const
{
    ss << '[';
    index_expr->toString(ss);//,persistent);
    ss << ']';
}

void ExprArrayComponent::visit(ExpressionVisitor& v)
{
    if(index_expr) index_expr->visit(v);
}

bool ExprArrayComponent::isTouched() const
{
    return (index_expr && index_expr->isTouched());
}


ExprMapComponent::ExprMapComponent(std::unique_ptr<StringExpression>&& key_expr)
    : key_expr(std::move(key_expr))
{}

std::unique_ptr<ExpressionComponent> ExprMapComponent::copy() const
{
    return std::make_unique<ExprMapComponent>(key_expr->copy<StringExpression>());
}

void ExprMapComponent::toString(std::ostream &ss, bool /*toPython*/) const
{
    ss << '[';
    key_expr->toString(ss);//,persistent);
    ss << ']';
}

void ExprMapComponent::visit(ExpressionVisitor& v)
{
    if(key_expr) key_expr->visit(v);
}

bool ExprMapComponent::isTouched() const
{
    return (key_expr && key_expr->isTouched());
}


ExprRangeComponent::ExprRangeComponent(
        std::unique_ptr<NumberExpression>&& begin_expr,
        std::unique_ptr<NumberExpression>&& end_expr,
        std::unique_ptr<NumberExpression>&& step_expr)
    : begin_expr(std::move(begin_expr))
    , end_expr(std::move(end_expr))
    , step_expr(std::move(step_expr))
{}

ExprRangeComponent::ExprRangeComponent(
        std::unique_ptr<NumberExpression>&& begin_expr,
        std::unique_ptr<NumberExpression>&& end_expr)
    : ExprRangeComponent(std::move(begin_expr),
                         std::move(end_expr),
                         std::make_unique<NumberExpression>(begin_expr->getOwner(),Base::Quantity(1)))
{}

std::unique_ptr<ExpressionComponent> ExprRangeComponent::copy() const
{
    return std::make_unique<ExprRangeComponent>(
                begin_expr->copy<NumberExpression>(),
                end_expr->copy<NumberExpression>(),
                step_expr->copy<NumberExpression>());
}

void ExprRangeComponent::toString(std::ostream &ss, bool /*toPython*/) const
{
    ss << '[';
    begin_expr->toString(ss);//,persistent);
    ss << ':';
    end_expr->toString(ss);//,persistent);
//    if(step_expr) {
        ss << ':';
        step_expr->toString(ss);//,persistent);
//    }
    ss << ']';
}

void ExprRangeComponent::visit(ExpressionVisitor& v)
{
    begin_expr->visit(v);
    end_expr->visit(v);
    step_expr->visit(v);
}

bool ExprRangeComponent::isTouched() const
{
    return begin_expr->isTouched()
            || end_expr->isTouched()
            || step_expr->isTouched();
}
