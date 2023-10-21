/***************************************************************************
 *   Copyright (c) 2011 Jürgen Riegel <juergen.riegel@web.de>              *
 *   Copyright (c) 2011 Werner Mayer <wmayer[at]users.sourceforge.net>     *
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

#ifndef _PreComp_
# include <cassert>
#endif

#include <Base/Console.h>
#include <Base/Reader.h>
#include <Base/Writer.h>
#include <Base/Threads/LockedIterator.h>

#include "Transactions.h"
#include "Document.h"
#include "DocumentObject.h"
#include "Property.h"


FC_LOG_LEVEL_INIT("App",true,true)

using namespace App;
using namespace std;
using namespace Base::Threads;

TYPESYSTEM_SOURCE(App::Transaction, Base::Persistence)

//**************************************************************************
// Construction/Destruction

Transaction::Transaction(int id)
{
    if(!id) id = getNewID();
    transID = id;
}

/**
 * A destructor.
 * A more elaborate description of the destructor.
 */
Transaction::~Transaction()
{
    // TODO: eliminate the need for setting the "Destroy" status,
    // by making object removal more robust through shared_ptr/locked_ptr.
    // This would make "~Transaction() = default"! :-)

    for (const auto& record : _Objects) {
        if (record.transaction->status == TransactionObject::New) {
            // If an object has been removed from the document the transaction
            // status is 'New'. The 'pcNameInDocument' member serves as criterion
            // to check whether the object is part of the document or not.
            // Note, it's possible that the transaction status is 'New' while the
            // object is (again) part of the document. This usually happens when
            // a previous removal is undone.
            // Thus, if the object has been removed, i.e. the status is 'New' and
            // is still not part of the document the object must be destroyed not
            // to cause a memory leak. This usually is the case when the removal
            // of an object is not undone or when an addition is undone.

            if (!record.object->isAttachedToDocument()) {
                if (record.object->getTypeId().isDerivedFrom(DocumentObject::getClassTypeId())) {
                    // #0003323: Crash when clearing transaction list
                    // It can happen that when clearing the transaction list several objects
                    // are destroyed with dependencies which can lead to dangling pointers.
                    // When setting the 'Destroy' flag of an object the destructors of link
                    // properties don't try to remove backlinks, i.e. they don't try to access
                    // possible dangling pointers.
                    // An alternative solution is to call breakDependency inside
                    // Document::_removeObject. Make this change in v0.18.
                    auto obj = static_pointer_cast<const DocumentObject>(record.object);
                    // TODO: get rid of this absurd cast.
                    const_cast<DocumentObject*>(obj.get())->setStatus(ObjectStatus::Destroy, true);
                }
            }
        }
    }
}

static std::atomic<int> _TransactionID;

int Transaction::getNewID() {
    int id = ++_TransactionID;
    if(id)
        return id;
    // wrap around? really?
    return ++_TransactionID;
}

// TODO: this is thread unsafe and should be removed!
int Transaction::getLastID() {
    return _TransactionID;
}

unsigned int Transaction::getMemSize () const
{
    return 0;
}

void Transaction::Save (Base::Writer &/*writer*/) const
{
    assert(0);
}

void Transaction::Restore(Base::XMLReader &/*reader*/)
{
    assert(0);
}

int Transaction::getID() const
{
    return transID;
}

bool Transaction::isEmpty() const
{
    return _Objects.empty();
}

bool Transaction::hasObject(const TransactionalObject *Obj) const
{
    return _Objects.contains(Obj);
}

void Transaction::addOrRemoveProperty(TransactionalObject* Obj,
                                      const Property* pcProp, bool add)
{
    addOrRemoveProperty(_prepareToAssumeOwnership(Obj), pcProp, add);
}

void Transaction::addOrRemoveProperty(std::shared_ptr<TransactionalObject> sharedObj,
                                      const Property* pcProp, bool add)
{
    ExclusiveLock lock(_Objects);
    auto pos = _Objects.find(sharedObj);

    TransactionObject *To;

    if (pos) {
        To = pos->transaction.get();
    }
    else {
        auto smartTo = TransactionFactory::instance().createTransaction(sharedObj->getTypeId());
        To = smartTo.get();
        To->status = TransactionObject::Chn;
        lock[_Objects].emplace(sharedObj, std::move(smartTo));
    }

    To->addOrRemoveProperty(pcProp,add);
}

