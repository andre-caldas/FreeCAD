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

#ifndef NAMEDSKETCHERGUI_VIEWPROVIDERNAMEDSKETCH_H
#define NAMEDSKETCHERGUI_VIEWPROVIDERNAMEDSKETCH_H

#include <Mod/Part/Gui/ViewProvider2DObject.h>

#include "NamedSketcherGlobal.h"

namespace NamedSketcherGui
{

/** @brief The NamedSketch ViewProvider for non-edit mode.
 *  Just for double-clicks.
 */
class NamedSketcherGuiExport ViewProviderNamedSketch
    : public PartGui::ViewProvider2DObject
{
    PROPERTY_HEADER_WITH_OVERRIDE(NamedSketcherGui::ViewProviderNamedSketch);
private:
    ViewProviderNamedSketch();

    bool doubleClicked(void) override;
    bool setEdit(int ModNum) override;
    void unsetEdit(int ModNum) override;
};

}// namespace NamedSketcherGui

#endif// NAMEDSKETCHERGUI_VIEWPROVIDERNAMEDSKETCH_H
