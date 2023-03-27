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
# include <cassert>
//#endif

#include <boost/algorithm/string/predicate.hpp>

#include <Base/Console.h>
#include <App/DocumentObjectPy.h>
#include <Base/GeometryPyCXX.h>
#include <Base/Tools.h>
#include <Base/Interpreter.h>
#include <Base/QuantityPy.h>
#include <Base/Console.h>
#include <Base/Reader.h>
#include <CXX/Objects.hxx>

#include "ObjectIdentifier.h"
#include "String.h"
#include "DocumentMapper.h"
#include "../Application.h"
#include "../Document.h"
#include "../ExpressionParser.h"
#include "../Link.h"
#include "../Property.h"


FC_LOG_LEVEL_INIT("ObjectPath",true,true)

using namespace App;
using namespace Base;

/**
 * @brief Construct an ObjectIdentifier object, given an owner and a single-value property.
 * @param _owner Owner of property.
 * @param property Name of property.
 */

ObjectIdentifier::ObjectIdentifier(const App::PropertyContainer * _owner,
        const std::string & property, int index)
    : owner(nullptr)
    , documentNameSet(false)
    , documentObjectNameSet(false)
    , localProperty(false)
    , _hash(0)
{
    if (_owner) {
        const DocumentObject * docObj = freecad_dynamic_cast<const DocumentObject>(_owner);
        if (!docObj)
            FC_THROWM(Base::RuntimeError,"Property must be owned by a document object.");
        owner = const_cast<DocumentObject*>(docObj);

        if (!property.empty()) {
            setDocumentObjectName(docObj);
        }
    }
    if (!property.empty()) {
        addComponent(SimpleComponentVar(property));
        if(index!=INT_MAX)
            addComponent(ArrayComponentVar(index));
    }
}

ObjectIdentifier::ObjectIdentifier(const App::PropertyContainer * _owner, bool localProperty)
    : owner(nullptr)
    , documentNameSet(false)
    , documentObjectNameSet(false)
    , localProperty(localProperty)
    , _hash(0)
{
    if (_owner) {
        const DocumentObject * docObj = freecad_dynamic_cast<const DocumentObject>(_owner);
        if (!docObj)
            FC_THROWM(Base::RuntimeError,"Property must be owned by a document object.");
        owner = const_cast<DocumentObject*>(docObj);
    }
}

/**
 * @brief Construct an ObjectIdentifier object given a property. The property is assumed to be single-valued.
 * @param prop Property to construct object identifier for.
 */

ObjectIdentifier::ObjectIdentifier(const Property &prop, int index)
    : owner(nullptr)
    , documentNameSet(false)
    , documentObjectNameSet(false)
    , localProperty(false)
    , _hash(0)
{
    DocumentObject * docObj = freecad_dynamic_cast<DocumentObject>(prop.getContainer());

    if (!docObj)
        FC_THROWM(Base::TypeError, "Property must be owned by a document object.");
    if (!prop.hasName())
        FC_THROWM(Base::RuntimeError, "Property must have a name.");

    owner = const_cast<DocumentObject*>(docObj);

    setDocumentObjectName(docObj);

    addComponent(SimpleComponentVar(String(prop.getName())));
    if(index!=INT_MAX)
        addComponent(ArrayComponentVar(index));
}

/**
 * @brief Get the name of the property.
 * @return Name
 */

std::string ObjectIdentifier::getPropertyName() const
{
    ResolveResults result(*this);

    assert(result.propertyIndex >=0 && static_cast<std::size_t>(result.propertyIndex) < components.size());

    return components.at(result.propertyIndex)->getName();
}

/**
 * @brief Get Component at given index \a i.
 * @param i: Index to get
 * @param idx: optional return of adjusted component index
 * @return A component.
 */

const ObjectPath::Component &ObjectIdentifier::getPropertyComponent(int i, int *idx) const
{
    ResolveResults result(*this);

    i += result.propertyIndex;
    if (idx)
        *idx = i;

    return *(components.at(i));
}

template<typename CompRef>
void App::ObjectIdentifier::setComponent(int idx, CompRef &&comp)
{
    components.at(idx) = std::move<CompRef>(comp);
    _cache.clear();
}

std::vector<ObjectIdentifier::ComponentRef> ObjectIdentifier::getPropertyComponents() const {
    if(components.size()<=1 || documentObjectName.getString().empty())
        return components;
    ResolveResults result(*this);
    if(result.propertyIndex==0)
        return components;
    std::vector<ObjectIdentifier::ComponentRef> res;
    res.insert(res.end(),components.begin()+result.propertyIndex,components.end());
    return res;
}

/**
 * @brief Compare object identifier with \a other.
 * @param other Other object identifier.
 * @return true if they are equal.
 */

bool ObjectIdentifier::operator ==(const ObjectIdentifier &other) const
{
    return owner==other.owner && toString() == other.toString();
}

/**
 * @brief Compare object identifier with \a other.
 * @param other Other object identifier
 * @return true if they differ from each other.
 */

bool ObjectIdentifier::operator !=(const ObjectIdentifier &other) const
{
    return !(operator==)(other);
}

/**
 * @brief Compare object identifier with other.
 * @param other Other object identifier.
 * @return true if this object is less than the other.
 */

bool ObjectIdentifier::operator <(const ObjectIdentifier &other) const
{
    if(owner < other.owner)
        return true;
    if(owner > other.owner)
        return false;
    return toString() < other.toString();
}

/**
 * @brief Return number of components.
 * @return Number of components in this identifier.
 */

int ObjectIdentifier::numComponents() const
{
    return components.size();
}

/**
 * @brief Compute number of sub components, i.e excluding the property.
 * @return Number of components.
 */

int ObjectIdentifier::numSubComponents() const
{
    ResolveResults result(*this);

    return components.size() - result.propertyIndex;
}

