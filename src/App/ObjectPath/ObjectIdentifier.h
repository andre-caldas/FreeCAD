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


#ifndef APP_ObjectIdentifier_H
#define APP_ObjectIdentifier_H

#include <utility>
#include <bitset>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <memory>
#include <boost/any.hpp>
#include <FCConfig.h>

#include "String.h"


namespace Py {
class Object;
}

namespace App
{

using namespace ObjectPath;

using any = boost::any;

template<class T>
inline const T &any_cast(const boost::any &value) {
    return boost::any_cast<const T&>(value);
}

template<class T>
inline T &any_cast(boost::any &value) {
    return boost::any_cast<T&>(value);
}

class Property;
class Document;
class PropertyContainer;
class DocumentObject;
class ExpressionVisitor;
namespace ObjectPath {
class String;
class Component;
}


class AppExport ObjectIdentifier {

public:
    explicit ObjectIdentifier(const App::PropertyContainer * _owner = nullptr,
            const std::string & property = std::string(), int index=INT_MAX);

    ObjectIdentifier(const App::PropertyContainer * _owner, bool localProperty);

    ObjectIdentifier(const App::Property & prop, int index=INT_MAX);//explicit bombs

    virtual ~ObjectIdentifier() = default;

    App::DocumentObject *getOwner() const { return owner; }

    // Components
    using ComponentRef = std::shared_ptr<Component>;
    template<typename CompRef>
    void addComponent(CompRef &&c) {
        components.push_back(std::forward<CompRef>(c));
        _cache.clear();
    }

    std::string getPropertyName() const;

    const Component & getPropertyComponent(int i, int *idx=nullptr) const;

    template<typename CompRef>
    void setComponent(int idx, CompRef &&comp);

    std::vector<ComponentRef> getPropertyComponents() const;
    const std::vector<ComponentRef> &getComponents() const { return components; }

    std::string getSubPathStr(bool toPython=false) const;

    int numComponents() const;

    int numSubComponents() const;

    const std::string &toString() const;

    std::string toPersistentString() const;

    std::string toEscapedString() const;

    bool isTouched() const;

    App::Property *getProperty(int *ptype=nullptr) const;

    App::ObjectIdentifier canonicalPath() const;

    // Document-centric functions

    void setDocumentName(String &&name, bool force = false);

    String getDocumentName() const;

    void setDocumentObjectName(String &&name, bool force = false,
            String &&subname = String(), bool checkImport=false);

    void setDocumentObjectName(const App::DocumentObject *obj, bool force = false,
            String &&subname = String(), bool checkImport=false);

    bool hasDocumentObjectName(bool forced=false) const;

    bool isLocalProperty() const { return localProperty; }

    String getDocumentObjectName() const;

    const std::string &getSubObjectName(bool newStyle) const;
    const std::string &getSubObjectName() const;

    using SubNameMap = std::map<std::pair<App::DocumentObject*,std::string>,std::string>;
    void importSubNames(const SubNameMap &subNameMap);

    bool updateLabelReference(App::DocumentObject *, const std::string &, const char *);

    bool relabeledDocument(ExpressionVisitor &v, const std::string &oldLabel, const std::string &newLabel);

    /** Type for storing dependency of an ObjectIdentifier
     *
     * The dependency is a map from document object to a set of property names.
     * An object identifier may references multiple objects using syntax like
     * 'Part.Group[0].Width'.
     *
     * Also, we use set of string instead of set of Property pointer, because
     * the property may not exist at the time this ObjectIdentifier is
     * constructed.
     */
    using Dependencies = std::map<App::DocumentObject *, std::set<std::string> >;

    /** Get dependencies of this object identifier
     *
     * @param needProps: whether need property dependencies.
     * @param labels: optional return of any label references.
     *
     * In case of multi-object references, like 'Part.Group[0].Width', if no
     * property dependency is required, then this function will only return the
     * first referred object dependency. Or else, all object and property
     * dependencies will be returned.
     */
    Dependencies getDep(bool needProps, std::vector<std::string> *labels=nullptr) const;

    /** Get dependencies of this object identifier
     *
     * @param deps: returns the dependencies.
     * @param needProps: whether need property dependencies.
     * @param labels: optional return of any label references.
     *
     * In case of multi-object references, like 'Part.Group[0].Width', if no
     * property dependency is required, then this function will only return the
     * first referred object dependency. Or else, all object and property
     * dependencies will be returned.
     */
    void getDep(Dependencies &deps, bool needProps, std::vector<std::string> *labels=nullptr) const;