//**************************************************************************
// separator for other implementation aspects


void Transaction::apply(Document &Doc, bool forward)
{
    std::string errMsg;
    try {
        for(const auto& [obj,trans] : _Objects)
            trans->applyDel(Doc, const_cast<TransactionalObject*>(obj.get()));
        for(const auto& [obj,trans] : _Objects)
            trans->applyNew(Doc, const_cast<TransactionalObject*>(obj.get()));
        for(const auto& [obj,trans] : _Objects)
            trans->applyChn(Doc, const_cast<TransactionalObject*>(obj.get()), forward);
    }catch(Base::Exception &e) {
        e.ReportException();
        errMsg = e.what();
    }catch(std::exception &e) {
        errMsg = e.what();
    }catch(...) {
        errMsg = "Unknown exception";
    }
    if(!errMsg.empty()) {
        FC_ERR("Exception on " << (forward?"redo":"undo") << " '" 
                << Name << "':" << errMsg);
    }
}

std::shared_ptr<TransactionalObject>
Transaction::_prepareToAssumeOwnership(TransactionalObject* Obj)
{
    try {
        return Obj->SharedFromThis<TransactionalObject>();
    }
    catch (std::bad_weak_ptr&) {
        // We assume ownership.
        // There is a small race condition here,
        // but I don't know how to solve it in a simple way.
        // In the future, every TransactionalObject will be created
        // through std::make_shared().
        return std::shared_ptr<TransactionalObject>{Obj};
    }
}

std::shared_ptr<const TransactionalObject>
Transaction::_prepareToAssumeOwnership(const TransactionalObject* Obj)
{
    try {
        return Obj->SharedFromThis<TransactionalObject>();
    }
    catch (std::bad_weak_ptr&) {
        assert(false);
    }
    return nullptr;
}

void Transaction::addObjectNew(TransactionalObject *Obj)
{
    addObjectNew(_prepareToAssumeOwnership(Obj));
}

void Transaction::addObjectNew(std::shared_ptr<TransactionalObject> sharedObj)
{
    ExclusiveLock lock(_Objects);
    auto pos = _Objects.find(sharedObj);
    if (pos) {
        if (pos->transaction->status == TransactionObject::Del) {
            lock[_Objects].erase(pos);
        }
        else {
            pos->transaction->status = TransactionObject::New;
            pos->transaction->_NameInDocument = sharedObj->detachFromDocument();
            // move item at the end to make sure the order of removal is kept
            lock[_Objects].move_back(pos);
        }
    }
    else {
        auto To = TransactionFactory::instance().createTransaction(sharedObj->getTypeId());
        To->status = TransactionObject::New;
        To->_NameInDocument = sharedObj->detachFromDocument();
        lock[_Objects].emplace(std::move(sharedObj),std::move(To));
    }
}

void Transaction::addObjectDel(const TransactionalObject *Obj)
{
    addObjectDel(_prepareToAssumeOwnership(Obj));
}

void Transaction::addObjectDel(std::shared_ptr<const TransactionalObject> sharedObj)
{
    ExclusiveLock lock(_Objects);
    auto pos = _Objects.find(sharedObj);
    if(pos) {
        // is it created in this transaction ?
        if (pos->transaction->status == TransactionObject::New) {
            lock[_Objects].erase(pos);
        }
        else if (pos->transaction->status == TransactionObject::Chn) {
            pos->transaction->status = TransactionObject::Del;
        }
    }
    else {
        auto To = TransactionFactory::instance().createTransaction(sharedObj->getTypeId());
        To->status = TransactionObject::Del;
        lock[_Objects].emplace(std::move(sharedObj),std::move(To));
    }
}

void Transaction::addObjectChange(const TransactionalObject* Obj, const Property* Prop)
{
    addObjectChange(_prepareToAssumeOwnership(Obj), Prop);
}