bool ObjectIdentifier::verify(const App::Property &prop, bool silent) const {
    ResolveResults result(*this);
    if(components.size() - result.propertyIndex != 1) {
        if(silent)
            return false;
        FC_THROWM(Base::ValueError,"Invalid property path: single component expected");
    }
    if(!components.at(result.propertyIndex)->isSimple()) {
        if(silent)
            return false;
        FC_THROWM(Base::ValueError,"Invalid property path: simple component expected");
    }
    const std::string &name = components.at(result.propertyIndex)->getName();
    CellAddress addr;
    bool isAddress = addr.parseAbsoluteAddress(name.c_str());
    if((isAddress && addr.toString(CellAddress::Cell::ShowRowColumn) != prop.getName()) ||
       (!isAddress && name!=prop.getName()))
    {
        if(silent)
            return false;
        FC_THROWM(Base::ValueError,"Invalid property path: name mismatch");
    }
    return true;
}

/**
 * @brief Create a string representation of this object identifier.
 *
 * An identifier is written as document#documentobject.property.subproperty1...subpropertyN
 * document# may be dropped; it is assumed to be within owner's document. If documentobject is dropped,
 * the property is assumed to be owned by the owner specified in the object identifiers constructor.
 *
 * @return A string
 */

const std::string &ObjectIdentifier::toString() const
{
    if(!_cache.empty() || !owner)
        return _cache;

    std::ostringstream s;
    ResolveResults result(*this);

    if(result.propertyIndex >= (int)components.size())
        return _cache;

    if(localProperty ||
       (result.resolvedProperty &&
        result.resolvedDocumentObject==owner &&
        components.size()>1 &&
        components.at(1)->isSimple() &&
        result.propertyIndex==0))
    {
        s << '.';
    }else if (documentNameSet && !documentName.getString().empty()) {
        if(documentObjectNameSet && !documentObjectName.getString().empty())
            s << documentName.toString() << "#"
              << documentObjectName.toString() << '.';
        else if(!result.resolvedDocumentObjectName.getString().empty())
            s << documentName.toString() << "#"
              << result.resolvedDocumentObjectName.toString() << '.';
    } else if (documentObjectNameSet && !documentObjectName.getString().empty()) {
        s << documentObjectName.toString() << '.';
    } else if (result.propertyIndex > 0) {
        components.at(0)->toString(s);
        s << '.';
    }

    if(!subObjectName.getString().empty())
        s << subObjectName.toString() << '.';

    s << components.at(result.propertyIndex)->getName();
    getSubPathStr(s,result);
    const_cast<ObjectIdentifier*>(this)->_cache = s.str();
    return _cache;
}

std::string ObjectIdentifier::toPersistentString() const {

    if(!owner)
        return std::string();

    std::ostringstream s;
    ResolveResults result(*this);

    if(result.propertyIndex >= (int)components.size())
        return std::string();

    if(localProperty ||
       (result.resolvedProperty &&
        result.resolvedDocumentObject==owner &&
        components.size()>1 &&
        components.at(1)->isSimple() &&
        result.propertyIndex==0))
    {
        s << '.';
    }else if(result.resolvedDocumentObject &&
        result.resolvedDocumentObject!=owner &&
        result.resolvedDocumentObject->isExporting())
    {
        s << result.resolvedDocumentObject->getExportName(true);
        if(documentObjectName.isRealString())
            s << '@';
        s << '.';
    } else if (documentNameSet && !documentName.getString().empty()) {
        if(documentObjectNameSet && !documentObjectName.getString().empty())
            s << documentName.toString() << "#"
                << documentObjectName.toString() << '.';
        else if(!result.resolvedDocumentObjectName.getString().empty())
            s << documentName.toString() << "#"
                << result.resolvedDocumentObjectName.toString() << '.';
    } else if (documentObjectNameSet && !documentObjectName.getString().empty()) {
        s << documentObjectName.toString() << '.';
    } else if (result.propertyIndex > 0) {
        components.at(0)->toString(s);
        s << '.';
    }

    if(!subObjectName.getString().empty()) {
        const char *subname = subObjectName.getString().c_str();
        std::string exportName;
        s << String(PropertyLinkBase::exportSubName(exportName,
                        result.resolvedDocumentObject,subname),true).toString() << '.';
    }

    s << components.at(result.propertyIndex)->getName();
    getSubPathStr(s,result);
    return s.str();
}

std::size_t ObjectIdentifier::hash() const
{
    if(_hash && !_cache.empty())
        return _hash;
    const_cast<ObjectIdentifier*>(this)->_hash = boost::hash_value(toString());
    return _hash;
}

bool ObjectIdentifier::replaceObject(ObjectIdentifier &res, const App::DocumentObject *parent,
            App::DocumentObject *oldObj, App::DocumentObject *newObj) const
{
    ResolveResults result(*this);

    if(!result.resolvedDocumentObject)
        return false;

    auto r = PropertyLinkBase::tryReplaceLink(owner, result.resolvedDocumentObject,
            parent, oldObj, newObj, subObjectName.getString().c_str());

    if(!r.first)
        return false;

    res = *this;
    if(r.first != result.resolvedDocumentObject) {
        if(r.first->getDocument()!=owner->getDocument()) {
            auto doc = r.first->getDocument();
            bool useLabel = res.documentName.isRealString();
            const char *name = useLabel?doc->Label.getValue():doc->getName();
            res.setDocumentName(String(name, useLabel), true);
        }
        if(documentObjectName.isRealString())
            res.documentObjectName = String(r.first->Label.getValue(),true);
        else
            res.documentObjectName = String(r.first->getNameInDocument(),false,true);
    }
    res.subObjectName = String(r.second,true);
    res._cache.clear();
    res.shadowSub.first.clear();
    res.shadowSub.second.clear();
    return true;
}

/**
 * @brief Escape toString representation so it is suitable for being embedded in a python command.
 * @return Escaped string.
 */

std::string ObjectIdentifier::toEscapedString() const
{
    return Base::Tools::escapedUnicodeFromUtf8(toString().c_str());
}