    /// Returns all label references
    void getDepLabels(std::vector<std::string> &labels) const;

    App::Document *getDocument(String name = String(), bool *ambiguous=nullptr) const;

    App::DocumentObject *getDocumentObject() const;

    std::vector<std::string> getStringList() const;

    App::ObjectIdentifier relativeTo(const App::ObjectIdentifier & other) const;

    bool replaceObject(ObjectIdentifier &res, const App::DocumentObject *parent,
            App::DocumentObject *oldObj, App::DocumentObject *newObj) const;

    // Operators

    template<typename CompRef>
    ObjectIdentifier & operator<<(CompRef&& value);

    bool operator==(const ObjectIdentifier & other) const;

    bool operator!=(const ObjectIdentifier & other) const;

    bool operator<(const ObjectIdentifier &other) const;

    // Getter

    App::any getValue(bool pathValue=false, bool *isPseudoProperty=nullptr) const;

    Py::Object getPyValue(bool pathValue=false, bool *isPseudoProperty=nullptr) const;

    // Setter: is const because it does not alter the object state,
    // but does have an aiding effect.

    void setValue(const App::any & value) const;

    // Static functions

    static ObjectIdentifier parse(const App::DocumentObject *docObj, const std::string & str);

    std::string resolveErrorString() const;

    bool adjustLinks(ExpressionVisitor &v, const std::set<App::DocumentObject *> &inList);

    bool updateElementReference(ExpressionVisitor &v, App::DocumentObject *feature=nullptr, bool reverse=false);

    void resolveAmbiguity();

    bool verify(const App::Property &prop, bool silent=false) const;

    std::size_t hash() const;

protected:

    struct ResolveResults {

        explicit ResolveResults(const ObjectIdentifier & oi);

        int propertyIndex;
        App::Document * resolvedDocument;
        String resolvedDocumentName;
        App::DocumentObject * resolvedDocumentObject;
        String resolvedDocumentObjectName;
        String subObjectName;
        App::DocumentObject * resolvedSubObject;
        App::Property * resolvedProperty;
        std::string propertyName;
        int propertyType;
        std::bitset<32> flags;

        std::string resolveErrorString() const;
        void getProperty(const ObjectIdentifier &oi);
    };

    friend struct ResolveResults;

    App::Property *resolveProperty(const App::DocumentObject *obj,
        const char *propertyName, App::DocumentObject *&sobj,int &ptype) const;

    void getSubPathStr(std::ostream &ss, const ResolveResults &result, bool toPython=false) const;

    Py::Object access(const ResolveResults &rs,
            Py::Object *value=nullptr, Dependencies *deps=nullptr) const;

    void resolve(ResolveResults & results) const;
    void resolveAmbiguity(ResolveResults & results);

    void getDepLabels(const ResolveResults &result, std::vector<std::string> &labels) const;

    App::DocumentObject * owner;
    String  documentName;
    String  documentObjectName;
    String  subObjectName;
    std::pair<std::string,std::string> shadowSub;
    std::vector<std::shared_ptr<Component>> components;
    bool documentNameSet;
    bool documentObjectNameSet;
    bool localProperty;

private:
    std::string _cache; // Cached string represstation of this identifier
    std::size_t _hash; // Cached hash of this string
};

inline std::size_t hash_value(const App::ObjectIdentifier & path) {
    return path.hash();
}

/** Helper function to convert Python object to/from App::any
*
* WARNING! Must hold Python global interpreter lock before calling these
* functions
*/
//@{
App::any AppExport pyObjectToAny(Py::Object pyobj, bool check=true);
Py::Object AppExport pyObjectFromAny(const App::any &value);
//@}

namespace ObjectPath {
App::DocumentObject *getDocumentObject(const Document *doc,
                                       const String &name,
                                       std::bitset<32> &flags);

} // namespace ObjectPath

} // namespace App

namespace std {

template<>
struct hash<App::ObjectIdentifier> {
    using argument_type = App::ObjectIdentifier;
    using result_type = std::size_t;
    inline result_type operator()(argument_type const& s) const {
        return s.hash();
    }
};

} //namespace std

#endif // APP_ObjectIdentifier_H
