/***************************************************************************
 *   Copyright (c) 2019 Werner Mayer <wmayer[at]users.sourceforge.net>     *
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

#include <FCGlobal.h>

#ifndef NAMEDSKETCHER_GLOBAL_H
#define NAMEDSKETCHER_GLOBAL_H


// NamedSketcher
#ifndef NamedSketcherExport
#ifdef NamedSketcher_EXPORTS
#  define NamedSketcherExport      FREECAD_DECL_EXPORT
#else
#  define NamedSketcherExport      FREECAD_DECL_IMPORT
#endif
#endif

// NamedSketcherGui
#ifndef NamedSketcherGuiExport
#ifdef NamedSketcherGui_EXPORTS
#  define NamedSketcherGuiExport   FREECAD_DECL_EXPORT
#else
#  define NamedSketcherGuiExport   FREECAD_DECL_IMPORT
#endif
#endif

#endif //NAMEDSKETCHER_GLOBAL_H