bool ObjectIdentifier::updateLabelReference(
        App::DocumentObject *obj, const std::string &ref, const char *newLabel)
{
    if(!owner)
        return false;

    ResolveResults result(*this);

    if(!subObjectName.getString().empty() && result.resolvedDocumentObject) {
        std::string sub = PropertyLinkBase::updateLabelReference(
                result.resolvedDocumentObject, subObjectName.getString().c_str(), obj,ref,newLabel);
        if(!sub.empty()) {
            subObjectName = String(sub,true);
            _cache.clear();
            return true;
        }
    }

    if(result.resolvedDocument != obj->getDocument())
        return false;

    if(!documentObjectName.getString().empty()) {
        if(documentObjectName.isForceIdentifier())
            return false;

        if(!documentObjectName.isRealString() &&
           documentObjectName.getString()==obj->getNameInDocument())
            return false;

        if(documentObjectName.getString()!=obj->Label.getValue())
            return false;

        documentObjectName = ObjectPath::String(newLabel, true);

        _cache.clear();
        return true;
    }

    if (result.resolvedDocumentObject==obj &&
        result.propertyIndex == 1 &&
        result.resolvedDocumentObjectName.isRealString() &&
        result.resolvedDocumentObjectName.getString()==obj->Label.getValue())
    {
        components.at(0)->setName(ObjectPath::String(newLabel, true));
        _cache.clear();
        return true;
    }

    // If object identifier uses the label then resolving the document object will fail.
    // So, it must be checked if using the new label will succeed
    if (components.size()>1 && components.at(0)->getName()==obj->Label.getValue()) {
        ObjectIdentifier id(*this);
        id.components.at(0)->name.str = newLabel;

        ResolveResults result(id);

        if (result.propertyIndex == 1 && result.resolvedDocumentObject == obj) {
            components.at(0)->name = id.components.at(0)->name;
            _cache.clear();
            return true;
        }
    }

    return false;
}

bool ObjectIdentifier::relabeledDocument(ExpressionVisitor &v,
        const std::string &oldLabel, const std::string &newLabel)
{
    if (documentNameSet && documentName.isRealString() && documentName.getString()==oldLabel) {
        v.aboutToChange();
        documentName = String(newLabel,true);
        _cache.clear();
        return true;
    }
    return false;
}

/**
 * @brief Get sub field part of a property as a string.
 * @return String representation of path.
 */

void ObjectIdentifier::getSubPathStr(std::ostream &s, const ResolveResults &result, bool toPython) const
{
    std::vector<Component>::const_iterator i = components.begin() + result.propertyIndex + 1;
    while (i != components.end()) {
        if(i->isSimple())
            s << '.';
        i->toString(s,toPython);
        ++i;
    }
}

std::string ObjectIdentifier::getSubPathStr(bool toPython) const {
    std::ostringstream ss;
    getSubPathStr(ss,ResolveResults(*this),toPython);
    return ss.str();
}


enum ResolveFlags {
    ResolveByIdentifier,
    ResolveByLabel,
    ResolveAmbiguous,
};

/**
 * @brief Resolve the object identifier to a concrete document, documentobject, and property.
 *
 * This method is a helper method that fills out data in the given ResolveResults object.
 *
 */

void ObjectIdentifier::resolve(ResolveResults &results) const
{
    if(!owner)
        return;

    bool docAmbiguous = false;

    /* Document name specified? */
    if (!documentName.getString().empty()) {
        results.resolvedDocument = getDocument(documentName,&docAmbiguous);
        results.resolvedDocumentName = documentName;
    }
    else {
        results.resolvedDocument = owner->getDocument();
        results.resolvedDocumentName = String(results.resolvedDocument->getName(), false, true);
    }

    results.subObjectName = subObjectName;
    results.propertyName = "";
    results.propertyIndex = 0;

    // Assume document name and object name from owner if not found
    if (!results.resolvedDocument) {
        if (!documentName.getString().empty()) {
            if(docAmbiguous)
                results.flags.set(ResolveAmbiguous);
            return;
        }

        results.resolvedDocument = owner->getDocument();
        if (!results.resolvedDocument)
            return;
    }

    results.resolvedDocumentName = String(results.resolvedDocument->getName(), false, true);

    /* Document object name specified? */
    if (!documentObjectName.getString().empty()) {
        results.resolvedDocumentObjectName = documentObjectName;
        results.resolvedDocumentObject = DocumentPath::getDocumentObject(
                results.resolvedDocument, documentObjectName, results.flags);
        if (!results.resolvedDocumentObject)
            return;

        if (components.empty())
            return;

        results.propertyName = components.at(0)->name.getString();
        results.propertyIndex = 0;
        results.getProperty( *this );
    }
    else {
        /* Document object name not specified, resolve from path */

        /* One component? */
        if (components.size() == 1 || (components.size()>1 && !components.at(0)->isSimple())) {
            /* Yes -- then this must be a property, so we get the document object's name from the owner */
            results.resolvedDocumentObjectName = String(owner->getNameInDocument(), false, true);
            results.resolvedDocumentObject = owner;
            results.propertyName = components.at(0)->name.getString();
            results.propertyIndex = 0;
            results.getProperty(*this);
        }
        else if (components.size() >= 2) {
            /* No --  */
            if (!components.at(0)->isSimple())
                return;

            results.resolvedDocumentObject = getDocumentObject(
                    results.resolvedDocument, components.at(0)->name, results.flags);

            /* Possible to resolve component to a document object? */
            if (results.resolvedDocumentObject) {
                /* Yes */
                results.resolvedDocumentObjectName = String {
                        components.at(0)->name.getString(),
                        false,
                        results.flags.test(ResolveByIdentifier)};
                results.propertyName = components.at(1)->name.getString();
                results.propertyIndex = 1;
                results.getProperty(*this);
                if(!results.resolvedProperty) {
                    // If the second component is not a property name, try to
                    // interpret the first component as the property name.
                    DocumentObject *sobj = nullptr;
                    results.resolvedProperty = resolveProperty(
                            owner,
                            components.at(0)->name.toString().c_str(),
                            sobj,
                            results.propertyType);
                    if(results.resolvedProperty) {
                        results.propertyName = components.at(0)->name.getString();
                        results.resolvedDocument = owner->getDocument();
                        results.resolvedDocumentName = String(results.resolvedDocument->getName(), false, true);
                        results.resolvedDocumentObjectName = String(owner->getNameInDocument(), false, true);
                        results.resolvedDocumentObject = owner;
                        results.resolvedSubObject = sobj;
                        results.propertyIndex = 0;
                    }
                }
            }
            else if (documentName.getString().empty()) {
                /* No, assume component is a property, and get document object's name from owner */
                results.resolvedDocument = owner->getDocument();
                results.resolvedDocumentName = String(results.resolvedDocument->getName(), false, true);
                results.resolvedDocumentObjectName = String(owner->getNameInDocument(), false, true);
                results.resolvedDocumentObject = owner->getDocument()->getObject(owner->getNameInDocument());
                results.propertyIndex = 0;
                results.propertyName = components.at(results.propertyIndex)->name.getString();
                results.getProperty(*this);
            }
        }
        else
            return;
    }
}

