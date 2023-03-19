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

//#include "PreCompiled.h"

//#ifndef _PreComp_
# include <string>
# include <sstream>
//#endif

#include <Base/Console.h>
#include <Base/Reader.h>
#include <App/Document.h>
#include <App/DocumentObject.h>
#include <App/ExpressionParser.h>
#include <App/PropertyLinks.h>

#include "ObjectIdentifier.h"
#include "String.h"

FC_LOG_LEVEL_INIT("ObjectPath",true,true)

namespace App::ObjectPath {

/**
 * @brief Quote input string according to quoting rules for an expression: because " and ' are
 * used to designate inch and foot units, strings are quoted as <<string>>.
 *
 * @param input
 * @return
 */

std::string quote(const std::string &input, bool toPython)
{
    std::stringstream output;

    std::string::const_iterator cur = input.begin();
    std::string::const_iterator end = input.end();

    output << (toPython?"'":"<<");
    while (cur != end) {
        switch (*cur) {
        case '\t':
            output << "\\t";
            break;
        case '\n':
            output << "\\n";
            break;
        case '\r':
            output << "\\r";
            break;
        case '\\':
            output << "\\\\";
            break;
        case '\'':
            output << "\\'";
            break;
        case '"':
            output << "\\\"";
            break;
        case '>':
            output << (toPython?">":"\\>");
            break;
        default:
            output << *cur;
        }
        ++cur;
    }
    output << (toPython?"'":">>");

    return output.str();
}


/**
 * @brief Get a string representation of this object identifier.
 * @return String representation.
 */

std::string String::toString(bool toPython) const
{
    if (isRealString())
        return quote(str,toPython);
    else
        return str;
}

void String::checkImport(const App::DocumentObject *owner,
        const App::DocumentObject *obj, String *objName)
{
    if(owner && owner->getDocument() && !str.empty() &&
       ExpressionParser::ExpressionImporter::reader()) {
        auto reader = ExpressionParser::ExpressionImporter::reader();
        if (obj || objName) {
            bool restoreLabel = false;
            str = PropertyLinkBase::importSubName(*reader,str.c_str(),restoreLabel);
            if (restoreLabel) {
                if (!obj) {
                    std::bitset<32> flags;
                    obj = ObjectIdentifier::getDocumentObject(owner->getDocument(),*objName,flags);
                    if (!obj) {
                        FC_ERR("Cannot find object " << objName->toString());
                    }
                }

                if (obj) {
                    PropertyLinkBase::restoreLabelReference(obj,str);
                }
            }
        }
        else if (str.back()!='@') {
            str = reader->getName(str.c_str());
        }
        else {
            str.resize(str.size()-1);
            auto mapped = reader->getName(str.c_str());
            auto objForMapped = owner->getDocument()->getObject(mapped);
            if (!objForMapped) {
                FC_ERR("Cannot find object " << str);
            }
            else {
                isString = true;
                forceIdentifier = false;
                str = objForMapped->Label.getValue();
            }
        }
    }
}

} // namespace App::Path
