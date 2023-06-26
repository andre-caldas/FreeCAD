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

#ifndef APP_Accessor_Accessor_H
#define APP_Accessor_Accessor_H

#include <type_traits>
#include <variant>
#include <any>
#include <memory>
#include <string>

#include <FCGlobal.h>

namespace App::Accessor
{

class SimpleAccessor;

/**
 * \brief Virtual base class for accessors.
 * The getters and setters are implemented as visitor (std::visitor) structures.
 */
class AppExport Accessor
{
public:
    using index_key = std::variant<std::monostate, int, float, std::string>;

    struct getAccessor
    {
        const Accessor& self;
        std::unique_ptr<Accessor> operator()(std::monostate);
        std::unique_ptr<Accessor> operator()(int index);
        std::unique_ptr<Accessor> operator()(float index);
        std::unique_ptr<Accessor> operator()(std::string_view key);
    };

    template<typename T>
    struct get
    {
        const Accessor& self;
        T& operator()(std::monostate);
        T& operator()(int index);
        T& operator()(float index);
        T& operator()(std::string_view key);
    };

    /**
     * @brief getSize is used when the array range has implicit bound.
     * @return number of elements in the accessed variable.
     */
    virtual int getSize() {return 0;}

protected:
    virtual std::any _getAny(std::monostate) const;
    virtual std::any _getAny(int index) const;
    virtual std::any _getAny(float index) const {return _getAny((int)index);}
    virtual std::any _getAny(std::string_view key) const;

    virtual std::unique_ptr<Accessor> _getAccessor(std::monostate) const;
    virtual std::unique_ptr<Accessor> _getAccessor(int index) const;
    virtual std::unique_ptr<Accessor> _getAccessor(float index) const {return _getAccessor((int) index);}
    virtual std::unique_ptr<Accessor> _getAccessor(std::string_view key) const;
};


template<typename VarType, typename GuardType = VarType>
class AppExport AccessorVar : public Accessor
{
public:
    AccessorVar(std::shared_ptr<GuardType>& guard, VarType* var) : guard(guard), var(var) {}
    template<typename = std::enable_if_t<std::is_equal<VarType, GuardType>::value>>
    AccessorVar(std::shared_ptr<VarType>& var) : AccessorVar(var, var.get()) {}
    AccessorVar(VarType* var) : var(var) {} // Deprecated.

    using get = get<VarType>;

protected:
    std::any _getAny(std::monostate) const override;

private:
    std::shared_ptr<GuardType> guard;
    VarType* var;
};


/**
 * \brief Virtual base class for map accessors.
 * That is, accessors that represent a list of objects identified by a key.
 * This class has the minimum infrastructure so the \class PathIterator
 * can iterate a path "chain of components".
 * The getters and setters are implemente in subclasses,
 * so we avoid casts.
 */
class AppExport MapAccessor : public Accessor
{
};

} //namespace App::Accessor

#endif // APP_Accessor_Accessor_H