/**
 * @brief Find a document with the given name.
 * @param name Name of document
 * @return Pointer to document, or 0 if it is not found or not uniquely defined by name.
 */

Document * ObjectIdentifier::getDocument(String name, bool *ambiguous) const
{
    if (name.getString().empty())
        name = getDocumentName();

    App::Document * docById = nullptr;

    if(!name.isRealString()) {
        docById = App::GetApplication().getDocument(name.toString().c_str());
        if (name.isForceIdentifier())
            return docById;
    }

    App::Document * docByLabel = nullptr;
    const std::vector<App::Document*> docs = App::GetApplication().getDocuments();

    for (std::vector<App::Document*>::const_iterator i = docs.begin(); i != docs.end(); ++i) {
        if ((*i)->Label.getValue() == name.getString()) {
            /* Multiple hits for same label? */
            if (docByLabel) {
                if(ambiguous) *ambiguous = true;
                return nullptr;
            }
            docByLabel = *i;
        }
    }

    /* Not found on id? */
    if (!docById)
        return docByLabel; // Either not found at all, or on label
    else {
        /* Not found on label? */
        if (!docByLabel) /* Then return doc by id */
            return docById;

        /* docByLabel and docById could be equal; that is ok */
        if (docByLabel == docById)
            return docById;
        if (ambiguous)
            *ambiguous = true;
        return nullptr;
    }
}

/**
 * @brief Get the document object for the object identifier.
 * @return Pointer to document object, or 0 if not found or uniquely defined.
 */

DocumentObject *ObjectIdentifier::getDocumentObject() const
{
    const App::Document * doc = getDocument();
    std::bitset<32> dummy;

    if (!doc)
        return nullptr;

    ResolveResults result(*this);

    return ObjectPath::getDocumentObject(doc, result.resolvedDocumentObjectName, dummy);
}


enum PseudoPropertyType {
    PseudoNone,
    PseudoShape,
    PseudoPlacement,
    PseudoMatrix,
    PseudoLinkPlacement,
    PseudoLinkMatrix,
    PseudoSelf,
    PseudoApp,
    PseudoPart,
    PseudoRegex,
    PseudoBuiltins,
    PseudoMath,
    PseudoCollections,
    PseudoGui,
    PseudoCadquery,
};

void ObjectIdentifier::getDepLabels(std::vector<std::string> &labels) const {
    getDepLabels(ResolveResults(*this),labels);
}

void ObjectIdentifier::getDepLabels(
        const ResolveResults &result, std::vector<std::string> &labels) const
{
    if(!documentObjectName.getString().empty()) {
        if(documentObjectName.isRealString())
            labels.push_back(documentObjectName.getString());
    } else if(result.propertyIndex == 1)
        labels.push_back(components.at(0)->name.getString());
    if(!subObjectName.getString().empty())
        PropertyLinkBase::getLabelReferences(labels,subObjectName.getString().c_str());
}

ObjectIdentifier::Dependencies
ObjectIdentifier::getDep(bool needProps, std::vector<std::string> *labels) const
{
    Dependencies deps;
    getDep(deps,needProps,labels);
    return deps;
}

void ObjectIdentifier::getDep(Dependencies &deps, bool needProps, std::vector<std::string> *labels) const
{
    ResolveResults result(*this);
    if(labels)
        getDepLabels(result,*labels);

    if(!result.resolvedDocumentObject)
       return;

    if(!needProps) {
        deps[result.resolvedDocumentObject];
        return;
    }

    if(!result.resolvedProperty) {
        if(!result.propertyName.empty())
            deps[result.resolvedDocumentObject].insert(result.propertyName);
        return;
    }

    Base::PyGILStateLocker lock;
    try {
        access(result, nullptr, &deps);
    }
    catch (Py::Exception& e) {
        e.clear();
    }
    catch (Base::Exception &) {
    }
}

/**
 * @brief Get components as a string list.
 * @return List of strings.
 */

std::vector<std::string> ObjectIdentifier::getStringList() const
{
    std::vector<std::string> l;
    ResolveResults result(*this);

    if(!result.resolvedProperty || result.resolvedDocumentObject != owner) {
        if (documentNameSet)
            l.push_back(documentName.toString());

        if (documentObjectNameSet)
            l.push_back(documentObjectName.toString());
    }
    if(!subObjectName.getString().empty()) {
        l.back() += subObjectName.toString();
    }
    std::vector<Component>::const_iterator i = components.begin();
    while (i != components.end()) {
        std::ostringstream ss;
        i->toString(ss);
        l.push_back(ss.str());
        ++i;
    }

    return l;
}

/**
 * @brief Construct the simplest possible object identifier relative to another.
 * @param other The other object identifier.
 * @return A new simplified object identifier.
 */

ObjectIdentifier ObjectIdentifier::relativeTo(const ObjectIdentifier &other) const
{
    ObjectIdentifier result(other.getOwner());
    ResolveResults thisresult(*this);
    ResolveResults otherresult(other);

    if (otherresult.resolvedDocument != thisresult.resolvedDocument)
        result.setDocumentName(std::move(thisresult.resolvedDocumentName), true);
    if (otherresult.resolvedDocumentObject != thisresult.resolvedDocumentObject)
        result.setDocumentObjectName(
                std::move(thisresult.resolvedDocumentObjectName), true, String(subObjectName));

    for (std::size_t i = thisresult.propertyIndex; i < components.size(); ++i)
        result << components.at(i);

    return result;
}