void Transaction::addObjectChange(std::shared_ptr<const TransactionalObject> sharedObj, const Property *Prop)
{
    ExclusiveLock lock(_Objects);
    auto pos = _Objects.find(sharedObj);

    TransactionObject* To;
    if (pos) {
        To = pos->transaction.get();
    }
    else {
        auto smartTo = TransactionFactory::instance().createTransaction(sharedObj->getTypeId());
        To = smartTo.get();
        To->status = TransactionObject::Chn;
        lock[_Objects].emplace(std::move(sharedObj), std::move(smartTo));
    }

    To->setProperty(Prop);
}


//**************************************************************************
//**************************************************************************
// TransactionObject
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPESYSTEM_SOURCE_ABSTRACT(App::TransactionObject, Base::Persistence)

//**************************************************************************
// Construction/Destruction

/**
 * A constructor.
 * A more elaborate description of the constructor.
 */
TransactionObject::TransactionObject() = default;

/**
 * A destructor.
 * A more elaborate description of the destructor.
 */
TransactionObject::~TransactionObject()
{
    for(auto &v : _PropChangeMap)
        delete v.second.property;
}

void TransactionObject::applyDel(Document & /*Doc*/, TransactionalObject * /*pcObj*/)
{
}

void TransactionObject::applyNew(Document & /*Doc*/, TransactionalObject * /*pcObj*/)
{
}

void TransactionObject::applyChn(Document & /*Doc*/, TransactionalObject *pcObj, bool /* Forward */)
{
    if (status == New || status == Chn) {
        // Property change order is not preserved, as it is recursive in nature
        for(auto &v : _PropChangeMap) {
            auto &data = v.second;
            auto prop = const_cast<Property*>(data.propertyOrig);

            if(!data.property) {
                // here means we are undoing/redoing and property add operation
                pcObj->removeDynamicProperty(v.second.name.c_str());
                continue;
            }

            // getPropertyName() is specially coded to be safe even if prop has
            // been destroies. We must prepare for the case where user removed
            // a dynamic property but does not recordered as transaction.
            auto name = pcObj->getPropertyName(prop);
            if(!name || (!data.name.empty() && data.name != name) || data.propertyType != prop->getTypeId()) {
                // Here means the original property is not found, probably removed
                if(data.name.empty()) {
                    // not a dynamic property, nothing to do
                    continue;
                }

                // It is possible for the dynamic property to be removed and
                // restored. But since restoring property is actually creating
                // a new property, the property key inside redo stack will not
                // match. So we search by name first.
                prop = pcObj->getDynamicPropertyByName(data.name.c_str());
                if(!prop) {
                    // Still not found, re-create the property
                    prop = pcObj->addDynamicProperty(
                            data.propertyType.getName(),
                            data.name.c_str(), data.group.c_str(), data.doc.c_str(),
                            data.attr, data.readonly, data.hidden);
                    if(!prop)
                        continue;
                    prop->setStatusValue(data.property->getStatus());
                }
            }

            // Many properties do not bother implement Copy() and accepts
            // derived types just fine in Paste(). So we do not enforce type
            // matching here. But instead, strengthen type checking in all
            // Paste() implementation.
            //
            // if(data.propertyType != prop->getTypeId()) {
            //     FC_WARN("Cannot " << (Forward?"redo":"undo")
            //             << " change of property " << prop->getName()
            //             << " because of type change: "
            //             << data.propertyType.getName()
            //             << " -> " << prop->getTypeId().getName());
            //     continue;
            // }
            try {
                prop->Paste(*data.property);
            } catch (Base::Exception &e) {
                e.ReportException();
                FC_ERR("exception while restoring " << prop->getFullName() << ": " << e.what());
            } catch (std::exception &e) {
                FC_ERR("exception while restoring " << prop->getFullName() << ": " << e.what());
            } catch (...)
            {}
        }
    }
}

void TransactionObject::setProperty(const Property* pcProp)
{
    auto &data = _PropChangeMap[pcProp->getID()];
    if(!data.property && data.name.empty()) {
        static_cast<DynamicProperty::PropData&>(data) = 
            pcProp->getContainer()->getDynamicPropertyData(pcProp);
        data.propertyOrig = pcProp;
        data.property = pcProp->Copy();
        data.propertyType = pcProp->getTypeId();
        data.property->setStatusValue(pcProp->getStatus());
    }
}

