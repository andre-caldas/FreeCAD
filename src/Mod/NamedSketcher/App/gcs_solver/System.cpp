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

#include <iostream>

#include <Base/Console.h>

#include "equations/Equation.h"
#include "equations/Equal.h"
#include "equations/Constant.h"
#include "parameters/ParameterGroupManager.h"
#include "parameters/ParameterGroup.h"
#include "parameters/ParameterValueMapper.h"
//#include "linear_solvers/Ldlt.h"
//#define SOLVER LinearSolvers::Ldlt
//#include "linear_solvers/SimpleSolver.h"
//#define SOLVER LinearSolvers::SimpleSolver
#include "linear_solvers/BiCG.h"
#define SOLVER LinearSolvers::BiCG

#include "System.h"

FC_LOG_LEVEL_INIT("NamedSketch",true,true)

namespace NamedSketcher::GCS
{

void System::addEquation(Equation* equation)
{
    std::cerr << "Adding equation... ";
    gradients.report();
    gradients.pushBack(equation, equation->differentialNonOptimized(shaker));
}

void System::addUserRedundantEquation(Equation* equation)
{
    userRedundantEquations.push_back(equation);
}

void System::removeEquation(Equation* equation)
{
    gradients.remove(equation);
}

int System::checkDependentEquations(const std::vector<Equation*>& equations) const
{
    int result = 0;
    Orthonormalization orthogonalComponents;
    for(auto eq: equations)
    {
        auto remainings = gradients.normalizedOrthogonalComponent(eq->differentialNonOptimized(shaker));
        orthogonalComponents.pushBack(eq, std::move(remainings));
        if(orthogonalComponents.isRedundant(eq))
        {
            ++result;
        }
    }
    return result;
}


void System::updateGradients()
{
    auto equations = gradients.reset();
    for(Equation* eq: equations)
    {
        addEquation(eq);
    }
}

double
System::error2(const ParameterGroupManager& manager) const
{
    // Eigen does not like zero dimensional vectors. :-(
    // I do believe that the norm of a zero dimensional vector is zero.
    // The only element is the zero vector!
    if(manager.inputSize() == 0)
    {
        return 0;
    }
    return minus_error(manager).squaredNorm();
}

System::equation_value_t
System::minus_error(const ParameterGroupManager& manager) const
{
    equation_value_t result(gradients.size());
    for(Equation* eq: gradients)
    {
        if(manager.hasEquation(eq))
        {
            result.insert(manager.getEquationIndex(eq)) = -eq->error(manager);
        }
    }
    return result;
}

void System::optimize()
{
    size_t next_equal = 0;
    size_t next_constant = 0;
    size_t next_linear = 0;

    for(size_t j=0; j < gradients.size(); ++j)
    {
        if(dynamic_cast<Equal*>(gradients[j]))
        {
            if(j != next_equal)
            {
                for(size_t k=j; k != next_equal; --k)
                {
                    gradients.moveForward(k);
                }
            }
            ++next_equal;
            ++next_constant;
            ++next_linear;
            continue;
        }
        if(dynamic_cast<Constant*>(gradients[j]))
        {
            if(j != next_constant)
            {
                for(size_t k=j; k != next_constant; --k)
                {
                    gradients.moveForward(k);
                }
            }
            ++next_constant;
            ++next_linear;
            continue;
        }
        if(gradients[j]->isLinear())
        {
            if(j != next_linear)
            {
                for(size_t k=j; k != next_linear; --k)
                {
                    gradients.moveForward(k);
                }
            }
            ++next_linear;
            continue;
        }
    }
}


bool System::solve() const
{
    ParameterGroupManager manager;
    OptimizedMatrix optimized_gcs;

    const std::vector<Equation*> non_redundant_equations = gradients.getNonRedundants();
    // TODO: (C++20: use range filter, instead)
    std::vector<Equation*> non_linear_equations;
    std::copy_if(non_redundant_equations.begin(),
                 non_redundant_equations.end(),
                 std::back_inserter(non_linear_equations),
                 [](Equation* eq){return !eq->isLinear();});

    // First we set proxies.
    for(Equation* eq: non_redundant_equations)
    {
        eq->declareParameters(manager);
    }

    // Then, we optimize proxies.
    bool shall_set_proxy = true;
    for(int i=0; shall_set_proxy && i < 1000; ++i)
    {
        shall_set_proxy = false;
        for(Equation* eq: non_redundant_equations)
        {
            shall_set_proxy = eq->optimizeParameters(manager) || shall_set_proxy;
        }
    }
    if(shall_set_proxy)
    {
        FC_WARN("Setting proxies through ParameterGroupManager seemed to be in an infinite loop.");
    }
    manager.finishOptimization();

    for(Equation* eq: non_redundant_equations)
    {
        OptimizedVector temp = eq->differentialOptimized(manager);
        if(!temp.isEmpty()) // The OptimizedVector isEmpty() when optimized out.
        {
            manager.addEquation(eq);
            optimized_gcs.addDual(eq, std::move(temp));
        }
    }

    // TODO: Add extra redundants.

    manager.report();

    SOLVER linear_solver(manager, optimized_gcs);

    // TODO: give up criteria.
    // TODO: use the shaker!
    for(int trials=0; trials < 30; ++trials)
    {
manager.report();
        double err2 = error2(manager);
        // TODO: decide on a good criteria.
        if(err2 <= 1e-9)
        {
std::cerr << "Success after " << trials + 1 << " trials." << std::endl;
            manager.commitParameters();
            return true;
        }

        for(Equation* eq: non_linear_equations)
        {
            linear_solver.updateGradient(eq);
        }

        OptimizedVector target = linear_solver.solve();
        bool step_result = false;
        if(!target.isZero())
        {
            double max_step = 1.0;
            for(Equation* eq: non_redundant_equations)
            {
                max_step = std::min(max_step, eq->limitStep(manager, target));
            }
            if(max_step != 1.0)
            {
                target *= max_step;
            }
            step_result = stepIntoTargetDirection(manager, target);
        }
        if(!step_result)
        {
            // Do something! Change solver? Look for other solution?
            // Determine kernel?
            std::cerr << "Failed to step closer to solution." << std::endl;
            break;
        }
    }
manager.commitParameters();

    return false;
}

bool System::stepIntoTargetDirection(
    ParameterGroupManager& manager,
    const OptimizedVector& direction
    ) const
{
    const OptimizedVector current_position = manager.getOptimizedParameterValues();
    std::cerr << "Current position: ";
    manager.print_vector(current_position);std::cerr << std::endl;

    manager.setOptimizedParameterValues(current_position);
    double start_err2 = error2(manager);
    double best_err2 = start_err2;

    std::cerr << "Stepping into direction: ";
    manager.print_vector(direction);
    std::cerr << std::endl;

    // TODO: Criteria for those two magic numbers.
    // N: Should probably depend on the variation of the gradient (Wronskian).
    int N = 16;
    int DEPTH = 4;

    // Search from [current_factor, current_factor + b]
    double increment = 1.0;
    double current_factor = 0.0;
    double best_factor = current_factor;

    for(int count=0; count < DEPTH; ++count)
    {
        increment /= N;
        OptimizedVector next_position;
        while(current_factor < 1.0)
        {
            current_factor += increment;
            next_position.setAsLinearCombination(1.0, current_position, current_factor, direction);
            manager.setOptimizedParameterValues(next_position);
            double err2 = error2(manager);
std::cerr << "COMPUTED ERROR: " << err2 << std::endl;
            if(err2 == 0)
            {
                return true;
            }

            // To avoid "flipping", we do not allow the error
            // to increase.
            // Use position just before the error started increasing.
            if(best_err2 < err2)
            {
                current_factor = std::max(0.0, best_factor - increment);
                break;
            }
            best_err2 = err2;
            best_factor = current_factor;
        }
std::cerr << "New factor: " << current_factor << std::endl;
    }
std::cerr << "Start error: " << start_err2 << ". Best: " << best_err2 << std::endl;
    if(best_err2 == start_err2)
    {
        return false;
    }
    assert(best_factor > 0.0);
    OptimizedVector result;
    result.setAsLinearCombination(1.0, current_position, best_factor, direction);
    manager.setOptimizedParameterValues(std::move(result));
    return true;
}

} // namespace NamedSketcher::GCS