/**
 * @brief Parse a string to create an object identifier.
 *
 * This method throws an exception if the string is invalid.
 *
 * @param docObj Document object that will own this object identifier.
 * @param str String to parse
 * @return A new object identifier.
 */

ObjectIdentifier ObjectIdentifier::parse(const DocumentObject *docObj, const std::string &str)
{
    std::unique_ptr<Expression> expr(ExpressionParser::parse(docObj, str.c_str()));
    VariableExpression * v = freecad_dynamic_cast<VariableExpression>(expr.get());

    if (v)
        return v->getPath();
    else
        FC_THROWM(Base::RuntimeError,"Invalid property specification.");
}

std::string ObjectIdentifier::resolveErrorString() const
{
    ResolveResults result(*this);

    return result.resolveErrorString();
}

/**
 * @brief << operator, used to add a component to the object identifier.
 * @param value Component object
 * @return Reference to itself.
 */

template<typename CompRef>
ObjectIdentifier &ObjectIdentifier::operator <<(CompRef&& value)
{
    components.push_back(std::forward<CompRef>(value));
    _cache.clear();
    return *this;
}


/**
 * @brief Get pointer to property pointed to by this object identifier.
 * @return Point to property if it is uniquely defined, or 0 otherwise.
 */

Property *ObjectIdentifier::getProperty(int *ptype) const
{
    ResolveResults result(*this);
    if(ptype)
        *ptype = result.propertyType;
    return result.resolvedProperty;
}

Property *ObjectIdentifier::resolveProperty(const App::DocumentObject *obj,
        const char *propertyName, App::DocumentObject *&sobj, int &ptype) const
{
    if(obj && !subObjectName.getString().empty()) {
        sobj = obj->getSubObject(subObjectName.toString().c_str());
        obj = sobj;
    }
    if(!obj)
        return nullptr;

    static std::unordered_map<const char*,int,CStringHasher,CStringHasher> _props = {
        {"_shape",PseudoShape},
        {"_pla",PseudoPlacement},
        {"_matrix",PseudoMatrix},
        {"__pla",PseudoLinkPlacement},
        {"__matrix",PseudoLinkMatrix},
        {"_self",PseudoSelf},
        {"_app",PseudoApp},
        {"_part",PseudoPart},
        {"_re",PseudoRegex},
        {"_py", PseudoBuiltins},
        {"_math", PseudoMath},
        {"_coll", PseudoCollections},
        {"_gui",PseudoGui},
        {"_cq",PseudoCadquery},
    };
    auto it = _props.find(propertyName);
    if(it == _props.end())
        ptype = PseudoNone;
    else {
        ptype = it->second;
        if(ptype != PseudoShape &&
           !subObjectName.getString().empty() &&
           !boost::ends_with(subObjectName.getString(),"."))
        {
            return nullptr;
        }
        return &const_cast<App::DocumentObject*>(obj)->Label; //fake the property
    }

    return obj->getPropertyByName(propertyName);
}



/**
 * @brief Create a canonical representation of an object identifier.
 *
 * The main work is actually done by the property's virtual canonicalPath(...) method,
 * which is invoked by this call.
 *
 * @return A new object identifier.
 */

ObjectIdentifier ObjectIdentifier::canonicalPath() const
{
    ObjectIdentifier res(*this);
    ResolveResults result(res);
    if(result.resolvedDocumentObject && result.resolvedDocumentObject!=owner) {
        res.owner = result.resolvedDocumentObject;
        res._cache.clear();
    }
    res.resolveAmbiguity(result);
    if(!result.resolvedProperty || result.propertyType!=PseudoNone)
        return res;
    return result.resolvedProperty->canonicalPath(res);
}

/**
 * @brief Set the document name for this object identifier.
 *
 * If force is true, the document name will always be included in the string representation.
 *
 * @param name Name of document object.
 * @param force Force name to be set
 */

void ObjectIdentifier::setDocumentName(ObjectPath::String &&name, bool force)
{
    if(name.getString().empty())
        force = false;
    documentNameSet = force;
    _cache.clear();
    documentName = DocumentMapper::mapString(name);
    if(documentName.empty())
    {
        documentName = std::move(name);
    }
}

/**
 * @brief Get the document name from this object identifier
 *
 * @return Document name as a String object.
 */

ObjectPath::String ObjectIdentifier::getDocumentName() const
{
    ResolveResults result(*this);

    return result.resolvedDocumentName;
}

/**
 * @brief Set the document object name of this object identifier.
 *
 * If force is true, the document object will not be resolved dynamically from the
 * object identifier's components, but used as given by this method.
 *
 * @param name Name of document object.
 * @param force Force name to be set.
 */

void ObjectIdentifier::setDocumentObjectName(ObjectPath::String &&name, bool force,
        ObjectPath::String &&subname, bool checkImport)
{
    if(checkImport) {
        name.checkImport(owner);
        subname.checkImport(owner,nullptr,&name);
    }

    documentObjectName = std::move(name);
    documentObjectNameSet = force;
    subObjectName = std::move(subname);

    _cache.clear();
}

void ObjectIdentifier::setDocumentObjectName(const App::DocumentObject *obj, bool force,
        ObjectPath::String &&subname, bool checkImport)
{
    if(!owner || !obj || !obj->getNameInDocument() || !obj->getDocument())
        FC_THROWM(Base::RuntimeError,"invalid object");

    if(checkImport)
        subname.checkImport(owner,obj);

    if(obj == owner)
        force = false;
    else
        localProperty = false;
    if(obj->getDocument() == owner->getDocument())
        setDocumentName(String());
    else if(!documentNameSet) {
        if(obj->getDocument() == owner->getDocument())
            setDocumentName(String());
        else {
            documentNameSet = true;
            documentName = String(obj->getDocument()->getName(),false,true);
        }
    }else if(documentName.isRealString())
        documentName = String(obj->getDocument()->Label.getStrValue(),true);
    else
        documentName = String(obj->getDocument()->getName(),false,true);

    documentObjectNameSet = force;
    documentObjectName = String(obj->getNameInDocument(),false,true);
    subObjectName = std::move(subname);

    _cache.clear();
}


/**
 * @brief Get the document object name
 * @return String with name of document object as resolved by object identifier.
 */