void TransactionObject::addOrRemoveProperty(const Property* pcProp, bool add)
{
    (void)add;
    if(!pcProp || !pcProp->getContainer())
        return;

    auto &data = _PropChangeMap[pcProp->getID()];
    if(!data.name.empty()) {
        if(!add && !data.property) {
            // this means add and remove the same property inside a single
            // transaction, so they cancel each other out.
            _PropChangeMap.erase(pcProp->getID());
        }
        return;
    }
    if(data.property) {
        delete data.property;
        data.property = nullptr;
    }
    data.propertyOrig = pcProp;
    static_cast<DynamicProperty::PropData&>(data) = 
        pcProp->getContainer()->getDynamicPropertyData(pcProp);
    if(add) 
        data.property = nullptr;
    else {
        data.property = pcProp->Copy();
        data.propertyType = pcProp->getTypeId();
        data.property->setStatusValue(pcProp->getStatus());
    }
}

unsigned int TransactionObject::getMemSize () const
{
    return 0;
}

void TransactionObject::Save (Base::Writer &/*writer*/) const
{
    assert(0);
}

void TransactionObject::Restore(Base::XMLReader &/*reader*/)
{
    assert(0);
}

//**************************************************************************
//**************************************************************************
// TransactionDocumentObject
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPESYSTEM_SOURCE_ABSTRACT(App::TransactionDocumentObject, App::TransactionObject)

//**************************************************************************
// Construction/Destruction

/**
 * A constructor.
 * A more elaborate description of the constructor.
 */
TransactionDocumentObject::TransactionDocumentObject() = default;

/**
 * A destructor.
 * A more elaborate description of the destructor.
 */
TransactionDocumentObject::~TransactionDocumentObject() = default;

void TransactionDocumentObject::applyDel(Document &Doc, TransactionalObject *pcObj)
{
    if (status == Del) {
        DocumentObject* obj = static_cast<DocumentObject*>(pcObj);

#ifndef USE_OLD_DAG
        //Make sure the backlinks of all linked objects are updated. As the links of the removed
        //object are never set to [] they also do not remove the backlink. But as they are 
        //not in the document anymore we need to remove them anyway to ensure a correct graph
        auto list = obj->getOutList();
        for (auto link : list)
            link->_removeBackLink(obj);
#endif

        // simply filling in the saved object
        Doc._removeObject(obj);
    }
}

void TransactionDocumentObject::applyNew(Document &Doc, TransactionalObject *pcObj)
{
    if (status == New) {
        std::shared_ptr<DocumentObject> obj = pcObj->SharedFromThis<DocumentObject>();
        Doc._addObject(obj, _NameInDocument.c_str());

#ifndef USE_OLD_DAG
        //make sure the backlinks of all linked objects are updated
        auto list = obj->getOutList();
        for (auto link : list)
            link->_addBackLink(obj.get());
#endif
    }
}

//**************************************************************************
//**************************************************************************
// TransactionFactory
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

App::TransactionFactory* App::TransactionFactory::self = nullptr;

TransactionFactory& TransactionFactory::instance()
{
    if (!self)
        self = new TransactionFactory;
    return *self;
}

void TransactionFactory::destruct()
{
    delete self;
    self = nullptr;
}

void TransactionFactory::addProducer (const Base::Type& type, Base::AbstractProducer *producer)
{
    producers[type] = producer;
}

/**
 * Creates a transaction object for the given type id.
 */
std::unique_ptr<TransactionObject> TransactionFactory::createTransaction (const Base::Type& type) const
{
    std::map<Base::Type, Base::AbstractProducer*>::const_iterator it;
    for (it = producers.begin(); it != producers.end(); ++it) {
        if (type.isDerivedFrom(it->first)) {
            return std::unique_ptr<TransactionObject>(static_cast<TransactionObject*>(it->second->Produce()));
        }
    }

    assert(0);
    return {};
}