ObjectPath::String ObjectIdentifier::getDocumentObjectName() const
{
    ResolveResults result(*this);

    return result.resolvedDocumentObjectName;
}

bool ObjectIdentifier::hasDocumentObjectName(bool forced) const {
    return !documentObjectName.getString().empty() && (!forced || documentObjectNameSet);
}

Py::Object ObjectIdentifier::access(const ResolveResults &result,
        Py::Object *value, Dependencies *deps) const
{
    if(!result.resolvedDocumentObject || !result.resolvedProperty ||
       (!subObjectName.getString().empty() && !result.resolvedSubObject))
    {
        FC_THROWM(Base::RuntimeError, result.resolveErrorString()
           << " in '" << toString() << "'");
    }

    Py::Object pyobj;
    int ptype = result.propertyType;

    // NOTE! We do not keep reference of the imported module, assuming once
    // imported they'll live (because of sys.modules) till the application
    // dies.
#define GET_MODULE(_name) do {\
        static PyObject *pymod;\
        if(!pymod) {\
           pymod = PyImport_ImportModule(#_name);\
            if(!pymod)\
                Base::PyException::ThrowException();\
            else\
                Py_DECREF(pymod);\
        }\
        pyobj = Py::Object(pymod);\
    }while(0)

    size_t idx = result.propertyIndex+1;
    switch(ptype) {
    case PseudoApp:
        GET_MODULE(FreeCAD);
        break;
    case PseudoGui:
        GET_MODULE(FreeCADGui);
        break;
    case PseudoPart:
        GET_MODULE(Part);
        break;
    case PseudoCadquery:
        GET_MODULE(freecad.fc_cadquery);
        break;
    case PseudoRegex:
        GET_MODULE(re);
        break;
    case PseudoBuiltins:
        GET_MODULE(builtins);
        break;
    case PseudoMath:
        GET_MODULE(math);
        break;
    case PseudoCollections:
        GET_MODULE(collections);
        break;
    case PseudoShape: {
        GET_MODULE(Part);
        Py::Callable func(pyobj.getAttr("getShape"));
        Py::Tuple tuple(1);
        tuple.setItem(0,Py::Object(result.resolvedDocumentObject->getPyObject(),true));
        if(result.subObjectName.getString().empty())
            pyobj = func.apply(tuple);
        else{
            Py::Dict dict;
            dict.setItem("subname",Py::String(result.subObjectName.getString()));
            dict.setItem("needSubElement",Py::True());
            pyobj = func.apply(tuple,dict);
        }
        break;
    } default: {
        Base::Matrix4D mat;
        auto obj = result.resolvedDocumentObject;
        switch(ptype) {
        case PseudoPlacement:
        case PseudoMatrix:
        case PseudoLinkPlacement:
        case PseudoLinkMatrix:
            obj->getSubObject(result.subObjectName.getString().c_str(),nullptr,&mat);
            break;
        default:
            break;
        }
        if(result.resolvedSubObject)
            obj = result.resolvedSubObject;
        switch(ptype) {
        case PseudoPlacement:
            pyobj = Py::Placement(Base::Placement(mat));
            break;
        case PseudoMatrix:
            pyobj = Py::Matrix(mat);
            break;
        case PseudoLinkPlacement:
        case PseudoLinkMatrix: {
            auto linked = obj->getLinkedObject(true,&mat,false);
            if(!linked || linked==obj) {
                auto ext = obj->getExtensionByType<App::LinkBaseExtension>(true);
                if(ext)
                    ext->getTrueLinkedObject(true,&mat);
            }
            if(ptype == PseudoLinkPlacement)
                pyobj = Py::Placement(Base::Placement(mat));
            else
                pyobj = Py::Matrix(mat);
            break;
        }
        case PseudoSelf:
            pyobj = Py::Object(obj->getPyObject(),true);
            break;
        default: {
            // NOTE! We cannot directly call Property::getPyObject(), but
            // instead, must obtain the property's python object through
            // DocumentObjectPy::getAttr(). Because, PyObjectBase has internal
            // attribute tracking only if we obtain attribute through
            // getAttr(). Without attribute tracking, we can't do things like
            //
            //      obj.Placement.Base.x = 10.
            //
            // What happens is that the when Python interpreter calls
            //
            //      Base.setAttr('x', 10),
            //
            // PyObjectBase will lookup Base's parent, i.e. Placement, and call
            //
            //      Placement.setAttr('Base', Base),
            //
            // and in turn calls
            //
            //      obj.setAttr('Placement',Placement)
            //
            // The tracking logic is implemented in PyObjectBase::__getattro/__setattro

            auto container = result.resolvedProperty->getContainer();
            if(container
                    && container!=result.resolvedDocumentObject
                    && container!=result.resolvedSubObject)
            {
                if(!container->isDerivedFrom(DocumentObject::getClassTypeId()))
                    FC_WARN("Invalid property container");
                else
                    obj = static_cast<DocumentObject*>(container);
            }
            pyobj = Py::Object(obj->getPyObject(),true);
            idx = result.propertyIndex;
            break;
        }}}
    }

    auto setPropDep = [deps](DocumentObject *obj, Property *prop, const char *propName) {
        if(!deps || !obj)
            return;
        if(prop && prop->getContainer()!=obj) {
            auto linkTouched = Base::freecad_dynamic_cast<PropertyBool>(
                    obj->getPropertyByName("_LinkTouched"));
            if(linkTouched)
                propName = linkTouched->getName();
            else {
                auto propOwner = Base::freecad_dynamic_cast<DocumentObject>(prop->getContainer());
                if(propOwner)
                    obj = propOwner;
                else
                    propName = nullptr;
            }
        }
        auto &propset = (*deps)[obj];
        // inserting a blank name in the propset indicates the dependency is
        // on all properties of the corresponding object.
        if (propset.size() != 1 || !propset.begin()->empty()) {
            if (!propName) {
                propset.clear();
                propset.insert("");
            }
            else {
                propset.insert(propName);
            }
        }
        return;
    };

    App::DocumentObject *lastObj = result.resolvedDocumentObject;
    if(result.resolvedSubObject) {
        setPropDep(lastObj,nullptr,nullptr);
        lastObj = result.resolvedSubObject;
    }
    if(ptype == PseudoNone)
        setPropDep(lastObj, result.resolvedProperty, result.resolvedProperty->getName());
    else
        setPropDep(lastObj,nullptr,nullptr);
    lastObj = nullptr;

    if(components.empty())
        return pyobj;

    size_t count = components.size();
    if(value) --count;
    assert(idx<=count);

    for(;idx<count;++idx)  {
        if(PyObject_TypeCheck(*pyobj, &DocumentObjectPy::Type))
            lastObj = static_cast<DocumentObjectPy*>(*pyobj)->getDocumentObjectPtr();
        else if(lastObj) {
            const char *attr = components.at(idx)->getName().c_str();
            auto prop = lastObj->getPropertyByName(attr);
            if(!prop && !pyobj.hasAttr(attr))
                attr = nullptr;
            setPropDep(lastObj,prop,attr);
            lastObj = nullptr;
        }
        pyobj = components.at(idx)->get(pyobj);
    }
    if(value) {
        components.at(idx)->set(pyobj,*value);
        return Py::Object();
    }
    return pyobj;
}

/**
 * @brief Get the value of the property or field pointed to by this object identifier.
 *
 * All type of objects are supported. Some types are casted to FC native
 * type, including: Int, Float, String, Unicode String, and Quantities. Others
 * are just kept as Python object wrapped by App::any.
 *
 * @param pathValue: if true, calls the property's getPathValue(), which is
 * necessary for Qunatities to work.
 *
 * @return The value of the property or field.
 */

App::any ObjectIdentifier::getValue(bool pathValue, bool *isPseudoProperty) const
{
    ResolveResults rs(*this);

    if(isPseudoProperty) {
        *isPseudoProperty = rs.propertyType!=PseudoNone;
        if(rs.propertyType == PseudoSelf
                && isLocalProperty()
                && rs.propertyIndex+1 < (int)components.size()
                && owner->getPropertyByName(components.at(rs.propertyIndex+1].getName().c_str()))
        {
            *isPseudoProperty = false;
        }
    }

    if(rs.resolvedProperty && rs.propertyType==PseudoNone && pathValue)
        return rs.resolvedProperty->getPathValue(*this);

    Base::PyGILStateLocker lock;
    try {
        return pyObjectToAny(access(rs));
    }catch(Py::Exception &) {
        Base::PyException::ThrowException();
    }
    return App::any();
}

Py::Object ObjectIdentifier::getPyValue(bool pathValue, bool *isPseudoProperty) const
{
    ResolveResults rs(*this);

    if(isPseudoProperty) {
        *isPseudoProperty = rs.propertyType!=PseudoNone;
        if(rs.propertyType == PseudoSelf
                && isLocalProperty()
                && rs.propertyIndex+1 < (int)components.size()
                && owner->getPropertyByName(components.at(rs.propertyIndex+1)->getName().c_str()))
        {
            *isPseudoProperty = false;
        }
    }

    if(rs.resolvedProperty && rs.propertyType==PseudoNone && pathValue) {
        Py::Object res;
        if(rs.resolvedProperty->getPyPathValue(*this,res))
            return res;
    }

    try {
        return access(rs);
    }catch(Py::Exception &) {
        Base::PyException::ThrowException();
    }
    return Py::Object();
}

/**
 * @brief Set value of a property or field pointed to by this object identifier.
 *
 * This method uses Python to do the actual work. and a limited set of types that
 * can be in the App::any variable is supported: Base::Quantity, double,
 * char*, const char*, int, unsigned int, short, unsigned short, char, and unsigned char.
 *
 * @param value Value to set
 */

void ObjectIdentifier::setValue(const App::any &value) const
{
    std::stringstream ss;
    ResolveResults rs(*this);
    if(rs.propertyType)
        FC_THROWM(Base::RuntimeError,"Cannot set pseudo property");

    Base::PyGILStateLocker lock;
    try {
        Py::Object pyvalue = pyObjectFromAny(value);
        access(rs,&pyvalue);
    }catch(Py::Exception &) {
        Base::PyException::ThrowException();
    }
}

const std::string &ObjectIdentifier::getSubObjectName(bool newStyle) const {
    if(newStyle && !shadowSub.first.empty())
        return shadowSub.first;
    if(!shadowSub.second.empty())
        return shadowSub.second;
    return subObjectName.getString();
}

const std::string &ObjectIdentifier::getSubObjectName() const {
    return subObjectName.getString();
}

void ObjectIdentifier::importSubNames(const ObjectIdentifier::SubNameMap &subNameMap)
{
    if(!owner || !owner->getDocument())
        return;
    ResolveResults result(*this);
    auto it = subNameMap.find(std::make_pair(result.resolvedDocumentObject,std::string()));
    if(it!=subNameMap.end()) {
        auto obj = owner->getDocument()->getObject(it->second.c_str());
        if(!obj) {
            FC_ERR("Failed to find import object " << it->second << " from "
                    << result.resolvedDocumentObject->getFullName());
            return;
        }
        documentNameSet = false;
        documentName.str.clear();
        if(documentObjectName.isRealString())
            documentObjectName.str = obj->Label.getValue();
        else
            documentObjectName.str = obj->getNameInDocument();
        _cache.clear();
    }
    if(subObjectName.getString().empty())
        return;
    it = subNameMap.find(std::make_pair(
                result.resolvedDocumentObject,subObjectName.str));
    if(it==subNameMap.end())
        return;
    subObjectName = String(it->second,true);
    _cache.clear();
    shadowSub.first.clear();
    shadowSub.second.clear();
}

bool ObjectIdentifier::updateElementReference(ExpressionVisitor &v,
        App::DocumentObject *feature, bool reverse)
{
    assert(v.getPropertyLink());
    if(subObjectName.getString().empty())
        return false;

    ResolveResults result(*this);
    if(!result.resolvedSubObject)
        return false;
    if(v.getPropertyLink()->_updateElementReference(
            feature,result.resolvedDocumentObject,subObjectName.str,shadowSub,reverse)) {
        _cache.clear();
        v.aboutToChange();
        return true;
    }
    return false;
}

bool ObjectIdentifier::adjustLinks(ExpressionVisitor &v, const std::set<App::DocumentObject *> &inList) {
    ResolveResults result(*this);
    if(!result.resolvedDocumentObject)
        return false;
    if(result.resolvedSubObject) {
        PropertyLinkSub prop;
        prop.setValue(result.resolvedDocumentObject, {subObjectName.getString()});
        if(prop.adjustLink(inList)) {
            v.aboutToChange();
            documentObjectName = String(prop.getValue()->getNameInDocument(),false,true);
            subObjectName = String(prop.getSubValues().front(),true);
            _cache.clear();
            return true;
        }
    }
    return false;
}

bool ObjectIdentifier::isTouched() const {
    try {
        ResolveResults result(*this);
        if(result.resolvedProperty) {
            if(result.propertyType==PseudoNone)
                return result.resolvedProperty->isTouched();
            else
                return result.resolvedDocumentObject->isTouched();
        }
    }catch(...) {}
    return false;
}

void ObjectIdentifier::resolveAmbiguity() {
    if(!owner || !owner->getNameInDocument() || isLocalProperty() ||
       (documentObjectNameSet && !documentObjectName.getString().empty() &&
        (documentObjectName.isRealString() || documentObjectName.isForceIdentifier())))
    {
        return;
    }

    ResolveResults result(*this);
    resolveAmbiguity(result);
}

void ObjectIdentifier::resolveAmbiguity(ResolveResults &result) {

    if(!result.resolvedDocumentObject)
        return;

    if(result.propertyIndex==1)
        components.erase(components.begin());

    String subname = subObjectName;
    if(result.resolvedDocumentObject == owner) {
        setDocumentObjectName(owner,false,std::move(subname));
    }else if(result.flags.test(ResolveByIdentifier))
        setDocumentObjectName(result.resolvedDocumentObject,true,std::move(subname));
    else
        setDocumentObjectName(
                String(result.resolvedDocumentObject->Label.getStrValue(),true,false),true,std::move(subname));

    if(result.resolvedDocumentObject->getDocument() == owner->getDocument())
        setDocumentName(String());
}

/** Construct and initialize a ResolveResults object, given an ObjectIdentifier instance.
 *
 * The constructor will invoke the ObjectIdentifier's resolve() method to initialize the object's data.
 */

ObjectIdentifier::ResolveResults::ResolveResults(const ObjectIdentifier &oi)
    : propertyIndex(0)
    , resolvedDocument(nullptr)
    , resolvedDocumentName()
    , resolvedDocumentObject(nullptr)
    , resolvedDocumentObjectName()
    , resolvedSubObject(nullptr)
    , resolvedProperty(nullptr)
    , propertyName()
    , propertyType(PseudoNone)
{
    oi.resolve(*this);
}

std::string ObjectIdentifier::ResolveResults::resolveErrorString() const
{
    std::ostringstream ss;
    if (!resolvedDocument) {
        if(flags.test(ResolveAmbiguous))
            ss << "Ambiguous document name/label '"
               << resolvedDocumentName.getString() << "'";
        else
            ss << "Document '" << resolvedDocumentName.toString() << "' not found";
    } else if (!resolvedDocumentObject) {
        if(flags.test(ResolveAmbiguous))
            ss << "Ambiguous document object name '"
                << resolvedDocumentObjectName.getString() << "'";
        else
            ss << "Document object '" << resolvedDocumentObjectName.toString()
                << "' not found";
    } else if (!subObjectName.getString().empty() && !resolvedSubObject) {
        ss << "Sub-object '" << resolvedDocumentObjectName.getString()
            << '.' << subObjectName.toString() << "' not found";
    } else if (!resolvedProperty) {
        if(propertyType != PseudoShape &&
           !subObjectName.getString().empty() &&
           !boost::ends_with(subObjectName.getString(),"."))
        {
            ss << "Non geometry subname reference must end with '.'";
        }else
            ss << "Property '" << propertyName << "' not found";
    }

    return ss.str();
}

void ObjectIdentifier::ResolveResults::getProperty(const ObjectIdentifier &oi) {
    resolvedProperty = oi.resolveProperty(
            resolvedDocumentObject,propertyName.c_str(),resolvedSubObject,propertyType);
}

/**
 * @brief Search for the document object given by name in doc.
 *
 * Name might be the internal name or a label. In any case, it must uniquely define
 * the document object.
 *
 * @param doc Document to search
 * @param name Name to search for.
 * @return Pointer to document object if a unique pointer is found, 0 otherwise.
 */

App::DocumentObject * ObjectPath::getDocumentObject(const App::Document * doc,
        const String & name, std::bitset<32> &flags)
{
    DocumentObject * objectById = nullptr;
    DocumentObject * objectByLabel = nullptr;

    if(!name.isRealString()) {
        // No object found with matching label, try using name directly
        objectById = doc->getObject(static_cast<const char*>(name));

        if (objectById) {
            flags.set(ResolveByIdentifier);
            return objectById;
        }
        if(name.isForceIdentifier())
            return nullptr;
    }

    std::vector<DocumentObject*> docObjects = doc->getObjects();
    for (std::vector<DocumentObject*>::iterator j = docObjects.begin(); j != docObjects.end(); ++j) {
        if (strcmp((*j)->Label.getValue(), static_cast<const char*>(name)) == 0) {
            // Found object with matching label
            if (objectByLabel)  {
                FC_WARN("duplicate object label " << doc->getName() << '#' << static_cast<const char*>(name));
                return nullptr;
            }
            objectByLabel = *j;
        }
    }

    if (!objectByLabel && !objectById) // Not found at all
        return nullptr;
    else if (!objectByLabel) { // Found by name
        flags.set(ResolveByIdentifier);
        return objectById;
    }
    else if (!objectById) { // Found by label
        flags.set(ResolveByLabel);
        return objectByLabel;
    }
    else if (objectByLabel == objectById) { // Found by both name and label, same object
        flags.set(ResolveByIdentifier);
        flags.set(ResolveByLabel);
        return objectByLabel;
    }
    else {
        flags.set(ResolveAmbiguous);
        return nullptr; // Found by both name and label, two different objects
    }
}
